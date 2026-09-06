import 'dart:async';
import 'dart:typed_data';
import 'package:image/image.dart' as img;
import 'bluetooth_service.dart';

// ─── Transfer progress snapshot ──────────────────────────────────────────────
enum TransferPhase { idle, compressing, waitingReady, sending, verifying, done, failed }

class TransferProgress {
  final TransferPhase phase;
  final int percent;        // 0–100
  final double speedKBs;   // KB/s
  final int etaSeconds;    // remaining seconds
  final String? error;

  const TransferProgress({
    required this.phase,
    this.percent = 0,
    this.speedKBs = 0,
    this.etaSeconds = 0,
    this.error,
  });

  static const idle = TransferProgress(phase: TransferPhase.idle);
}

// ─── CRC-32 (IEEE 802.3) ─────────────────────────────────────────────────────
class _Crc32 {
  static final List<int> _table = _buildTable();

  static List<int> _buildTable() {
    final t = List<int>.filled(256, 0);
    for (int i = 0; i < 256; i++) {
      int c = i;
      for (int j = 0; j < 8; j++) {
        c = (c & 1) != 0 ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
      }
      t[i] = c;
    }
    return t;
  }

  static int compute(Uint8List data) {
    int crc = 0xFFFFFFFF;
    for (final b in data) {
      crc = _table[(crc ^ b) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
  }

  static String toHex(int crc) =>
      crc.toUnsigned(32).toRadixString(16).padLeft(8, '0');
}

// ─── ImageTransferService ─────────────────────────────────────────────────────
class ImageTransferService {
  static const int targetWidth  = 240;
  static const int targetHeight = 280;
  static const int maxBytes     = 100 * 1024; // 100 KB
  static const int chunkSize    = 512;

  final BLEService _ble;

  final _progressController =
      StreamController<TransferProgress>.broadcast();
  Stream<TransferProgress> get progress => _progressController.stream;

  TransferProgress _lastProgress = TransferProgress.idle;
  bool _cancelled = false;

  ImageTransferService(this._ble);

  void dispose() => _progressController.close();

  // ── STEP 1: Process image ────────────────────────────────────────────────
  /// Accepts raw image bytes (any format — JPEG, PNG, HEIC…).
  /// Returns JPEG bytes ≤ maxBytes, or throws if impossible.
  Future<Uint8List> processImage(Uint8List inputBytes) async {
    _emit(TransferProgress(phase: TransferPhase.compressing, percent: 5));

    // Decode
    final decoded = img.decodeImage(inputBytes);
    if (decoded == null) throw Exception('Could not decode image');

    // Resize + crop to exact 240×280 using high-sharpness bicubic interpolation
    var resized = _cropAndResize(decoded, targetWidth, targetHeight);

    // Subtle color & contrast boost for IPS display clarity
    resized = img.adjustColor(resized, contrast: 1.06, saturation: 1.04);

    _emit(TransferProgress(phase: TransferPhase.compressing, percent: 20));

    // High quality JPEG encoding (starts at 95 for ultra-crisp detail)
    Uint8List? jpegBytes;
    int quality = 95;
    while (quality >= 40) {
      final encoded = img.encodeJpg(resized, quality: quality);
      jpegBytes = Uint8List.fromList(encoded);
      if (jpegBytes.length <= maxBytes) break;
      quality -= 5;
      jpegBytes = null;
    }

    if (jpegBytes == null) {
      // Last-resort: drop to 240×280 at quality 20
      final encoded = img.encodeJpg(resized, quality: 20);
      jpegBytes = Uint8List.fromList(encoded);
    }

    if (jpegBytes.length > maxBytes) {
      throw Exception(
          'Image too large after compression (${jpegBytes.length} B > $maxBytes B). '
          'Please choose a simpler image.');
    }

    _emit(TransferProgress(phase: TransferPhase.compressing, percent: 40));
    return jpegBytes;
  }

  // ── STEP 2: Send wallpaper over BLE ─────────────────────────────────────
  Future<void> sendWallpaper(Uint8List jpegBytes) async {
    _cancelled = false;

    if (!_ble.isConnected) {
      throw Exception('Not connected to Luna hardware');
    }
    if (!_ble.imageCharAvailable) {
      throw Exception(
          'Image BLE characteristic not found. '
          'Please flash the latest Luna firmware.');
    }

    final crc = _Crc32.compute(jpegBytes);
    final crcHex = _Crc32.toHex(crc);
    final totalBytes = jpegBytes.length;

    // 1. Send IMG_START and wait for IMG_READY
    _emit(TransferProgress(phase: TransferPhase.waitingReady, percent: 0));
    final readyCompleter = Completer<void>();
    StreamSubscription? sub;
    sub = _ble.robotEvents.listen((event) {
      if (event == 'IMG_READY') {
        sub?.cancel();
        if (!readyCompleter.isCompleted) readyCompleter.complete();
      } else if (event.startsWith('IMG_FAIL')) {
        sub?.cancel();
        if (!readyCompleter.isCompleted) {
          readyCompleter.completeError(Exception('Device rejected start: $event'));
        }
      }
    });

    await _ble.transmitImgStart(totalBytes, crcHex);
    await readyCompleter.future.timeout(
      const Duration(seconds: 10),
      onTimeout: () {
        sub?.cancel();
        throw Exception('Timeout waiting for IMG_READY from device');
      },
    );

    if (_cancelled) throw Exception('Transfer cancelled');

    // 2. Stream binary chunks
    int sent = 0;
    final startTime = DateTime.now();
    _emit(TransferProgress(phase: TransferPhase.sending, percent: 0));

    while (sent < totalBytes) {
      if (_cancelled) {
        await _ble.transmitImgCancel();
        throw Exception('Transfer cancelled');
      }

      final end = (sent + chunkSize < totalBytes) ? sent + chunkSize : totalBytes;
      final chunk = jpegBytes.sublist(sent, end);

      bool ok = false;
      for (int attempt = 0; attempt < 3; attempt++) {
        ok = await _ble.sendImageChunk(chunk);
        if (ok) break;
        await Future.delayed(const Duration(milliseconds: 50));
      }
      if (!ok) {
        await _ble.transmitImgCancel();
        throw Exception('Chunk send failed after retries at offset $sent');
      }

      sent = end;

      // Throttle: give the ESP32 BLE stack time to process
      await Future.delayed(const Duration(milliseconds: 10));

      final elapsed = DateTime.now().difference(startTime).inMilliseconds;
      final speedKBs = elapsed > 0 ? (sent / 1024) / (elapsed / 1000.0) : 0.0;
      final remaining = totalBytes - sent;
      final eta = speedKBs > 0 ? (remaining / 1024 / speedKBs).round() : 0;
      final pct = ((sent / totalBytes) * 100).round();

      _emit(TransferProgress(
        phase: TransferPhase.sending,
        percent: pct,
        speedKBs: speedKBs,
        etaSeconds: eta,
      ));
    }

    // 3. Send IMG_END and wait for IMG_OK / IMG_FAIL
    _emit(TransferProgress(phase: TransferPhase.verifying, percent: 100));
    final doneCompleter = Completer<void>();
    StreamSubscription? doneSub;
    doneSub = _ble.robotEvents.listen((event) {
      if (event == 'IMG_OK') {
        doneSub?.cancel();
        if (!doneCompleter.isCompleted) doneCompleter.complete();
      } else if (event.startsWith('IMG_FAIL')) {
        doneSub?.cancel();
        if (!doneCompleter.isCompleted) {
          doneCompleter.completeError(Exception('Transfer failed on device: $event'));
        }
      }
    });

    await _ble.transmitImgEnd();
    await doneCompleter.future.timeout(
      const Duration(seconds: 30),
      onTimeout: () {
        doneSub?.cancel();
        throw Exception('Timeout waiting for IMG_OK confirmation');
      },
    );

    _emit(TransferProgress(phase: TransferPhase.done, percent: 100));
  }

  void cancel() {
    _cancelled = true;
  }

  // ── Private helpers ──────────────────────────────────────────────────────
  void _emit(TransferProgress p) {
    _lastProgress = p;
    _progressController.add(p);
  }

  /// Crops and resizes the image to exactly [targetW]×[targetH],
  /// preserving aspect ratio (cover / fill mode, centred crop).
  img.Image _cropAndResize(img.Image src, int targetW, int targetH) {
    final srcW = src.width;
    final srcH = src.height;
    final scaleW = targetW / srcW;
    final scaleH = targetH / srcH;
    final scale = scaleW > scaleH ? scaleW : scaleH;

    final scaledW = (srcW * scale).round();
    final scaledH = (srcH * scale).round();
    final scaled = img.copyResize(src, width: scaledW, height: scaledH,
        interpolation: img.Interpolation.cubic);

    final cropX = (scaledW - targetW) ~/ 2;
    final cropY = (scaledH - targetH) ~/ 2;
    return img.copyCrop(scaled,
        x: cropX, y: cropY, width: targetW, height: targetH);
  }
}
