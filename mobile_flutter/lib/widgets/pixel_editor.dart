import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:google_fonts/google_fonts.dart';

class PixelEditor extends StatefulWidget {
  const PixelEditor({Key? key}) : super(key: key);

  @override
  State<PixelEditor> createState() => _PixelEditorState();
}

class _PixelEditorState extends State<PixelEditor> {
  // 16 cols x 8 rows matrix. false = black, true = white
  late List<List<bool>> _grid;
  bool _isDrawingMode = true; // true = draw (white), false = erase (black)
  final TextEditingController _codeController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _resetGrid();
    _generateCppCode();
  }

  void _resetGrid() {
    _grid = List.generate(8, (_) => List.generate(16, (_) => false));
  }

  void _clearEditor() {
    setState(() {
      _resetGrid();
      _generateCppCode();
    });
  }

  void _invertEditor() {
    setState(() {
      for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 16; x++) {
          _grid[y][x] = !_grid[y][x];
        }
      }
      _generateCppCode();
    });
  }

  void _handlePaint(Offset localPosition, BoxConstraints constraints) {
    final cellWidth = constraints.maxWidth / 16;
    final cellHeight = constraints.maxHeight / 8;

    final x = (localPosition.dx / cellWidth).floor().clamp(0, 15);
    final y = (localPosition.dy / cellHeight).floor().clamp(0, 7);

    if (_grid[y][x] != _isDrawingMode) {
      setState(() {
        _grid[y][x] = _isDrawingMode;
        _generateCppCode();
      });
    }
  }

  void _generateCppCode() {
    // 16 columns by 8 rows. Each row represents 2 bytes. Total 16 bytes.
    final bytes = List<int>.filled(16, 0);

    for (int y = 0; y < 8; y++) {
      int byte0 = 0;
      int byte1 = 0;

      for (int x = 0; x < 8; x++) {
        if (_grid[y][x]) {
          byte0 |= (1 << (7 - x));
        }
      }
      for (int x = 8; x < 16; x++) {
        if (_grid[y][x]) {
          byte1 |= (1 << (15 - x));
        }
      }

      bytes[y * 2] = byte0;
      bytes[y * 2 + 1] = byte1;
    }

    // Format hex output
    final buffer = StringBuffer();
    buffer.writeln("const unsigned char my_sprite[] PROGMEM = {");
    buffer.write("  ");
    for (int i = 0; i < bytes.length; i++) {
      final hex = bytes[i].toRadixString(16).padLeft(2, '0').toUpperCase();
      buffer.write("0x$hex");
      if (i < bytes.length - 1) {
        buffer.write(", ");
      }
      if (i % 8 == 7 && i < bytes.length - 1) {
        buffer.writeln();
        buffer.write("  ");
      }
    }
    buffer.writeln("\n};");

    _codeController.text = buffer.toString();
  }

  void _copyToClipboard() {
    Clipboard.setData(ClipboardData(text: _codeController.text));
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            const Icon(Icons.check_circle, color: Colors.green),
            const SizedBox(width: 8),
            Text(
              "Copied GFX bitmap code to clipboard!",
              style: GoogleFonts.outfit(fontWeight: FontWeight.w600),
            ),
          ],
        ),
        backgroundColor: const Color(0xFF1E1B4B),
        duration: const Duration(seconds: 2),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Mode Selectors
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              "PIXEL EDITOR",
              style: TextStyle(
                color: Colors.white.withOpacity(0.6),
                fontSize: 12,
                fontWeight: FontWeight.bold,
                letterSpacing: 1,
              ),
            ),
            Row(
              children: [
                ChoiceChip(
                  label: const Text("DRAW"),
                  selected: _isDrawingMode,
                  onSelected: (val) {
                    setState(() {
                      _isDrawingMode = true;
                    });
                  },
                  selectedColor: const Color(0xFF8B5CF6),
                  backgroundColor: Colors.white.withOpacity(0.05),
                  labelStyle: TextStyle(
                    color: _isDrawingMode ? Colors.white : Colors.white60,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(width: 8),
                ChoiceChip(
                  label: const Text("ERASE"),
                  selected: !_isDrawingMode,
                  onSelected: (val) {
                    setState(() {
                      _isDrawingMode = false;
                    });
                  },
                  selectedColor: Colors.red.shade900,
                  backgroundColor: Colors.white.withOpacity(0.05),
                  labelStyle: TextStyle(
                    color: !_isDrawingMode ? Colors.white : Colors.white60,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ],
        ),
        const SizedBox(height: 12),

        // Grid Box Container
        AspectRatio(
          aspectRatio: 16 / 8,
          child: Container(
            decoration: BoxDecoration(
              color: Colors.black,
              border: Border.all(color: const Color(0xFF1E1B4B), width: 3),
              borderRadius: BorderRadius.circular(8),
            ),
            child: LayoutBuilder(
              builder: (context, constraints) {
                return GestureDetector(
                  onPanDown: (details) => _handlePaint(details.localPosition, constraints),
                  onPanUpdate: (details) => _handlePaint(details.localPosition, constraints),
                  child: CustomPaint(
                    painter: _GridPainter(grid: _grid),
                  ),
                );
              },
            ),
          ),
        ),
        const SizedBox(height: 16),

        // Clear and Invert Action Buttons
        Row(
          children: [
            Expanded(
              child: ElevatedButton.icon(
                onPressed: _clearEditor,
                icon: const Icon(Icons.clear, size: 16),
                label: const Text("CLEAR"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.white.withOpacity(0.05),
                  foregroundColor: Colors.white,
                  side: BorderSide(color: Colors.white.withOpacity(0.1)),
                ),
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: ElevatedButton.icon(
                onPressed: _invertEditor,
                icon: const Icon(Icons.invert_colors, size: 16),
                label: const Text("INVERT"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.white.withOpacity(0.05),
                  foregroundColor: Colors.white,
                  side: BorderSide(color: Colors.white.withOpacity(0.1)),
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: 16),

        // Export Code Section
        Text(
          "C++ Sprites Code Output",
          style: GoogleFonts.outfit(
            color: Colors.white70,
            fontWeight: FontWeight.w600,
            fontSize: 14,
          ),
        ),
        const SizedBox(height: 8),
        Stack(
          children: [
            TextField(
              controller: _codeController,
              readOnly: true,
              maxLines: 4,
              style: GoogleFonts.firaCode(
                color: const Color(0xFF34D399),
                fontSize: 11,
              ),
              decoration: InputDecoration(
                filled: true,
                fillColor: const Color(0xFF05040A),
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(8),
                  borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                ),
                enabledBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(8),
                  borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                ),
                contentPadding: const EdgeInsets.all(12),
              ),
            ),
            Positioned(
              top: 6,
              right: 6,
              child: IconButton(
                onPressed: _copyToClipboard,
                icon: const Icon(Icons.copy_all, size: 18),
                color: Colors.white54,
                hoverColor: Colors.white10,
                tooltip: "Copy Code",
              ),
            ),
          ],
        ),
      ],
    );
  }
}

class _GridPainter extends CustomPainter {
  final List<List<bool>> grid;

  _GridPainter({required this.grid});

  @override
  void paint(Canvas canvas, Size size) {
    final cellWidth = size.width / 16;
    final cellHeight = size.height / 8;

    final paintWhite = Paint()..color = Colors.white;
    final paintBlack = Paint()..color = Colors.black;

    final gridPaint = Paint()
      ..color = const Color(0xFF1E1B4B)
      ..strokeWidth = 1.0;

    // Draw cells
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 16; x++) {
        final rect = Rect.fromLTWH(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
        canvas.drawRect(rect, grid[y][x] ? paintWhite : paintBlack);
      }
    }

    // Draw lines
    for (int x = 0; x <= 16; x++) {
      canvas.drawLine(Offset(x * cellWidth, 0), Offset(x * cellWidth, size.height), gridPaint);
    }
    for (int y = 0; y <= 8; y++) {
      canvas.drawLine(Offset(0, y * cellHeight), Offset(size.width, y * cellHeight), gridPaint);
    }
  }

  @override
  Widget build(BuildContext context) {
    throw UnimplementedError();
  }

  @override
  bool shouldRepaint(covariant _GridPainter oldDelegate) => true;
}
