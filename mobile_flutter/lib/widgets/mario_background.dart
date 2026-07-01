import 'dart:math';
import 'package:flutter/material.dart';

/// A Mario-themed animated background with scrolling clouds, floating coins,
/// pipe structures, question blocks, and a scrolling ground platform.
class MarioBackground extends StatefulWidget {
  final Widget child;
  const MarioBackground({Key? key, required this.child}) : super(key: key);

  @override
  State<MarioBackground> createState() => _MarioBackgroundState();
}

class _MarioBackgroundState extends State<MarioBackground>
    with TickerProviderStateMixin {
  late AnimationController _scrollController;
  late AnimationController _floatController;
  late AnimationController _coinController;
  late AnimationController _starController;

  final Random _rng = Random(42);

  // Static cloud positions (normalized x 0..1, y 0..1)
  late final List<_Cloud> _clouds;
  // Static coin positions
  late final List<_Coin> _coins;
  // Static star/sparkle positions
  late final List<_Star> _stars;

  @override
  void initState() {
    super.initState();

    _scrollController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 22),
    )..repeat();

    _floatController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2400),
    )..repeat(reverse: true);

    _coinController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 900),
    )..repeat(reverse: true);

    _starController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1600),
    )..repeat(reverse: true);

    _clouds = List.generate(6, (i) => _Cloud(
      x: _rng.nextDouble(),
      y: 0.04 + _rng.nextDouble() * 0.14,
      scale: 0.7 + _rng.nextDouble() * 0.6,
      speed: 0.018 + _rng.nextDouble() * 0.012,
    ));

    _coins = List.generate(8, (i) => _Coin(
      x: _rng.nextDouble(),
      y: 0.18 + _rng.nextDouble() * 0.55,
      phase: _rng.nextDouble(),
    ));

    _stars = List.generate(14, (i) => _Star(
      x: _rng.nextDouble(),
      y: _rng.nextDouble() * 0.9,
      phase: _rng.nextDouble(),
      size: 2.0 + _rng.nextDouble() * 2.5,
    ));
  }

  @override
  void dispose() {
    _scrollController.dispose();
    _floatController.dispose();
    _coinController.dispose();
    _starController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: Listenable.merge([
        _scrollController,
        _floatController,
        _coinController,
        _starController,
      ]),
      builder: (context, _) {
        return Stack(
          children: [
            // ── Sky Gradient ──────────────────────────────────────────────
            Positioned.fill(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.topCenter,
                    end: Alignment.bottomCenter,
                    stops: [0.0, 0.55, 0.55, 1.0],
                    colors: [
                      Color(0xFFBFE5FF), // sky blue
                      Color(0xFFE8F5FF), // light horizon
                      Color(0xFF4CAF50), // grass top strip
                      Color(0xFF388E3C), // grass body
                    ],
                  ),
                ),
              ),
            ),

            // ── Pixel grid overlay (subtle) ───────────────────────────────
            Positioned.fill(
              child: CustomPaint(painter: _PixelGridPainter()),
            ),

            // ── Stars (twinkle) ───────────────────────────────────────────
            ..._stars.map((s) {
              final twinkle = (sin((s.phase + _starController.value) * pi * 2) * 0.5 + 0.5);
              return Positioned(
                left: s.x * MediaQuery.of(context).size.width,
                top: s.y * MediaQuery.of(context).size.height * 0.55,
                child: Opacity(
                  opacity: 0.25 + twinkle * 0.55,
                  child: Container(
                    width: s.size,
                    height: s.size,
                    decoration: const BoxDecoration(
                      shape: BoxShape.circle,
                      color: Color(0xFFFFEB3B),
                    ),
                  ),
                ),
              );
            }),

            // ── Clouds (scrolling) ────────────────────────────────────────
            ..._clouds.map((c) {
              final scrollOffset = (_scrollController.value * c.speed * 2) % 1.4 - 0.2;
              final screenW = MediaQuery.of(context).size.width;
              final screenH = MediaQuery.of(context).size.height;
              final x = ((c.x - scrollOffset + 1.4) % 1.4 - 0.2) * screenW;
              return Positioned(
                left: x,
                top: c.y * screenH,
                child: Transform.scale(
                  scale: c.scale,
                  alignment: Alignment.topLeft,
                  child: const _PixelCloud(),
                ),
              );
            }),

            // ── Question Blocks (floating bob) ────────────────────────────
            ..._buildQuestionBlocks(context),

            // ── Brick blocks ──────────────────────────────────────────────
            ..._buildBrickBlocks(context),

            // ── Coins (floating up/down) ──────────────────────────────────
            ..._coins.map((coin) {
              final bob = sin((coin.phase + _coinController.value) * pi * 2) * 6.0;
              final screenW = MediaQuery.of(context).size.width;
              final screenH = MediaQuery.of(context).size.height;
              return Positioned(
                left: coin.x * screenW,
                top: coin.y * screenH * 0.75 + bob,
                child: const _PixelCoin(),
              );
            }),

            // ── Pipes ─────────────────────────────────────────────────────
            ..._buildPipes(context),

            // ── Ground tile row ───────────────────────────────────────────
            _buildGroundRow(context),

            // ── Content ───────────────────────────────────────────────────
            widget.child,
          ],
        );
      },
    );
  }

  List<Widget> _buildQuestionBlocks(BuildContext context) {
    final screenW = MediaQuery.of(context).size.width;
    final screenH = MediaQuery.of(context).size.height;
    final float = sin(_floatController.value * pi) * 4.0;
    final positions = [
      Offset(screenW * 0.15, screenH * 0.30),
      Offset(screenW * 0.55, screenH * 0.22),
      Offset(screenW * 0.80, screenH * 0.35),
    ];
    return positions.map((p) => Positioned(
      left: p.dx,
      top: p.dy + float,
      child: const _PixelQuestionBlock(),
    )).toList();
  }

  List<Widget> _buildBrickBlocks(BuildContext context) {
    final screenW = MediaQuery.of(context).size.width;
    final screenH = MediaQuery.of(context).size.height;
    final positions = [
      Offset(screenW * 0.28, screenH * 0.31),
      Offset(screenW * 0.36, screenH * 0.31),
      Offset(screenW * 0.62, screenH * 0.24),
      Offset(screenW * 0.70, screenH * 0.24),
    ];
    return positions.map((p) => Positioned(
      left: p.dx,
      top: p.dy,
      child: const _PixelBrick(),
    )).toList();
  }

  List<Widget> _buildPipes(BuildContext context) {
    final screenW = MediaQuery.of(context).size.width;
    final screenH = MediaQuery.of(context).size.height;
    return [
      Positioned(
        left: screenW * 0.04,
        bottom: screenH * 0.06,
        child: const _PixelPipe(height: 80),
      ),
      Positioned(
        right: screenW * 0.06,
        bottom: screenH * 0.06,
        child: const _PixelPipe(height: 56),
      ),
    ];
  }

  Widget _buildGroundRow(BuildContext context) {
    final screenW = MediaQuery.of(context).size.width;
    final screenH = MediaQuery.of(context).size.height;
    final tileSize = 22.0;
    final tileCount = (screenW / tileSize).ceil() + 2;
    final groundY = screenH * 0.54;

    return Positioned(
      left: 0,
      top: groundY - tileSize,
      child: Row(
        children: List.generate(tileCount, (_) => _PixelGroundTile(size: tileSize)),
      ),
    );
  }
}

// ── Data classes ──────────────────────────────────────────────────────────────

class _Cloud {
  final double x, y, scale, speed;
  const _Cloud({required this.x, required this.y, required this.scale, required this.speed});
}

class _Coin {
  final double x, y, phase;
  const _Coin({required this.x, required this.y, required this.phase});
}

class _Star {
  final double x, y, phase, size;
  const _Star({required this.x, required this.y, required this.phase, required this.size});
}

// ── Pixel Grid Painter ────────────────────────────────────────────────────────

class _PixelGridPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = Colors.white.withOpacity(0.04)
      ..strokeWidth = 0.5;
    const step = 20.0;
    for (double x = 0; x < size.width; x += step) {
      canvas.drawLine(Offset(x, 0), Offset(x, size.height * 0.54), paint);
    }
    for (double y = 0; y < size.height * 0.54; y += step) {
      canvas.drawLine(Offset(0, y), Offset(size.width, y), paint);
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Cloud ───────────────────────────────────────────────────────────────

class _PixelCloud extends StatelessWidget {
  const _PixelCloud();

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: const Size(72, 30), painter: _CloudPainter());
  }
}

class _CloudPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final p = Paint()..color = Colors.white;
    // pixel-art cloud shape (grid of 2x2 blocks, 4px each)
    final s = size.width / 18;
    final blocks = [
      // row 0 (top bump)
      [5, 0], [6, 0], [7, 0], [8, 0], [11, 0], [12, 0],
      [4, 1], [5, 1], [6, 1], [7, 1], [8, 1], [9, 1], [10, 1], [11, 1], [12, 1], [13, 1],
      [2, 2], [3, 2], [4, 2], [5, 2], [6, 2], [7, 2], [8, 2], [9, 2], [10, 2], [11, 2], [12, 2], [13, 2], [14, 2], [15, 2],
      [1, 3], [2, 3], [3, 3], [4, 3], [5, 3], [6, 3], [7, 3], [8, 3], [9, 3], [10, 3], [11, 3], [12, 3], [13, 3], [14, 3], [15, 3], [16, 3],
      [0, 4], [1, 4], [2, 4], [3, 4], [4, 4], [5, 4], [6, 4], [7, 4], [8, 4], [9, 4], [10, 4], [11, 4], [12, 4], [13, 4], [14, 4], [15, 4], [16, 4], [17, 4],
      [0, 5], [1, 5], [2, 5], [3, 5], [4, 5], [5, 5], [6, 5], [7, 5], [8, 5], [9, 5], [10, 5], [11, 5], [12, 5], [13, 5], [14, 5], [15, 5], [16, 5], [17, 5],
    ];
    for (final b in blocks) {
      canvas.drawRect(Rect.fromLTWH(b[0] * s, b[1] * s, s - 0.5, s - 0.5), p);
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Question Block ──────────────────────────────────────────────────────

class _PixelQuestionBlock extends StatelessWidget {
  const _PixelQuestionBlock();

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: const Size(26, 26), painter: _QuestionBlockPainter());
  }
}

class _QuestionBlockPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final s = size.width / 8;
    // Background gold
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
        Paint()..color = const Color(0xFFFFC107));
    // Dark border
    final border = Paint()..color = const Color(0xFF795548);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, s), border);
    canvas.drawRect(Rect.fromLTWH(0, size.height - s, size.width, s), border);
    canvas.drawRect(Rect.fromLTWH(0, 0, s, size.height), border);
    canvas.drawRect(Rect.fromLTWH(size.width - s, 0, s, size.height), border);
    // "?" mark white
    final txt = TextPainter(
      text: const TextSpan(
        text: '?',
        style: TextStyle(color: Colors.white, fontSize: 13, fontWeight: FontWeight.bold),
      ),
      textDirection: TextDirection.ltr,
    )..layout();
    txt.paint(canvas, Offset((size.width - txt.width) / 2, (size.height - txt.height) / 2 - 1));
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Brick ───────────────────────────────────────────────────────────────

class _PixelBrick extends StatelessWidget {
  const _PixelBrick();

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: const Size(24, 24), painter: _BrickPainter());
  }
}

class _BrickPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
        Paint()..color = const Color(0xFFE65100));
    final grout = Paint()..color = const Color(0xFFBF360C);
    // horizontal grout lines
    canvas.drawRect(Rect.fromLTWH(0, size.height / 2 - 1, size.width, 2), grout);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, 2), grout);
    canvas.drawRect(Rect.fromLTWH(0, size.height - 2, size.width, 2), grout);
    // vertical lines (alternating)
    canvas.drawRect(Rect.fromLTWH(size.width / 2 - 1, 0, 2, size.height / 2), grout);
    canvas.drawRect(Rect.fromLTWH(size.width / 4 - 1, size.height / 2, 2, size.height / 2), grout);
    canvas.drawRect(Rect.fromLTWH(size.width * 3 / 4 - 1, size.height / 2, 2, size.height / 2), grout);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Coin ────────────────────────────────────────────────────────────────

class _PixelCoin extends StatelessWidget {
  const _PixelCoin();

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: const Size(14, 14), painter: _CoinPainter());
  }
}

class _CoinPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    // Outer gold circle
    canvas.drawCircle(size.center(Offset.zero), size.width / 2,
        Paint()..color = const Color(0xFFFFD600));
    // Inner highlight
    canvas.drawCircle(Offset(size.width * 0.38, size.height * 0.35), size.width * 0.18,
        Paint()..color = Colors.white.withOpacity(0.7));
    // Dollar sign symbol
    final txt = TextPainter(
      text: const TextSpan(
        text: '\$',
        style: TextStyle(color: Color(0xFFFF6F00), fontSize: 8, fontWeight: FontWeight.bold),
      ),
      textDirection: TextDirection.ltr,
    )..layout();
    txt.paint(canvas, Offset((size.width - txt.width) / 2, (size.height - txt.height) / 2 - 0.5));
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Pipe ────────────────────────────────────────────────────────────────

class _PixelPipe extends StatelessWidget {
  final double height;
  const _PixelPipe({required this.height});

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: Size(38, height), painter: _PipePainter());
  }
}

class _PipePainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    // Body
    final body = Paint()..color = const Color(0xFF388E3C);
    canvas.drawRect(Rect.fromLTWH(4, 12, size.width - 8, size.height - 12), body);
    // Highlights
    final highlight = Paint()..color = const Color(0xFF66BB6A);
    canvas.drawRect(Rect.fromLTWH(6, 14, 6, size.height - 16), highlight);
    // Cap (top)
    final cap = Paint()..color = const Color(0xFF2E7D32);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, 14), cap);
    final capHighlight = Paint()..color = const Color(0xFF43A047);
    canvas.drawRect(Rect.fromLTWH(2, 2, 8, 10), capHighlight);
    // Dark border/outline
    final border = Paint()
      ..color = const Color(0xFF1B5E20)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.5;
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, 14), border);
    canvas.drawRect(Rect.fromLTWH(4, 12, size.width - 8, size.height - 12), border);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Pixel Ground Tile ─────────────────────────────────────────────────────────

class _PixelGroundTile extends StatelessWidget {
  final double size;
  const _PixelGroundTile({required this.size});

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: Size(size, size), painter: _GroundTilePainter());
  }
}

class _GroundTilePainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
        Paint()..color = const Color(0xFFE65100));
    final grout = Paint()..color = const Color(0xFFBF360C);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, 2), grout);
    canvas.drawRect(Rect.fromLTWH(0, 0, 2, size.height), grout);
    final highlight = Paint()..color = Colors.white.withOpacity(0.12);
    canvas.drawRect(Rect.fromLTWH(2, 2, size.width * 0.4, 3), highlight);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
