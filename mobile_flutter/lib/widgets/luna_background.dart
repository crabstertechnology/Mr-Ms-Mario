import 'dart:math';
import 'package:flutter/material.dart';

/// Luna-themed animated background — ultra-light, white-dominant.
/// Mr. Luna: white + blue + red accents.
/// Ms. Luna: white + pink accents.
/// Floating pixel elements are very subtle (low opacity) so content remains clear.
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
  late AnimationController _cloudController;
  late AnimationController _floatController;
  late AnimationController _twinkleController;

  late final List<_FloatItem> _clouds;
  late final List<_FloatItem> _coins;
  late final List<_FloatItem> _stars;
  late final List<_FloatItem> _questionBlocks;

  final _rng = Random(7);

  @override
  void initState() {
    super.initState();

    _cloudController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 30),
    )..repeat();

    _floatController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2800),
    )..repeat(reverse: true);

    _twinkleController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1800),
    )..repeat(reverse: true);

    _clouds = List.generate(5, (i) => _FloatItem(
          x: _rng.nextDouble(),
          y: 0.02 + _rng.nextDouble() * 0.10,
          phase: _rng.nextDouble(),
          speed: 0.012 + _rng.nextDouble() * 0.010,
          scale: 0.55 + _rng.nextDouble() * 0.45,
        ));

    _coins = List.generate(6, (i) => _FloatItem(
          x: _rng.nextDouble(),
          y: 0.12 + _rng.nextDouble() * 0.72,
          phase: _rng.nextDouble(),
          speed: 0,
          scale: 0.6 + _rng.nextDouble() * 0.5,
        ));

    _stars = List.generate(16, (i) => _FloatItem(
          x: _rng.nextDouble(),
          y: _rng.nextDouble() * 0.88,
          phase: _rng.nextDouble(),
          speed: 0,
          scale: 1.2 + _rng.nextDouble() * 1.8,
        ));

    _questionBlocks = List.generate(3, (i) => _FloatItem(
          x: 0.06 + i * 0.40 + _rng.nextDouble() * 0.08,
          y: 0.10 + _rng.nextDouble() * 0.07,
          phase: _rng.nextDouble(),
          speed: 0,
          scale: 0.75 + _rng.nextDouble() * 0.35,
        ));
  }

  @override
  void dispose() {
    _cloudController.dispose();
    _floatController.dispose();
    _twinkleController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final size = MediaQuery.of(context).size;

    // Color palette per variant
    final Color primaryAccent =
        widget.isMsLuna ? const Color(0xFFE91E8C) : const Color(0xFF0284C7);
    final Color secondaryAccent =
        widget.isMsLuna ? const Color(0xFFF48FB1) : const Color(0xFFE53935);
    final Color bgEnd =
        widget.isMsLuna ? const Color(0xFFFCE4EC) : const Color(0xFFE3F2FD);

    return AnimatedBuilder(
      animation: Listenable.merge(
          [_cloudController, _floatController, _twinkleController]),
      builder: (ctx, _) {
        return Stack(
          children: [
            // ── 1. WHITE-DOMINANT GRADIENT ────────────────────────────────
            Positioned.fill(
              child: Container(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.topLeft,
                    end: Alignment.bottomRight,
                    stops: const [0.0, 0.6, 1.0],
                    colors: [
                      Colors.white,
                      Colors.white,
                      bgEnd.withOpacity(0.45),
                    ],
                  ),
                ),
              ),
            ),

            // ── 2. TOP EDGE ACCENT BAND ───────────────────────────────────
            Positioned(
              top: 0,
              left: 0,
              right: 0,
              child: Container(
                height: 3,
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    colors: [
                      primaryAccent,
                      secondaryAccent,
                      primaryAccent,
                    ],
                  ),
                ),
              ),
            ),

            // ── 3. SUBTLE GLOW ORBS ───────────────────────────────────────
            Positioned(
              top: -60,
              left: -60,
              child: Container(
                width: 200,
                height: 200,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: RadialGradient(colors: [
                    primaryAccent.withOpacity(0.07),
                    Colors.transparent,
                  ]),
                ),
              ),
            ),
            Positioned(
              bottom: -80,
              right: -60,
              child: Container(
                width: 260,
                height: 260,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: RadialGradient(colors: [
                    secondaryAccent.withOpacity(0.06),
                    Colors.transparent,
                  ]),
                ),
              ),
            ),

            // ── 4. DOT PATTERN ────────────────────────────────────────────
            Positioned.fill(
              child: CustomPaint(painter: _DotPatternPainter()),
            ),

            // ── 5. TWINKLING STARS ────────────────────────────────────────
            ..._stars.map((s) {
              final alpha = 0.10 +
                  sin((s.phase + _twinkleController.value) * pi * 2).abs() *
                      0.28;
              return Positioned(
                left: s.x * size.width,
                top: s.y * size.height,
                child: Opacity(
                  opacity: alpha,
                  child: Container(
                    width: s.scale,
                    height: s.scale,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      color: s.phase > 0.6
                          ? primaryAccent
                          : const Color(0xFFFFD600),
                    ),
                  ),
                ),
              );
            }),

            // ── 6. SCROLLING CLOUDS ───────────────────────────────────────
            ..._clouds.map((c) {
              final scroll =
                  (_cloudController.value * c.speed * 1.6) % 1.4;
              final x =
                  ((c.x - scroll + 1.4) % 1.4 - 0.2) * size.width;
              return Positioned(
                left: x,
                top: c.y * size.height,
                child: Opacity(
                  opacity: 0.50,
                  child: Transform.scale(
                    scale: c.scale,
                    alignment: Alignment.topLeft,
                    child: const _PixelCloud(),
                  ),
                ),
              );
            }),

            // ── 7. FLOATING QUESTION BLOCKS ───────────────────────────────
            ..._questionBlocks.map((b) {
              final bob =
                  sin((b.phase + _floatController.value) * pi * 2) * 4.0;
              return Positioned(
                left: b.x * size.width - 12,
                top: b.y * size.height + bob,
                child: Opacity(
                  opacity: 0.14,
                  child: Transform.scale(
                    scale: b.scale,
                    child: _PixelQuestionBlock(color: primaryAccent),
                  ),
                ),
              );
            }),

            // ── 8. FLOATING COINS ─────────────────────────────────────────
            ..._coins.map((coin) {
              final bob =
                  sin((coin.phase + _floatController.value) * pi * 2) * 6.0;
              return Positioned(
                left: coin.x * size.width,
                top: coin.y * size.height + bob,
                child: Opacity(
                  opacity: 0.18,
                  child: Transform.scale(
                    scale: coin.scale,
                    child: const _PixelCoin(),
                  ),
                ),
              );
            }),

            // ── 9. EDGE PIPES ─────────────────────────────────────────────
            Positioned(
              bottom: 76,
              left: -10,
              child: Opacity(
                opacity: 0.11,
                child: const _PixelPipe(width: 30, height: 54),
              ),
            ),
            Positioned(
              bottom: 76,
              right: -10,
              child: Opacity(
                opacity: 0.11,
                child: const _PixelPipe(width: 28, height: 40),
              ),
            ),

            // ── 10. CONTENT ───────────────────────────────────────────────
            widget.child,
          ],
        );
      },
    );
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Data model
// ─────────────────────────────────────────────────────────────────────────────

class _FloatItem {
  final double x, y, phase, speed, scale;
  const _FloatItem(
      {required this.x,
      required this.y,
      required this.phase,
      required this.speed,
      required this.scale});
}

// ─────────────────────────────────────────────────────────────────────────────
// Dot pattern painter
// ─────────────────────────────────────────────────────────────────────────────

class _DotPatternPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final p = Paint()
      ..color = const Color(0xFF000000).withOpacity(0.015)
      ..style = PaintingStyle.fill;
    const step = 28.0;
    const r = 1.2;
    for (double x = step; x < size.width; x += step) {
      for (double y = step; y < size.height; y += step) {
        canvas.drawCircle(Offset(x, y), r, p);
      }
    }
  }

  @override
  bool shouldRepaint(_) => false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel Cloud
// ─────────────────────────────────────────────────────────────────────────────

class _PixelCloud extends StatelessWidget {
  const _PixelCloud();

  @override
  Widget build(BuildContext context) =>
      CustomPaint(size: const Size(70, 28), painter: _CloudPainter());
}

class _CloudPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final s = size.width / 18.0;
    final p = Paint()..color = Colors.white;
    final blocks = [
      [5, 0], [6, 0], [7, 0], [11, 0], [12, 0],
      [4, 1], [5, 1], [6, 1], [7, 1], [8, 1], [9, 1], [10, 1], [11, 1], [12, 1], [13, 1],
      [2, 2], [3, 2], [4, 2], [5, 2], [6, 2], [7, 2], [8, 2], [9, 2], [10, 2], [11, 2], [12, 2], [13, 2], [14, 2], [15, 2],
      [0, 3], [1, 3], [2, 3], [3, 3], [4, 3], [5, 3], [6, 3], [7, 3], [8, 3], [9, 3], [10, 3], [11, 3], [12, 3], [13, 3], [14, 3], [15, 3], [16, 3], [17, 3],
      [0, 4], [1, 4], [2, 4], [3, 4], [4, 4], [5, 4], [6, 4], [7, 4], [8, 4], [9, 4], [10, 4], [11, 4], [12, 4], [13, 4], [14, 4], [15, 4], [16, 4], [17, 4],
    ];
    for (final b in blocks) {
      canvas.drawRect(
          Rect.fromLTWH(b[0] * s, b[1] * s, s - 0.8, s - 0.8), p);
    }
  }

  @override
  bool shouldRepaint(_) => false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel Question Block
// ─────────────────────────────────────────────────────────────────────────────

class _PixelQuestionBlock extends StatelessWidget {
  final Color color;
  const _PixelQuestionBlock({this.color = const Color(0xFFFFB300)});

  @override
  Widget build(BuildContext context) =>
      CustomPaint(size: const Size(26, 26), painter: _QBlockPainter(color));
}

class _QBlockPainter extends CustomPainter {
  final Color color;
  const _QBlockPainter(this.color);

  @override
  void paint(Canvas canvas, Size size) {
    final s = size.width / 8;
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
        Paint()..color = const Color(0xFFFFB300));
    final border = Paint()..color = const Color(0xFF6D4C41);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, s), border);
    canvas.drawRect(
        Rect.fromLTWH(0, size.height - s, size.width, s), border);
    canvas.drawRect(Rect.fromLTWH(0, 0, s, size.height), border);
    canvas.drawRect(
        Rect.fromLTWH(size.width - s, 0, s, size.height), border);
    final tp = TextPainter(
      text: const TextSpan(
          text: '?',
          style: TextStyle(
              color: Colors.white,
              fontSize: 13,
              fontWeight: FontWeight.w900)),
      textDirection: TextDirection.ltr,
    )..layout();
    tp.paint(canvas,
        Offset((size.width - tp.width) / 2, (size.height - tp.height) / 2 - 1));
  }

  @override
  bool shouldRepaint(_) => false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel Coin
// ─────────────────────────────────────────────────────────────────────────────

class _PixelCoin extends StatelessWidget {
  const _PixelCoin();

  @override
  Widget build(BuildContext context) =>
      CustomPaint(size: const Size(16, 16), painter: _CoinPainter());
}

class _CoinPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final c = size.center(Offset.zero);
    final r = size.width / 2;
    canvas.drawCircle(c, r, Paint()..color = const Color(0xFFFFD600));
    canvas.drawCircle(c, r * 0.70, Paint()..color = const Color(0xFFFF8F00));
    canvas.drawCircle(Offset(c.dx - r * 0.22, c.dy - r * 0.22), r * 0.20,
        Paint()..color = Colors.white.withOpacity(0.75));
  }

  @override
  bool shouldRepaint(_) => false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pixel Pipe
// ─────────────────────────────────────────────────────────────────────────────

class _PixelPipe extends StatelessWidget {
  final double width, height;
  const _PixelPipe({required this.width, required this.height});

  @override
  Widget build(BuildContext context) =>
      CustomPaint(size: Size(width, height), painter: _PipePainter());
}

class _PipePainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    canvas.drawRect(Rect.fromLTWH(4, 14, size.width - 8, size.height - 14),
        Paint()..color = const Color(0xFF388E3C));
    canvas.drawRect(Rect.fromLTWH(6, 16, 5, size.height - 18),
        Paint()..color = const Color(0xFF66BB6A));
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, 15),
        Paint()..color = const Color(0xFF2E7D32));
    canvas.drawRect(Rect.fromLTWH(2, 2, 8, 11),
        Paint()..color = const Color(0xFF43A047));
  }

  @override
  bool shouldRepaint(_) => false;
}
