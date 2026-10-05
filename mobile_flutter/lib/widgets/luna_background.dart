import 'dart:math';
import 'package:flutter/material.dart';

/// SYNAPS Character Squad Animated Background
/// Features the 6 SYNAPS characters floating gracefully in the background,
/// soft character squad watermark banner, floating colorful bubbles,
/// and smooth ambient light orbs while keeping foreground content ultra-readable.
class LunaBackground extends StatefulWidget {
  final Widget child;
  final bool isMsLuna;

  const LunaBackground({
    Key? key,
    required this.child,
    this.isMsLuna = false,
  }) : super(key: key);

  @override
  State<LunaBackground> createState() => _LunaBackgroundState();
}

class _LunaBackgroundState extends State<LunaBackground>
    with TickerProviderStateMixin {
  late AnimationController _floatController;
  late AnimationController _driftController;
  late AnimationController _twinkleController;

  final List<Map<String, dynamic>> _characters = [
    {'name': 'bunny', 'asset': 'assets/characters/bunny.png', 'color': Color(0xFF00E5FF), 'x': 0.08, 'y': 0.12, 'size': 54.0},
    {'name': 'fox', 'asset': 'assets/characters/fox.png', 'color': Color(0xFFFF7A00), 'x': 0.82, 'y': 0.18, 'size': 50.0},
    {'name': 'turtle', 'asset': 'assets/characters/turtle.png', 'color': Color(0xFF10B981), 'x': 0.06, 'y': 0.46, 'size': 48.0},
    {'name': 'panda', 'asset': 'assets/characters/panda.png', 'color': Color(0xFF6366F1), 'x': 0.85, 'y': 0.52, 'size': 56.0},
    {'name': 'cat', 'asset': 'assets/characters/cat.png', 'color': Color(0xFF9333EA), 'x': 0.12, 'y': 0.80, 'size': 52.0},
    {'name': 'bird', 'asset': 'assets/characters/bird.png', 'color': Color(0xFF38BDF8), 'x': 0.82, 'y': 0.82, 'size': 50.0},
  ];

  @override
  void initState() {
    super.initState();

    _floatController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 4),
    )..repeat(reverse: true);

    _driftController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 20),
    )..repeat();

    _twinkleController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2400),
    )..repeat(reverse: true);
  }

  @override
  void dispose() {
    _floatController.dispose();
    _driftController.dispose();
    _twinkleController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final size = MediaQuery.of(context).size;

    return AnimatedBuilder(
      animation: Listenable.merge([_floatController, _driftController, _twinkleController]),
      builder: (context, _) {
        final floatVal = sin(_floatController.value * 2 * pi);

        return Stack(
          children: [
            // ── 1. CLEAN LUMINOUS WHITE BACKGROUND ─────────────────────
            Positioned.fill(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.topCenter,
                    end: Alignment.bottomCenter,
                    colors: [
                      Colors.white,
                      Color(0xFFF8FAFC),
                      Color(0xFFF1F5F9),
                    ],
                  ),
                ),
              ),
            ),

            // ── 2. ARTISTIC SYNAPS CHARACTERS WATERMARK BANNER ─────────
            // Softly visible behind content with gradient fade
            Positioned(
              top: 40 + floatVal * 8,
              left: 0,
              right: 0,
              height: size.height * 0.38,
              child: Opacity(
                opacity: 0.14,
                child: ShaderMask(
                  shaderCallback: (rect) {
                    return const LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      stops: [0.0, 0.6, 1.0],
                      colors: [Colors.black, Colors.black, Colors.transparent],
                    ).createShader(rect);
                  },
                  blendMode: BlendMode.dstIn,
                  child: Image.asset(
                    'assets/characters/characters_banner.png',
                    fit: BoxFit.contain,
                  ),
                ),
              ),
            ),

            // ── 3. AMBIENT CHARACTER CHROMATIC AURA ORBS ───────────────
            Positioned(
              top: -40,
              left: -40,
              child: Container(
                width: 240,
                height: 240,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: RadialGradient(
                    colors: [
                      const Color(0xFF00E5FF).withOpacity(0.08),
                      Colors.transparent,
                    ],
                  ),
                ),
              ),
            ),
            Positioned(
              bottom: 80,
              right: -50,
              child: Container(
                width: 260,
                height: 260,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: RadialGradient(
                    colors: [
                      const Color(0xFF9333EA).withOpacity(0.07),
                      Colors.transparent,
                    ],
                  ),
                ),
              ),
            ),

            // ── 4. FLOATING CHARACTERS SQUAD BUBBLES IN BACKGROUND ─────
            ..._characters.asMap().entries.map((entry) {
              final idx = entry.key;
              final c = entry.value;
              final double phase = (idx * 1.05);
              final double offsetY = sin((_floatController.value * 2 * pi) + phase) * 12;
              final double offsetX = cos((_driftController.value * 2 * pi) + phase) * 8;

              final double posX = (c['x'] as double) * size.width + offsetX;
              final double posY = (c['y'] as double) * size.height + offsetY;
              final double bubbleSize = c['size'] as double;
              final Color glowColor = c['color'] as Color;

              return Positioned(
                left: posX.clamp(0.0, size.width - bubbleSize),
                top: posY.clamp(0.0, size.height - bubbleSize),
                child: Opacity(
                  opacity: 0.35 + (_twinkleController.value * 0.15),
                  child: Container(
                    width: bubbleSize,
                    height: bubbleSize,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      color: Colors.white.withOpacity(0.75),
                      border: Border.all(
                        color: glowColor.withOpacity(0.4),
                        width: 1.5,
                      ),
                      boxShadow: [
                        BoxShadow(
                          color: glowColor.withOpacity(0.20),
                          blurRadius: 10,
                          offset: const Offset(0, 3),
                        ),
                      ],
                    ),
                    child: ClipOval(
                      child: Image.asset(
                        c['asset'] as String,
                        fit: BoxFit.cover,
                      ),
                    ),
                  ),
                ),
              );
            }),

            // ── 5. SPARKLING PLAYFUL PARTICLES ─────────────────────────
            Positioned.fill(
              child: CustomPaint(
                painter: _PlayfulSparklesPainter(
                  time: _driftController.value,
                  twinkle: _twinkleController.value,
                ),
              ),
            ),

            // ── 6. FOREGROUND CONTENT ──────────────────────────────────
            widget.child,
          ],
        );
      },
    );
  }
}

class _PlayfulSparklesPainter extends CustomPainter {
  final double time;
  final double twinkle;

  _PlayfulSparklesPainter({
    required this.time,
    required this.twinkle,
  });

  static final List<Offset> _baseStars = [
    const Offset(0.22, 0.18),
    const Offset(0.75, 0.14),
    const Offset(0.30, 0.40),
    const Offset(0.68, 0.38),
    const Offset(0.18, 0.65),
    const Offset(0.85, 0.70),
    const Offset(0.45, 0.88),
  ];

  static final List<Color> _starColors = [
    Color(0xFF00E5FF),
    Color(0xFFFF9800),
    Color(0xFF10B981),
    Color(0xFF8B5CF6),
    Color(0xFFFF2A85),
    Color(0xFF38BDF8),
    Color(0xFFFBBF24),
  ];

  @override
  void paint(Canvas canvas, Size size) {
    for (int i = 0; i < _baseStars.length; i++) {
      final base = _baseStars[i];
      final color = _starColors[i % _starColors.length];

      final double px = (base.dx * size.width + sin(time * 2 * pi + i) * 6) % size.width;
      final double py = (base.dy * size.height + cos(time * 2 * pi + i) * 6) % size.height;

      final double scale = 1.0 + 0.3 * sin(twinkle * 2 * pi + i);
      final double alpha = 0.20 + 0.15 * sin(twinkle * pi + i);

      final paint = Paint()
        ..color = color.withOpacity(alpha)
        ..style = PaintingStyle.fill;

      // Draw little four-point sparkle
      _drawSparkle(canvas, Offset(px, py), 4.5 * scale, paint);
    }
  }

  void _drawSparkle(Canvas canvas, Offset center, double r, Paint paint) {
    final path = Path();
    path.moveTo(center.dx, center.dy - r);
    path.quadraticBezierTo(center.dx, center.dy, center.dx + r, center.dy);
    path.quadraticBezierTo(center.dx, center.dy, center.dx, center.dy + r);
    path.quadraticBezierTo(center.dx, center.dy, center.dx - r, center.dy);
    path.quadraticBezierTo(center.dx, center.dy, center.dx, center.dy - r);
    path.close();
    canvas.drawPath(path, paint);
  }

  @override
  bool shouldRepaint(covariant _PlayfulSparklesPainter oldDelegate) => true;
}
