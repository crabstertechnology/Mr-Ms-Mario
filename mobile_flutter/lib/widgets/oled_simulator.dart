import 'dart:convert';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import '../services/database_service.dart';

class OLEDSimulator extends StatefulWidget {
  final String activeGifId;
  final String activeLabel;
  final String? marqueeText; // If set, displays scrolling marquee instead of GIF
  final bool? invertColor; // If set, overrides db.oledInvert

  const OLEDSimulator({
    Key? key,
    required this.activeGifId,
    required this.activeLabel,
    this.marqueeText,
    this.invertColor,
  }) : super(key: key);

  @override
  State<OLEDSimulator> createState() => _OLEDSimulatorState();
}

class _OLEDSimulatorState extends State<OLEDSimulator> with SingleTickerProviderStateMixin {
  AnimationController? _marqueeController;
  late Animation<double> _marqueeAnimation;
  double _textWidth = 0.0;
  bool _isMarqueeActive = false;

  @override
  void initState() {
    super.initState();
    _marqueeController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 4),
    );
    _marqueeAnimation = Tween<double>(begin: 1.0, end: -1.0).animate(_marqueeController!)
      ..addStatusListener((status) {
        if (status == AnimationStatus.completed) {
          setState(() {
            _isMarqueeActive = false;
          });
          _marqueeController?.reset();
        }
      });

    if (widget.marqueeText != null && widget.marqueeText!.isNotEmpty) {
      _startMarquee();
    }
  }

  @override
  void didUpdateWidget(covariant OLEDSimulator oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.marqueeText != oldWidget.marqueeText &&
        widget.marqueeText != null &&
        widget.marqueeText!.isNotEmpty) {
      _startMarquee();
    }
  }

  void _startMarquee() {
    setState(() {
      _isMarqueeActive = true;
      // Estimate text width: ~8 pixels per character on standard scale
      _textWidth = widget.marqueeText!.length * 8.0;
    });
    
    // Duration proportional to text length to keep velocity constant
    final charCount = widget.marqueeText!.length;
    final durationSecs = max(3.0, charCount * 0.15);
    
    _marqueeController?.duration = Duration(milliseconds: (durationSecs * 1000).toInt());
    _marqueeController?.forward(from: 0.0);
  }

  @override
  void dispose() {
    _marqueeController?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);

    // Apply brightness, contrast (opacity/blending), invert, and rotation
    final brightnessVal = db.oledBrightness;
    final invertVal = widget.invertColor ?? db.oledInvert;
    final rotationVal = db.oledRotation; // in degrees (0, 90, 180, 270)

    final oledCyan = const Color(0xFF00F0FF);
    final oledColor = invertVal ? Colors.black : oledCyan;
    final oledBgColor = invertVal ? oledCyan : const Color(0xFF000000);

    // Transform degrees to quarter turns for RotatedBox
    int quarterTurns = (rotationVal / 90.0).round() % 4;

    Widget screenContent;
    if (_isMarqueeActive && widget.marqueeText != null) {
      screenContent = LayoutBuilder(
        builder: (context, constraints) {
          final screenWidth = constraints.maxWidth;
          return Stack(
            alignment: Alignment.center,
            children: [
              AnimatedBuilder(
                animation: _marqueeAnimation,
                builder: (context, child) {
                  // Scroll from right (+screenWidth) to left (-_textWidth)
                  final offset = _marqueeAnimation.value * (screenWidth + _textWidth) / 2 + (screenWidth - _textWidth) / 2;
                  return Positioned(
                    left: offset,
                    child: Text(
                      widget.marqueeText!.toUpperCase(),
                      style: GoogleFonts.pressStart2p(
                        color: oledColor,
                        fontSize: 9,
                        fontWeight: FontWeight.bold,
                        shadows: [
                          Shadow(
                            color: oledCyan.withOpacity(0.6),
                            blurRadius: 4,
                          ),
                        ],
                      ),
                    ),
                  );
                },
              ),
            ],
          );
        },
      );
    } else {
      // Normal Eye expression GIF
      Widget imageWidget;
      final currentGif = db.gifs.firstWhere((g) => g.id == widget.activeGifId, orElse: () => db.gifs.first);
      
      if (currentGif.customData != null && currentGif.customData!.isNotEmpty) {
        // Base64 user uploaded image
        try {
          final base64Str = currentGif.customData!.split(',').last;
          final bytes = base64Decode(base64Str);
          imageWidget = Image.memory(
            bytes,
            fit: BoxFit.contain,
            gaplessPlayback: true,
          );
        } catch (e) {
          imageWidget = const Icon(Icons.broken_image, color: Colors.red);
        }
      } else {
        // Built-in assets GIF
        imageWidget = Image.asset(
          'assets/animations/${widget.activeGifId}.gif',
          fit: BoxFit.contain,
          gaplessPlayback: true,
          errorBuilder: (context, error, stackTrace) => Image.asset(
            'assets/animations/blank.gif',
            fit: BoxFit.contain,
            gaplessPlayback: true,
          ),
        );
      }

      screenContent = Stack(
        children: [
          // Glowing Cyan text label overlay
          Positioned(
            top: 6,
            left: 0,
            right: 0,
            child: Center(
              child: Text(
                widget.activeLabel.toUpperCase(),
                style: GoogleFonts.pressStart2p(
                  color: oledColor,
                  fontSize: 6,
                  shadows: [
                    Shadow(
                      color: oledCyan.withOpacity(0.6),
                      blurRadius: 3,
                    ),
                  ],
                ),
              ),
            ),
          ),
          // OLED colored image (using color filter to color pixelated white/grey eyes to cyan)
          Center(
            child: Container(
              width: 128,
              height: 64,
              padding: const EdgeInsets.only(top: 8),
              child: ColorFiltered(
                colorFilter: const ColorFilter.mode(
                  Color(0xFF00F0FF),
                  BlendMode.modulate,
                ),
                child: ColorFiltered(
                  colorFilter: ColorFilter.matrix(
                    invertVal
                        ? const [
                            -1.0, 0.0, 0.0, 0.0, 255.0,
                            0.0, -1.0, 0.0, 0.0, 255.0,
                            0.0, 0.0, -1.0, 0.0, 255.0,
                            0.0, 0.0, 0.0, 1.0, 0.0,
                          ]
                        : const [
                            1.0, 0.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 1.0, 0.0,
                          ],
                  ),
                  child: Opacity(
                    opacity: brightnessVal.clamp(0.0, 1.0),
                    child: imageWidget,
                  ),
                ),
              ),
            ),
          ),
        ],
      );
    }

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        // Bezel container
        RotatedBox(
          quarterTurns: quarterTurns,
          child: Container(
            width: 160,
            height: 96,
            decoration: BoxDecoration(
              color: oledBgColor,
              border: Border.all(color: const Color(0xFF27273A), width: 6),
              borderRadius: BorderRadius.circular(8),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.5),
                  blurRadius: 16,
                  offset: const Offset(0, 8),
                ),
                BoxShadow(
                  color: oledCyan.withOpacity(0.08),
                  blurRadius: 24,
                  spreadRadius: 2,
                ),
              ],
            ),
            child: ClipRRect(
              borderRadius: BorderRadius.circular(2),
              child: screenContent,
            ),
          ),
        ),
        const SizedBox(height: 12),
        // Simulator Caption label
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              "SCREEN PREVIEW",
              style: TextStyle(
                color: Colors.white.withOpacity(0.6),
                fontSize: 12,
                fontWeight: FontWeight.bold,
                letterSpacing: 0.5,
              ),
            ),
            const SizedBox(width: 6),
            Text(
              "[${widget.activeLabel.toUpperCase()}]",
              style: GoogleFonts.pressStart2p(
                color: Colors.yellow,
                fontSize: 8,
              ),
            ),
          ],
        ),
      ],
    );
  }
}
