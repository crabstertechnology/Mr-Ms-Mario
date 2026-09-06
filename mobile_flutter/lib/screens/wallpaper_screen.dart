import 'dart:io';
import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:image_picker/image_picker.dart';
import 'package:provider/provider.dart';
import '../services/bluetooth_service.dart';
import '../services/image_transfer_service.dart';

class WallpaperScreen extends StatefulWidget {
  const WallpaperScreen({super.key});

  @override
  State<WallpaperScreen> createState() => _WallpaperScreenState();
}

class _WallpaperScreenState extends State<WallpaperScreen>
    with TickerProviderStateMixin {
  // ── State ──────────────────────────────────────────────────────────────
  Uint8List? _rawImageBytes;
  Uint8List? _compressedJpeg;
  TransferProgress _progress = TransferProgress.idle;
  ImageTransferService? _txService;
  bool _isBusy = false;
  String? _statusMessage;
  bool _success = false;

  final GlobalKey _cropKey = GlobalKey();
  late final TransformationController _transformController;

  late final AnimationController _pulseCtrl;
  late final AnimationController _successCtrl;

  // ── Lifecycle ──────────────────────────────────────────────────────────
  @override
  void initState() {
    super.initState();
    _transformController = TransformationController();

    _pulseCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 900),
    )..repeat(reverse: true);

    _successCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 600),
    );
  }

  @override
  void dispose() {
    _transformController.dispose();
    _pulseCtrl.dispose();
    _successCtrl.dispose();
    _txService?.dispose();
    super.dispose();
  }

  // ── Picking ────────────────────────────────────────────────────────────
  Future<void> _pickImage() async {
    final picker = ImagePicker();
    final picked = await picker.pickImage(source: ImageSource.gallery);
    if (picked == null) return;

    setState(() {
      _isBusy = true;
      _rawImageBytes = null;
      _compressedJpeg = null;
      _statusMessage = 'Loading image preview…';
      _success = false;
    });

    try {
      final bytes = await File(picked.path).readAsBytes();
      setState(() {
        _rawImageBytes = bytes;
        _isBusy = false;
        _transformController.value = Matrix4.identity();
        _statusMessage = 'Drag & pinch image to frame for Luna Watch';
      });
    } catch (e) {
      setState(() {
        _isBusy = false;
        _statusMessage = 'Error loading image: $e';
      });
    }
  }

  // ── Crop & Transfer ───────────────────────────────────────────────────
  Future<void> _sendWallpaper() async {
    if (_rawImageBytes == null) return;

    setState(() {
      _isBusy = true;
      _success = false;
      _statusMessage = 'Cropping & preparing image…';
      _progress = const TransferProgress(
          phase: TransferPhase.compressing, percent: 5);
    });

    try {
      // 1. Capture exact framed pixels from InteractiveViewer
      final boundary = _cropKey.currentContext?.findRenderObject()
          as RenderRepaintBoundary?;
      if (boundary == null) throw Exception('Could not capture frame preview');

      final ui.Image capturedImage = await boundary.toImage(pixelRatio: 2.0);
      final ByteData? byteData =
          await capturedImage.toByteData(format: ui.ImageByteFormat.png);
      if (byteData == null) throw Exception('Failed to encode cropped image');

      final Uint8List croppedBytes = byteData.buffer.asUint8List();

      final ble = context.read<BLEService>();
      _txService?.dispose();
      _txService = ImageTransferService(ble);

      _txService!.progress.listen((p) {
        if (mounted) setState(() => _progress = p);
      });

      // 2. Compress cropped framing to 240x280 JPEG <= 100KB
      final jpeg = await _txService!.processImage(croppedBytes);
      _compressedJpeg = jpeg;

      // 3. Send over BLE
      await _txService!.sendWallpaper(jpeg);
      _successCtrl.forward(from: 0);
      setState(() {
        _isBusy = false;
        _success = true;
        _statusMessage = 'Wallpaper set successfully! 🎉';
      });
    } catch (e) {
      setState(() {
        _isBusy = false;
        _progress = TransferProgress(
            phase: TransferPhase.failed, error: e.toString());
        _statusMessage = 'Failed: $e';
      });
    }
  }

  void _cancelTransfer() {
    _txService?.cancel();
    setState(() {
      _isBusy = false;
      _progress = TransferProgress.idle;
      _statusMessage = 'Transfer cancelled';
    });
  }

  // ── Delete ─────────────────────────────────────────────────────────────
  Future<void> _deleteWallpaper() async {
    final ble = context.read<BLEService>();
    await ble.transmitImgDelete();
    setState(() {
      _rawImageBytes = null;
      _compressedJpeg = null;
      _statusMessage = 'Wallpaper cleared from device';
      _success = false;
    });
  }

  // ── Build (Non-scrollable, Fixed Layout) ─────────────────────────────
  @override
  Widget build(BuildContext context) {
    final ble = context.watch<BLEService>();
    final connected = ble.isConnected;

    return Scaffold(
      backgroundColor: const Color(0xFF0D0D1A),
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back_ios_new, color: Colors.white70),
          onPressed: () => Navigator.of(context).pop(),
        ),
        title: const Text(
          'Luna Wallpaper Studio',
          style: TextStyle(
            color: Colors.white,
            fontWeight: FontWeight.w600,
            letterSpacing: 0.5,
          ),
        ),
        centerTitle: true,
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
            colors: [Color(0xFF0D0D1A), Color(0xFF1A0A2E), Color(0xFF0D1A2E)],
          ),
        ),
        child: SafeArea(
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            child: Column(
              children: [
                // ── Connection banner ───────────────────────────────────
                if (!connected) ...[
                  _GlassCard(
                    child: Row(
                      children: [
                        AnimatedBuilder(
                          animation: _pulseCtrl,
                          builder: (_, __) => Icon(
                            Icons.bluetooth_disabled,
                            color: Color.lerp(
                                Colors.red, Colors.orange, _pulseCtrl.value),
                            size: 18,
                          ),
                        ),
                        const SizedBox(width: 10),
                        const Expanded(
                          child: Text(
                            'Connect to Luna hardware first',
                            style: TextStyle(color: Colors.white70, fontSize: 13),
                          ),
                        ),
                      ],
                    ),
                  ),
                  const SizedBox(height: 10),
                ],

                // ── Interactive Watch Frame Canvas ───────────────────────
                Expanded(
                  child: Center(
                    child: _buildPreview(),
                  ),
                ),

                const SizedBox(height: 12),

                // ── Status text ─────────────────────────────────────────
                if (_statusMessage != null)
                  AnimatedSwitcher(
                    duration: const Duration(milliseconds: 300),
                    child: Text(
                      _statusMessage!,
                      key: ValueKey(_statusMessage),
                      textAlign: TextAlign.center,
                      style: TextStyle(
                        color: _success
                            ? const Color(0xFF00E5CC)
                            : _progress.phase == TransferPhase.failed
                                ? Colors.redAccent
                                : Colors.white70,
                        fontSize: 13,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                  ),

                const SizedBox(height: 10),

                // ── Progress bar ────────────────────────────────────────
                if (_isBusy) ...[
                  _buildProgressBar(),
                  const SizedBox(height: 10),
                ],

                // ── Action buttons ──────────────────────────────────────
                _buildActionButtons(connected),

                const SizedBox(height: 6),

                // ── Clear button ───────────────────────────────────────
                if (connected && !_isBusy)
                  TextButton.icon(
                    onPressed: _deleteWallpaper,
                    icon: const Icon(Icons.delete_outline,
                        color: Colors.redAccent, size: 16),
                    label: const Text(
                      'Clear device wallpaper',
                      style: TextStyle(color: Colors.redAccent, fontSize: 12),
                    ),
                  ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  // ── Preview & Gesture Cropper ──────────────────────────────────────────
  Widget _buildPreview() {
    final double pw = (MediaQuery.of(context).size.width - 70).clamp(0, 210);
    final double ph = pw * (280 / 240);

    return Stack(
      alignment: Alignment.center,
      children: [
        // Watch frame bezel
        Container(
          width: pw + 16,
          height: ph + 16,
          decoration: BoxDecoration(
            color: const Color(0xFF1E1E30),
            borderRadius: BorderRadius.circular(24),
            border: Border.all(
              color: const Color(0xFF6B4EFF).withOpacity(0.8),
              width: 3,
            ),
            boxShadow: [
              BoxShadow(
                color: const Color(0xFF6B4EFF).withOpacity(0.35),
                blurRadius: 28,
                spreadRadius: 2,
              ),
            ],
          ),
        ),

        // Interactive Cropper Canvas (Gestures: Pan, Drag, Pinch-to-zoom)
        ClipRRect(
          borderRadius: BorderRadius.circular(18),
          child: SizedBox(
            width: pw,
            height: ph,
            child: _rawImageBytes != null
                ? RepaintBoundary(
                    key: _cropKey,
                    child: Container(
                      color: Colors.black,
                      child: InteractiveViewer(
                        transformationController: _transformController,
                        minScale: 1.0,
                        maxScale: 4.0,
                        clipBehavior: Clip.hardEdge,
                        child: Image.memory(
                          _rawImageBytes!,
                          width: pw,
                          height: ph,
                          fit: BoxFit.cover,
                        ),
                      ),
                    ),
                  )
                : Container(
                    color: const Color(0xFF0D0D1A),
                    child: Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Icon(Icons.add_photo_alternate_outlined,
                            size: 44,
                            color: Colors.white.withOpacity(0.2)),
                        const SizedBox(height: 10),
                        Text(
                          'Tap below to pick photo',
                          style: TextStyle(
                              color: Colors.white.withOpacity(0.3),
                              fontSize: 12),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          '240 × 280 Watch Display',
                          style: TextStyle(
                              color: const Color(0xFF00E5CC)
                                  .withOpacity(0.4),
                              fontSize: 11,
                              fontWeight: FontWeight.w500),
                        ),
                      ],
                    ),
                  ),
          ),
        ),

        // Success Overlay
        if (_success)
          ScaleTransition(
            scale: CurvedAnimation(
                parent: _successCtrl, curve: Curves.elasticOut),
            child: Container(
              width: pw,
              height: ph,
              decoration: BoxDecoration(
                color: Colors.black.withOpacity(0.65),
                borderRadius: BorderRadius.circular(18),
              ),
              child: const Center(
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(Icons.check_circle,
                        color: Color(0xFF00E5CC), size: 56),
                    SizedBox(height: 8),
                    Text(
                      'Wallpaper Flashed!',
                      style: TextStyle(
                        color: Colors.white,
                        fontWeight: FontWeight.bold,
                        fontSize: 13,
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
      ],
    );
  }

  // ── Progress bar ────────────────────────────────────────────────────────
  Widget _buildProgressBar() {
    final phase = _progress.phase;
    final isReceiving = phase == TransferPhase.sending;
    final label = switch (phase) {
      TransferPhase.compressing => 'Cropping & compressing JPEG…',
      TransferPhase.waitingReady => 'Waiting for device…',
      TransferPhase.sending =>
        '${_progress.percent}%  •  ${_progress.speedKBs.toStringAsFixed(1)} KB/s  •  ${_progress.etaSeconds}s',
      TransferPhase.verifying => 'Verifying on device…',
      TransferPhase.done => 'Complete!',
      TransferPhase.failed => 'Failed',
      _ => '',
    };

    return Column(
      children: [
        ClipRRect(
          borderRadius: BorderRadius.circular(8),
          child: LinearProgressIndicator(
            value: isReceiving ? _progress.percent / 100 : null,
            backgroundColor: const Color(0xFF1E1E30),
            valueColor:
                const AlwaysStoppedAnimation<Color>(Color(0xFF00E5CC)),
            minHeight: 6,
          ),
        ),
        const SizedBox(height: 6),
        Text(label,
            style: const TextStyle(color: Colors.white60, fontSize: 11)),
      ],
    );
  }

  // ── Action buttons ──────────────────────────────────────────────────────
  Widget _buildActionButtons(bool connected) {
    if (_isBusy && _progress.phase == TransferPhase.sending) {
      return _LunaButton(
        label: 'Cancel Transfer',
        icon: Icons.cancel_outlined,
        color: Colors.redAccent,
        onTap: _cancelTransfer,
      );
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Pick image
        _LunaButton(
          label: _rawImageBytes == null
              ? 'Pick from Gallery'
              : 'Pick Different Image',
          icon: Icons.photo_library_outlined,
          color: const Color(0xFF6B4EFF),
          onTap: _isBusy ? null : _pickImage,
        ),
        const SizedBox(height: 10),
        // Send
        if (_rawImageBytes != null)
          _LunaButton(
            label: 'Crop & Send to Luna Watch',
            icon: Icons.send_rounded,
            color: connected ? const Color(0xFF00C896) : Colors.grey,
            onTap: (connected && !_isBusy) ? _sendWallpaper : null,
          ),
      ],
    );
  }
}

// ─── Reusable widgets ─────────────────────────────────────────────────────────

class _GlassCard extends StatelessWidget {
  final Widget child;
  const _GlassCard({required this.child});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.05),
        borderRadius: BorderRadius.circular(14),
        border: Border.all(color: Colors.white.withOpacity(0.08)),
      ),
      child: child,
    );
  }
}

class _LunaButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final Color color;
  final VoidCallback? onTap;

  const _LunaButton({
    required this.label,
    required this.icon,
    required this.color,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    final enabled = onTap != null;
    return GestureDetector(
      onTap: onTap,
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 200),
        padding: const EdgeInsets.symmetric(vertical: 14, horizontal: 18),
        decoration: BoxDecoration(
          gradient: enabled
              ? LinearGradient(
                  colors: [color.withOpacity(0.85), color],
                  begin: Alignment.centerLeft,
                  end: Alignment.centerRight,
                )
              : null,
          color: enabled ? null : Colors.grey.withOpacity(0.15),
          borderRadius: BorderRadius.circular(14),
          boxShadow: enabled
              ? [
                  BoxShadow(
                      color: color.withOpacity(0.35),
                      blurRadius: 10,
                      offset: const Offset(0, 3))
                ]
              : [],
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon,
                color: Colors.white.withOpacity(enabled ? 1 : 0.4), size: 18),
            const SizedBox(width: 8),
            Text(
              label,
              style: TextStyle(
                color: Colors.white.withOpacity(enabled ? 1 : 0.4),
                fontWeight: FontWeight.w600,
                fontSize: 14,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
