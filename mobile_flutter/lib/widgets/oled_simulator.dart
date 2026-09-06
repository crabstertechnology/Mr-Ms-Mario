import 'dart:async';
import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import '../services/database_service.dart';
import '../models/gif_model.dart';
import 'package:gif/gif.dart';

class OLEDSimulator extends StatefulWidget {
  final String activeGifId;
  final String activeLabel;
  final String? marqueeText; // If set, displays scrolling marquee instead of GIF
  final bool? invertColor; // If set, overrides db.oledInvert
  final Uint8List? wallpaperBytes; // Selected wallpaper preview bytes

  const OLEDSimulator({
    Key? key,
    required this.activeGifId,
    required this.activeLabel,
    this.marqueeText,
    this.invertColor,
    this.wallpaperBytes,
  }) : super(key: key);

  @override
  State<OLEDSimulator> createState() => _OLEDSimulatorState();
}

class _OLEDSimulatorState extends State<OLEDSimulator> with TickerProviderStateMixin {
  AnimationController? _marqueeController;
  late Animation<double> _marqueeAnimation;
  double _textWidth = 0.0;
  bool _isMarqueeActive = false;

  int _gifResetCounter = 0;
  Timer? _refreshTimer;

  // GIF cycling support
  int _cycleIndex = 0;
  Timer? _cycleTimer;

  late GifController _gifController;

  @override
  void initState() {
    super.initState();
    _gifController = GifController(vsync: this);
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

    _checkCycleTimer();

    // 1-second general refresh timer for clock display
    _refreshTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      if (mounted && widget.activeGifId == 'clock') {
        setState(() {});
      }
    });
  }

  void _checkCycleTimer() {
    final isCycling = widget.activeLabel.toUpperCase() == "CYCLING ALL GIFS";
    if (isCycling) {
      if (_cycleTimer == null) {
        _cycleTimer = Timer.periodic(const Duration(seconds: 3), (timer) {
          if (mounted) {
            setState(() {
              _cycleIndex++;
            });
          }
        });
      }
    } else {
      _cycleTimer?.cancel();
      _cycleTimer = null;
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
  void didUpdateWidget(covariant OLEDSimulator oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.marqueeText != oldWidget.marqueeText &&
        widget.marqueeText != null &&
        widget.marqueeText!.isNotEmpty) {
      _startMarquee();
    }
    _checkCycleTimer();
  }

  @override
  void dispose() {
    _cycleTimer?.cancel();
    _refreshTimer?.cancel();
    _marqueeController?.dispose();
    _gifController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);

    // Apply brightness, contrast (opacity/blending), invert, and rotation
    final brightnessVal = db.oledBrightness;
    final invertVal = widget.invertColor ?? db.oledInvert;
    final rotationVal = db.oledRotation; // in degrees (0, 90, 180, 270)

    final isMiss = db.primaryRobot?.variant == 'ms_luna';
    final oledThemeColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFF00F0FF);
    final oledColor = invertVal ? oledThemeColor : Colors.white;
    final oledBgColor = invertVal ? Colors.white : oledThemeColor;

    // Transform degrees to quarter turns for RotatedBox
    int quarterTurns = (rotationVal / 90.0).round() % 4;

    Widget screenContent;
    if (widget.activeGifId == 'clock') {
      final now = DateTime.now();
      final List<String> weekdays = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
      final List<String> months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
      final weekday = weekdays[now.weekday - 1];
      final month = months[now.month - 1];
      final dateStr = "${now.day.toString().padLeft(2, '0')} $month";
      final dayDateStr = "$weekday, $dateStr";

      if (db.clockStyle == 1) {
        // STYLE 1: Minimalist
        String timeNoSec;
        int hour = now.hour;
        if (db.is12HourFormat) {
          hour = now.hour % 12;
          if (hour == 0) hour = 12;
        }
        timeNoSec = "${hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
        final suffix = db.is12HourFormat
            ? (now.hour >= 12 ? 'PM' : 'AM')
            : now.second.toString().padLeft(2, '0');

        screenContent = Center(
          child: SizedBox(
            width: 128,
            height: 64,
            child: Stack(
              children: [
                Positioned(
                  left: 10,
                  top: 12,
                  child: Row(
                    crossAxisAlignment: CrossAxisAlignment.baseline,
                    textBaseline: TextBaseline.alphabetic,
                    children: [
                      Text(
                        timeNoSec,
                        style: GoogleFonts.pressStart2p(
                          color: oledColor,
                          fontSize: 14,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(width: 4),
                      Text(
                        suffix,
                        style: GoogleFonts.pressStart2p(
                          color: oledColor,
                          fontSize: 6,
                        ),
                      ),
                    ],
                  ),
                ),
                Positioned(
                  left: 10,
                  bottom: 12,
                  child: Text(
                    dayDateStr,
                    style: GoogleFonts.pressStart2p(
                      color: oledColor,
                      fontSize: 6,
                    ),
                  ),
                ),
              ],
            ),
          ),
        );
      } else if (db.clockStyle == 2) {
        // STYLE 2: Analog Split
        String digitalTime;
        int hour = now.hour;
        if (db.is12HourFormat) {
          hour = now.hour % 12;
          if (hour == 0) hour = 12;
          digitalTime = "${hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
        } else {
          digitalTime = "${hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
        }

        screenContent = Center(
          child: SizedBox(
            width: 128,
            height: 64,
            child: Row(
              children: [
                const SizedBox(width: 8),
                CustomPaint(
                  size: const Size(44, 44),
                  painter: AnalogClockPainter(now, oledColor),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        digitalTime,
                        style: GoogleFonts.pressStart2p(
                          color: oledColor,
                          fontSize: 8,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        weekday,
                        style: GoogleFonts.pressStart2p(
                          color: oledColor,
                          fontSize: 6,
                        ),
                      ),
                      const SizedBox(height: 2),
                      Text(
                        dateStr,
                        style: GoogleFonts.pressStart2p(
                          color: oledColor,
                          fontSize: 5,
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
          ),
        );
      } else if (db.clockStyle == 3) {
        // STYLE 3: Custom Photo Wallpaper Clock
        String timeStr;
        if (db.is12HourFormat) {
          int hour = now.hour % 12;
          if (hour == 0) hour = 12;
          final ampm = now.hour >= 12 ? 'PM' : 'AM';
          final mm = now.minute.toString().padLeft(2, '0');
          timeStr = "$hour:$mm $ampm";
        } else {
          final hh = now.hour.toString().padLeft(2, '0');
          final mm = now.minute.toString().padLeft(2, '0');
          final ss = now.second.toString().padLeft(2, '0');
          timeStr = "$hh:$mm:$ss";
        }

        final hasWallpaper = widget.wallpaperBytes != null;

        screenContent = Center(
          child: Container(
            width: 128,
            height: 64,
            decoration: BoxDecoration(
              color: hasWallpaper ? Colors.black : const Color(0xFF1E293B),
              image: hasWallpaper
                  ? DecorationImage(
                      image: MemoryImage(widget.wallpaperBytes!),
                      fit: BoxFit.cover,
                    )
                  : const DecorationImage(
                      image: AssetImage('assets/logo.png'),
                      fit: BoxFit.contain,
                      opacity: 0.15,
                    ),
            ),
            child: Stack(
              children: [
                Positioned(
                  top: 10,
                  left: 0,
                  right: 0,
                  child: Center(
                    child: Text(
                      timeStr,
                      style: GoogleFonts.pressStart2p(
                        color: Colors.white,
                        fontSize: 8,
                        fontWeight: FontWeight.bold,
                        shadows: const [
                          Shadow(color: Colors.black, offset: Offset(1, 1), blurRadius: 1),
                        ],
                      ),
                    ),
                  ),
                ),
                Positioned(
                  bottom: 10,
                  left: 14,
                  right: 14,
                  child: Container(
                    padding: const EdgeInsets.symmetric(vertical: 2),
                    decoration: BoxDecoration(
                      color: (db.primaryRobot?.variant == 'mr_luna' ? const Color(0xFF0074D9) : const Color(0xFFEC4899)).withOpacity(0.5),
                      borderRadius: BorderRadius.circular(4),
                    ),
                    child: Center(
                      child: Text(
                        dayDateStr,
                        style: GoogleFonts.pressStart2p(
                          color: Colors.white,
                          fontSize: 5,
                        ),
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
        );
      } else {
        // STYLE 0: Classic Border
        String timeStr;
        if (db.is12HourFormat) {
          int hour = now.hour % 12;
          if (hour == 0) hour = 12;
          final ampm = now.hour >= 12 ? 'PM' : 'AM';
          final mm = now.minute.toString().padLeft(2, '0');
          timeStr = "$hour:$mm $ampm";
        } else {
          final hh = now.hour.toString().padLeft(2, '0');
          final mm = now.minute.toString().padLeft(2, '0');
          final ss = now.second.toString().padLeft(2, '0');
          timeStr = "$hh:$mm:$ss";
        }

        screenContent = Center(
          child: Container(
            width: 128,
            height: 64,
            alignment: Alignment.center,
            decoration: BoxDecoration(
              border: Border.all(color: oledColor, width: 1.2),
              borderRadius: BorderRadius.circular(4),
            ),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(
                  timeStr,
                  style: GoogleFonts.pressStart2p(
                    color: oledColor,
                    fontSize: 10,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(height: 6),
                Text(
                  dayDateStr,
                  style: GoogleFonts.pressStart2p(
                    color: oledColor,
                    fontSize: 6,
                  ),
                ),
              ],
            ),
          ),
        );
      }
    } else if (_isMarqueeActive && widget.marqueeText != null) {
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
                      softWrap: false,
                      maxLines: 1,
                      overflow: TextOverflow.visible,
                      style: GoogleFonts.pressStart2p(
                        color: oledColor,
                        fontSize: 9,
                        fontWeight: FontWeight.bold,
                        shadows: [
                          Shadow(
                            color: oledThemeColor.withOpacity(0.6),
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
      // Sprite AI / Face Expression Renderer for 1.69" 240x280 display
      final isCycling = widget.activeLabel.toUpperCase() == "CYCLING ALL GIFS";
      final GifModel currentGif;

      if (isCycling && db.gifs.isNotEmpty) {
        final cycleGifs = db.gifs.where((g) => g.id != "clock").toList();
        if (cycleGifs.isNotEmpty) {
          currentGif = cycleGifs[_cycleIndex % cycleGifs.length];
        } else {
          currentGif = db.gifs.isNotEmpty ? db.gifs[_cycleIndex % db.gifs.length] : GifModel(id: 'sprite_ai_0', name: 'Sprite AI 1', category: 'Sprite AI', size: 0, flashSize: '0KB');
        }
      } else {
        currentGif = db.gifs.firstWhere((g) => g.id == widget.activeGifId, orElse: () => db.gifs.isNotEmpty ? db.gifs.first : GifModel(id: 'sprite_ai_0', name: 'Sprite AI 1', category: 'Sprite AI', size: 0, flashSize: '0KB'));
      }
      
      Widget faceWidget;
      if (currentGif.customData != null && currentGif.customData!.isNotEmpty) {
        try {
          final base64Str = currentGif.customData!.split(',').last;
          final bytes = base64Decode(base64Str);
          faceWidget = Image.memory(bytes, fit: BoxFit.contain);
        } catch (e) {
          faceWidget = const Icon(Icons.broken_image, color: Colors.red);
        }
      } else {
        // Built-in Sprite AI animation rendering (240x240 RGB565 face on 240x280 TFT)
        faceWidget = CustomPaint(
          size: const Size(240, 240),
          painter: SpriteAIEyePainter(
            animId: currentGif.id,
            eyeColor: oledThemeColor,
            cycleIndex: _cycleIndex,
          ),
        );
      }

      screenContent = Stack(
        alignment: Alignment.center,
        children: [
          // Background dark container
          Container(color: Colors.black),
          // Glowing Cyan text label overlay
          Positioned(
            top: 10,
            left: 0,
            right: 0,
            child: Center(
              child: Text(
                widget.activeLabel.toUpperCase(),
                style: GoogleFonts.pressStart2p(
                  color: oledThemeColor,
                  fontSize: 7,
                  shadows: [
                    Shadow(
                      color: oledThemeColor.withOpacity(0.8),
                      blurRadius: 4,
                    ),
                  ],
                ),
              ),
            ),
          ),
          // 240x240 Robot Eye Face Display
          Center(
            child: SizedBox(
              width: 200,
              height: 200,
              child: faceWidget,
            ),
          ),
        ],
      );
    }

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        // 1.69" 240x280 TFT Display Bezel container
        RotatedBox(
          quarterTurns: quarterTurns,
          child: Container(
            width: 200,
            height: 233, // 240x280 aspect ratio (1:1.166)
            decoration: BoxDecoration(
              color: Colors.black,
              border: Border.all(color: const Color(0xFF27273A), width: 7),
              borderRadius: BorderRadius.circular(12),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.6),
                  blurRadius: 18,
                  offset: const Offset(0, 8),
                ),
                BoxShadow(
                  color: oledThemeColor.withOpacity(0.12),
                  blurRadius: 28,
                  spreadRadius: 2,
                ),
              ],
            ),
            child: ClipRRect(
              borderRadius: BorderRadius.circular(6),
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

class AnalogClockPainter extends CustomPainter {
  final DateTime time;
  final Color color;
  AnalogClockPainter(this.time, this.color);

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width / 2;
    
    final paint = Paint()
      ..color = color
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.0;

    // Draw outer circle
    canvas.drawCircle(center, radius, paint);
    
    // Draw center dot
    final dotPaint = Paint()
      ..color = color
      ..style = PaintingStyle.fill;
    canvas.drawCircle(center, 1.5, dotPaint);

    // Calculate angles
    final double angleHour = (time.hour % 12 + time.minute / 60.0) * 30.0 * 3.14159 / 180.0;
    final double angleMin = (time.minute + time.second / 60.0) * 6.0 * 3.14159 / 180.0;
    final double angleSec = time.second * 6.0 * 3.14159 / 180.0;

    // Hour hand
    final hourLength = radius * 0.5;
    canvas.drawLine(
      center,
      Offset(center.dx + hourLength * sin(angleHour), center.dy - hourLength * cos(angleHour)),
      paint..strokeWidth = 1.5,
    );

    // Minute hand
    final minLength = radius * 0.75;
    canvas.drawLine(
      center,
      Offset(center.dx + minLength * sin(angleMin), center.dy - minLength * cos(angleMin)),
      paint..strokeWidth = 1.0,
    );

    // Second hand
    final secLength = radius * 0.85;
    final secPaint = Paint()
      ..color = color
      ..strokeWidth = 0.5;
    canvas.drawLine(
      center,
      Offset(center.dx + secLength * sin(angleSec), center.dy - secLength * cos(angleSec)),
      secPaint,
    );
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}

class SpriteAIEyePainter extends CustomPainter {
  final String animId;
  final Color eyeColor;
  final int cycleIndex;

  SpriteAIEyePainter({
    required this.animId,
    required this.eyeColor,
    required this.cycleIndex,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final centerLeft = Offset(size.width * 0.32, size.height * 0.5);
    final centerRight = Offset(size.width * 0.68, size.height * 0.5);
    final eyeWidth = size.width * 0.24;
    final eyeHeight = size.height * 0.36;

    final paint = Paint()
      ..color = eyeColor
      ..style = PaintingStyle.fill;

    final glowPaint = Paint()
      ..color = eyeColor.withOpacity(0.3)
      ..style = PaintingStyle.fill
      ..maskFilter = const MaskFilter.blur(BlurStyle.normal, 12);

    if (animId == 'sprite_ai_2') {
      // Happy eyes / Arc
      final pathLeft = Path()
        ..addArc(Rect.fromCenter(center: centerLeft, width: eyeWidth * 1.1, height: eyeHeight * 0.9), 3.14, 3.14);
      final pathRight = Path()
        ..addArc(Rect.fromCenter(center: centerRight, width: eyeWidth * 1.1, height: eyeHeight * 0.9), 3.14, 3.14);
      
      final strokePaint = Paint()
        ..color = eyeColor
        ..style = PaintingStyle.stroke
        ..strokeWidth = 14
        ..strokeCap = StrokeCap.round;

      canvas.drawPath(pathLeft, glowPaint);
      canvas.drawPath(pathRight, glowPaint);
      canvas.drawPath(pathLeft, strokePaint);
      canvas.drawPath(pathRight, strokePaint);
    } else if (animId == 'sprite_ai_1') {
      // Looking / Expressive shifted eyes
      final shiftX = (cycleIndex % 2 == 0) ? -10.0 : 10.0;
      final rectLeft = RRect.fromRectAndRadius(
        Rect.fromCenter(center: Offset(centerLeft.dx + shiftX, centerLeft.dy), width: eyeWidth, height: eyeHeight),
        const Radius.circular(16),
      );
      final rectRight = RRect.fromRectAndRadius(
        Rect.fromCenter(center: Offset(centerRight.dx + shiftX, centerRight.dy), width: eyeWidth, height: eyeHeight),
        const Radius.circular(16),
      );

      canvas.drawRRect(rectLeft, glowPaint);
      canvas.drawRRect(rectRight, glowPaint);
      canvas.drawRRect(rectLeft, paint);
      canvas.drawRRect(rectRight, paint);
    } else {
      // Default Sprite AI Blink / Oval robot eyes
      final rectLeft = RRect.fromRectAndRadius(
        Rect.fromCenter(center: centerLeft, width: eyeWidth, height: eyeHeight),
        const Radius.circular(20),
      );
      final rectRight = RRect.fromRectAndRadius(
        Rect.fromCenter(center: centerRight, width: eyeWidth, height: eyeHeight),
        const Radius.circular(20),
      );

      canvas.drawRRect(rectLeft, glowPaint);
      canvas.drawRRect(rectRight, glowPaint);
      canvas.drawRRect(rectLeft, paint);
      canvas.drawRRect(rectRight, paint);
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}
