import 'dart:async';
import 'dart:io';
import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:record/record.dart';
import 'package:flutter_pcm_sound/flutter_pcm_sound.dart';
import 'bluetooth_service.dart';

class AudioStreamService with ChangeNotifier {
  WebSocket? _socket;
  final AudioRecorder _recorder = AudioRecorder();
  StreamSubscription<List<int>>? _recordSub;
  
  bool _isCalling = false;
  bool _isStreamingMusic = false;
  bool _isBleStreaming = false;
  String? _statusMessage;

  bool _isMusicPaused = false;
  int _musicStreamOffset = 0;
  int _musicStreamTotalSize = 0;
  Timer? _musicTimer;
  Uint8List? _pcmBytes;

  // Streaming path fields
  StreamSubscription? _pcmEventSub;

  bool get isCalling => _isCalling;
  bool get isStreamingMusic => _isStreamingMusic;
  bool get isBleStreaming => _isBleStreaming;
  bool get isMusicPaused => _isMusicPaused;
  int get musicStreamOffset => _musicStreamOffset;
  int get musicStreamTotalSize => _musicStreamTotalSize;

  String? get statusMessage => _statusMessage;
  set statusMessage(String? val) {
    _statusMessage = val;
    notifyListeners();
  }

  // ─── PCM accumulator (filled by codec, drained by timer at audio clock rate) ─
  final List<int> _pcmAccumulator = [];
  bool _decodeComplete = false;

  // ─── Streaming BLE from file path ────────────────────────────────────────
  // Phase 1: EventChannel decodes into _pcmAccumulator (fast, background)
  // Phase 2: Timer drains accumulator at 32000 bytes/sec (16kHz 16-bit mono)
  Future<bool> startMusicStreamBLEFromPath(BLEService ble, String filePath) async {
    if (_isCalling || _isStreamingMusic) return false;

    _isStreamingMusic = true;
    _isBleStreaming = true;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmAccumulator.clear();
    _decodeComplete = false;
    _statusMessage = "Buffering...";
    notifyListeners();

    // ── Phase 1: Stream-decode into accumulator ──────────────────────────────
    const eventChannel = EventChannel('com.mrmsluna/audio_stream');
    _pcmEventSub?.cancel();
    _pcmEventSub = eventChannel
        .receiveBroadcastStream({'path': filePath})
        .listen(
      (dynamic data) {
        if (data is Uint8List) {
          _pcmAccumulator.addAll(data);
          _musicStreamTotalSize = _pcmAccumulator.length;

          // Start the rate-controlled sender once we have ~0.5s of audio buffered
          if (!_musicTimerRunning && _pcmAccumulator.length >= 16000) {
            _startRateTimer(ble);
          }
          notifyListeners();
        }
      },
      onError: (e) {
        _statusMessage = "Decode error: $e";
        _isStreamingMusic = false;
        _isBleStreaming = false;
        notifyListeners();
      },
      onDone: () {
        _decodeComplete = true;
        _musicStreamTotalSize = _pcmAccumulator.length;
        // If timer hasn't started yet (very short file), start it now
        if (!_musicTimerRunning && _pcmAccumulator.isNotEmpty) {
          _startRateTimer(ble);
        }
      },
      cancelOnError: true,
    );

    await ble.transmitStartMusic();
    return true;
  }

  bool _musicTimerRunning = false;

  // ── Phase 2: Rate-controlled sender ─────────────────────────────────────────
  // Sends exactly 640 bytes every 20ms = 32000 bytes/sec = 16kHz 16-bit mono
  // Timer is fully synchronous — no await — so it fires precisely every 20ms
  void _startRateTimer(BLEService ble) {
    if (_musicTimerRunning) return;
    _musicTimerRunning = true;
    _statusMessage = "Streaming Music...";
    notifyListeners();

    const int bytesPerTick = 640; // 20ms × 32000 B/s = 640 bytes per tick
    _musicTimer?.cancel();
    _musicTimer = Timer.periodic(const Duration(milliseconds: 20), (timer) {
      if (!_isStreamingMusic) {
        timer.cancel();
        _musicTimerRunning = false;
        return;
      }

      if (_isMusicPaused) return;

      final available = _pcmAccumulator.length - _musicStreamOffset;

      if (available <= 0) {
        if (_decodeComplete) {
          timer.cancel();
          _musicTimerRunning = false;
          _isStreamingMusic = false;
          _isBleStreaming = false;
          _statusMessage = "Playback complete.";
          notifyListeners();
        }
        return;
      }

      final toSend = available < bytesPerTick ? available : bytesPerTick;
      final chunk = Uint8List.fromList(
        _pcmAccumulator.sublist(_musicStreamOffset, _musicStreamOffset + toSend),
      );
      _musicStreamOffset += toSend;
      notifyListeners();

      // Fire and forget — BLE stack queues the packets, no blocking
      ble.transmitAudioChunk(chunk);
    });
  }

  Future<void> stopMusicStreamBLEFromPath() async {
    _pcmEventSub?.cancel();
    _pcmEventSub = null;
    _musicTimer?.cancel();
    _musicTimer = null;
    _musicTimerRunning = false;
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmAccumulator.clear();
    _decodeComplete = false;
    _statusMessage = "Stopped.";
    notifyListeners();
  }

  void pauseMusic() {
    if (_isStreamingMusic && !_isMusicPaused) {
      _isMusicPaused = true;
      _statusMessage = "Music Paused";
      notifyListeners();
    }
  }

  void resumeMusic() {
    if (_isStreamingMusic && _isMusicPaused) {
      _isMusicPaused = false;
      _statusMessage = "Streaming Music...";
      notifyListeners();
    }
  }

  void seekMusic(int newOffset) {
    if (_isStreamingMusic) {
      int alignedOffset = (newOffset ~/ 2) * 2;
      _musicStreamOffset = alignedOffset.clamp(0, _musicStreamTotalSize);
      notifyListeners();
    }
  }
  
  // Convert binary byte buffer to signed 16-bit integer array for the player
  List<int> bytesToInt16List(Uint8List bytes) {
    final int16List = Int16List.view(bytes.buffer, bytes.offsetInBytes, bytes.lengthInBytes ~/ 2);
    return int16List.toList();
  }

  // Set up audio player (FlutterPcmSound)
  Future<void> initPlayer() async {
    await FlutterPcmSound.setup(
      sampleRate: 16000,
      channelCount: 1, // Mono
    );
  }

  // Connect to the WebSocket broker
  Future<bool> startCall(String serverIp, String robotMac) async {
    if (_isCalling || _isStreamingMusic) return false;
    _statusMessage = "Connecting to VoIP broker...";
    notifyListeners();
    
    final cleanMac = robotMac.replaceAll(':', '').toUpperCase();
    final urlStr = 'ws://$serverIp:8001/ws?mac=$cleanMac&variant=phone';
    
    try {
      // 1. Initialize player
      await initPlayer();
      
      // 2. Connect WebSocket
      _socket = await WebSocket.connect(urlStr).timeout(const Duration(seconds: 5));
      _isCalling = true;
      _statusMessage = "Call Connected!";
      notifyListeners();
      
      // 3. Listen to incoming audio from ESP32
      _socket!.listen(
        (data) async {
          if (data is List<int>) {
            // Write received audio bytes directly to the pcm player
            final samples = bytesToInt16List(Uint8List.fromList(data));
            if (samples.isNotEmpty) {
              await FlutterPcmSound.feed(PcmArrayInt16.fromList(samples));
            }
          }
        },
        onError: (err) {
          stopCall();
        },
        onDone: () {
          stopCall();
        },
      );
      
      // 4. Request mic permissions & Start Recording mic input
      final hasPermission = await _recorder.hasPermission();
      if (!hasPermission) {
        _statusMessage = "Mic permission denied.";
        stopCall();
        return false;
      }
      
      final recordConfig = const RecordConfig(
        encoder: AudioEncoder.pcm16bits,
        sampleRate: 16000,
        numChannels: 1,
      );
      
      final recordStream = await _recorder.startStream(recordConfig);
      _recordSub = recordStream.listen(
        (chunk) {
          if (_socket != null && _isCalling) {
            _socket!.add(chunk);
          }
        },
        onError: (err) {
          stopCall();
        },
      );
      
      return true;
    } catch (e) {
      _statusMessage = "Call connection failed: $e";
      _isCalling = false;
      notifyListeners();
      return false;
    }
  }
  
  Future<void> stopCall() async {
    _statusMessage = "Call stopped.";
    _isCalling = false;
    notifyListeners();
    
    await _recordSub?.cancel();
    _recordSub = null;
    
    await _recorder.stop();
    await FlutterPcmSound.release();
    
    await _socket?.close();
    _socket = null;
  }

  // Stream picked music (WAV / parsed PCM)
  Future<bool> startMusicStream(String serverIp, String robotMac, Uint8List pcmBytes) async {
    if (_isCalling || _isStreamingMusic) return false;
    _statusMessage = "Connecting to music broker...";
    notifyListeners();
    
    final cleanMac = robotMac.replaceAll(':', '').toUpperCase();
    final urlStr = 'ws://$serverIp:8001/ws?mac=$cleanMac&variant=phone';
    
    try {
      _socket = await WebSocket.connect(urlStr).timeout(const Duration(seconds: 5));
      _isStreamingMusic = true;
      _isMusicPaused = false;
      _pcmBytes = pcmBytes;
      _musicStreamOffset = 0;
      _musicStreamTotalSize = pcmBytes.length;
      _statusMessage = "Streaming Music...";
      notifyListeners();
      
      // Send music in chunks to avoid overloading the ESP32 buffer
      // 16kHz, 16-bit, Mono PCM = 32,000 bytes per second.
      // Let's send chunks of 2048 bytes every 64ms.
      const chunkSize = 2048;
      
      _musicTimer?.cancel();
      _musicTimer = Timer.periodic(const Duration(milliseconds: 64), (timer) {
        if (!_isStreamingMusic || _socket == null) {
          timer.cancel();
          return;
        }

        if (_isMusicPaused) {
          return;
        }
        
        if (_musicStreamOffset >= _musicStreamTotalSize) {
          stopMusicStream();
          timer.cancel();
          return;
        }
        
        int end = _musicStreamOffset + chunkSize;
        if (end > _musicStreamTotalSize) end = _musicStreamTotalSize;
        
        final chunk = _pcmBytes!.sublist(_musicStreamOffset, end);
        _socket!.add(chunk);
        _musicStreamOffset = end;
        notifyListeners();
      });
      
      return true;
    } catch (e) {
      _statusMessage = "Music connection failed: $e";
      _isStreamingMusic = false;
      notifyListeners();
      return false;
    }
  }
  
  Future<void> stopMusicStream() async {
    _statusMessage = "Music stream stopped.";
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmBytes = null;
    notifyListeners();
    
    _musicTimer?.cancel();
    _musicTimer = null;
    
    await _socket?.close();
    _socket = null;
  }

  Future<bool> startMusicStreamBLE(BLEService ble, Uint8List pcmBytes) async {
    if (_isCalling || _isStreamingMusic) return false;
    _statusMessage = "Connecting Bluetooth Audio...";
    notifyListeners();
    
    _isStreamingMusic = true;
    _isBleStreaming = true;
    _isMusicPaused = false;
    _pcmBytes = pcmBytes;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = pcmBytes.length;
    _statusMessage = "Streaming Music...";
    notifyListeners();
    
    final startTime = DateTime.now().millisecondsSinceEpoch;
    
    _musicTimer?.cancel();
    _musicTimer = Timer.periodic(const Duration(milliseconds: 15), (timer) async {
      if (!_isStreamingMusic || !_isBleStreaming) {
        timer.cancel();
        return;
      }

      if (_isMusicPaused) {
        return;
      }
      
      final elapsedMs = DateTime.now().millisecondsSinceEpoch - startTime;
      final targetOffset = (elapsedMs * 32) ~/ 2 * 2;
      
      if (_musicStreamOffset >= _musicStreamTotalSize) {
        stopMusicStreamBLE();
        timer.cancel();
        return;
      }
      
      if (targetOffset > _musicStreamOffset) {
        int bytesToSend = targetOffset - _musicStreamOffset;
        if (bytesToSend > 960) bytesToSend = 960;
        
        int end = _musicStreamOffset + bytesToSend;
        if (end > _musicStreamTotalSize) end = _musicStreamTotalSize;
        
        final chunk = _pcmBytes!.sublist(_musicStreamOffset, end);
        _musicStreamOffset = end;
        notifyListeners();
        
        await ble.transmitAudioChunk(chunk);
      }
    });
    
    return true;
  }

  Future<void> stopMusicStreamBLE() async {
    _statusMessage = "Music stream stopped.";
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmBytes = null;
    notifyListeners();
    
    _musicTimer?.cancel();
    _musicTimer = null;
  }
}
