import 'dart:ui' as ui;
import 'package:flutter/material.dart';

/// Next-Gen Frosted Cyber-Glass Card for SYNAPS UI
/// Features subtle gradient border reflections, backdrop blur filter,
/// soft chromatic elevation shadows, and optional neural circuit texture.
class GlassCard extends StatelessWidget {
  final Widget child;
  final EdgeInsetsGeometry padding;
  final double borderRadius;
  final Border? border;
  final Color? backgroundColor;
  final bool hasTexture;

  const GlassCard({
    Key? key,
    required this.child,
    this.padding = const EdgeInsets.all(20),
    this.borderRadius = 20,
    this.border,
    this.backgroundColor,
    this.hasTexture = true,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final effectiveBg = backgroundColor ?? Colors.white.withOpacity(0.85);

    return Container(
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(borderRadius),
        boxShadow: [
          // Soft ambient drop shadow
          BoxShadow(
            color: const Color(0xFF0F172A).withOpacity(0.04),
            blurRadius: 20,
            spreadRadius: 0,
            offset: const Offset(0, 6),
          ),
          // Subtle neon rim depth
          BoxShadow(
            color: const Color(0xFF00E5FF).withOpacity(0.03),
            blurRadius: 10,
            spreadRadius: 0,
            offset: const Offset(0, 2),
          ),
        ],
      ),
      child: ClipRRect(
        borderRadius: BorderRadius.circular(borderRadius),
        child: BackdropFilter(
          filter: ui.ImageFilter.blur(sigmaX: 12, sigmaY: 12),
          child: Container(
            decoration: BoxDecoration(
              color: effectiveBg,
              borderRadius: BorderRadius.circular(borderRadius),
              border: border ??
                  Border.all(
                    color: Colors.white.withOpacity(0.85),
                    width: 1.2,
                  ),
            ),
            child: Stack(
              children: [
                // Top subtle gradient light reflection shimmer
                Positioned(
                  top: 0,
                  left: 0,
                  right: 0,
                  height: 28,
                  child: Container(
                    decoration: BoxDecoration(
                      gradient: LinearGradient(
                        begin: Alignment.topCenter,
                        end: Alignment.bottomCenter,
                        colors: [
                          Colors.white.withOpacity(0.40),
                          Colors.white.withOpacity(0.0),
                        ],
                      ),
                    ),
                  ),
                ),

                // High-tech subtle dot-matrix texture
                if (hasTexture)
                  Positioned.fill(
                    child: CustomPaint(
                      painter: _CyberMatrixPainter(
                        color: const Color(0xFF0F172A).withOpacity(0.015),
                        spacing: 14.0,
                        dotRadius: 0.8,
                      ),
                    ),
                  ),

                Padding(
                  padding: padding,
                  child: child,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class _CyberMatrixPainter extends CustomPainter {
  final Color color;
  final double spacing;
  final double dotRadius;

  _CyberMatrixPainter({
    required this.color,
    required this.spacing,
    required this.dotRadius,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = color
      ..style = PaintingStyle.fill;

    for (double x = spacing; x < size.width; x += spacing) {
      for (double y = spacing; y < size.height; y += spacing) {
        canvas.drawCircle(Offset(x, y), dotRadius, paint);
      }
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
