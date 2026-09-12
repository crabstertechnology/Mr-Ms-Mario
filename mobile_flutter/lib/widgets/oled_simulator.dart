import 'dart:async';
import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import '../services/database_service.dart';
import '../services/bluetooth_service.dart';
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

  // Pomodoro state
  int _pomoRemainingSec = 1500;
  int _pomoTotalSec = 1500;
  bool _pomoRunning = false;
  int _pomoMode = 0; // 0: 25m Focus, 1: 5m Short Break, 2: 15m Long Break

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

    // 1-second general refresh timer for clock and timer updates
    _refreshTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      if (mounted) {
        final mode = widget.activeGifId.toLowerCase();
        if (mode == 'clock' || mode == 'pomodoro') {
          if (_pomoRunning && _pomoRemainingSec > 0) {
            _pomoRemainingSec--;
            if (_pomoRemainingSec == 0) {
              _pomoRunning = false;
            }
          }
          setState(() {});
        }
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
      _textWidth = widget.marqueeText!.length * 8.0;
    });

    final charCount = widget.marqueeText!.length;
    final durationSecs = max(3.0, charCount * 0.15);

    _marqueeController?.duration = Duration(milliseconds: (durationSecs * 1000).toInt());
    _marqueeController?.forward(from: 0.0);
  }

  @override
  void didUpdateWidget(covariant OLEDSimulator oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.activeGifId != oldWidget.activeGifId) {
      _gifResetCounter++;
      _gifController.reset();
    }
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

  // ── 1.69" TFT Status Bar (240x24px, matching firmware drawStatusBar) ──
  Widget _buildStatusBar(DateTime now, int batteryPct, bool isConnected, Color accent, Color textCol, Color bgCol) {
    return Container(
      height: 24,
      padding: const EdgeInsets.symmetric(horizontal: 18),
      decoration: BoxDecoration(
        color: bgCol,
        border: Border(bottom: BorderSide(color: textCol.withOpacity(0.12), width: 1)),
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            "${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}",
            style: GoogleFonts.outfit(color: textCol, fontSize: 11, fontWeight: FontWeight.bold),
          ),
          Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              Icon(
                isConnected ? Icons.bluetooth_connected : Icons.bluetooth,
                size: 13,
                color: isConnected ? accent : textCol.withOpacity(0.4),
              ),
              const SizedBox(width: 6),
              Text(
                "$batteryPct%",
                style: GoogleFonts.outfit(color: textCol, fontSize: 10, fontWeight: FontWeight.w600),
              ),
              const SizedBox(width: 4),
              Container(
                width: 18,
                height: 9,
                decoration: BoxDecoration(
                  border: Border.all(color: textCol, width: 1),
                  borderRadius: BorderRadius.circular(2),
                ),
                padding: const EdgeInsets.all(1),
                alignment: Alignment.centerLeft,
                child: Container(
                  width: (14 * (batteryPct / 100.0)).clamp(2.0, 14.0),
                  decoration: BoxDecoration(
                    color: batteryPct < 20 ? Colors.red : (batteryPct < 55 ? Colors.amber : Colors.green),
                    borderRadius: BorderRadius.circular(1),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  // ── Clock Screen (matching firmware drawClockScreen Styles 0-3) ──
  Widget _buildClockScreen(DatabaseService db, BLEService ble, Color accent, Color textCol, Color bgCol, Color cardBg) {
    final now = DateTime.now();
    final List<String> weekdays = ['MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT', 'SUN'];
    final List<String> months = ['JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC'];
    final weekday = weekdays[now.weekday - 1];
    final month = months[now.month - 1];
    final dateStr = "${now.day.toString().padLeft(2, '0')} $month";
    final dayDateStr = "$weekday, $dateStr";

    Widget clockBody;

    if (db.clockStyle == 1) {
      // Style 1: Minimalist Pixel
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

      clockBody = Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.baseline,
              textBaseline: TextBaseline.alphabetic,
              children: [
                Text(
                  timeNoSec,
                  style: GoogleFonts.pressStart2p(
                    color: textCol,
                    fontSize: 22,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                const SizedBox(width: 6),
                Text(
                  suffix,
                  style: GoogleFonts.pressStart2p(
                    color: accent,
                    fontSize: 9,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 20),
            Text(
              dayDateStr,
              style: GoogleFonts.pressStart2p(
                color: textCol,
                fontSize: 9,
              ),
            ),
          ],
        ),
      );
    } else if (db.clockStyle == 2) {
      // Style 2: Analog Split
      String digitalTime;
      int hour = now.hour;
      if (db.is12HourFormat) {
        hour = now.hour % 12;
        if (hour == 0) hour = 12;
        digitalTime = "${hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
      } else {
        digitalTime = "${hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
      }

      clockBody = Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            CustomPaint(
              size: const Size(100, 100),
              painter: AnalogClockPainter(now, accent),
            ),
            const SizedBox(height: 16),
            Text(
              digitalTime,
              style: GoogleFonts.pressStart2p(
                color: textCol,
                fontSize: 14,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              dayDateStr,
              style: GoogleFonts.pressStart2p(
                color: textCol,
                fontSize: 8,
              ),
            ),
          ],
        ),
      );
    } else if (db.clockStyle == 3 && widget.wallpaperBytes != null) {
      // Style 3: Custom Wallpaper
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

      return Container(
        width: 240,
        height: 280,
        decoration: BoxDecoration(
          image: DecorationImage(
            image: MemoryImage(widget.wallpaperBytes!),
            fit: BoxFit.cover,
          ),
        ),
        child: Column(
          children: [
            _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, Colors.white, Colors.black38),
            const Spacer(),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 8),
              decoration: BoxDecoration(
                color: Colors.black.withOpacity(0.6),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Text(
                timeStr,
                style: GoogleFonts.pressStart2p(color: Colors.white, fontSize: 14, fontWeight: FontWeight.bold),
              ),
            ),
            const SizedBox(height: 8),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
              decoration: BoxDecoration(
                color: accent.withOpacity(0.8),
                borderRadius: BorderRadius.circular(6),
              ),
              child: Text(
                dayDateStr,
                style: GoogleFonts.pressStart2p(color: Colors.white, fontSize: 8),
              ),
            ),
            const SizedBox(height: 20),
          ],
        ),
      );
    } else {
      // Style 0: Cyberpunk Digital Dashboard (Firmware default style 0)
      int dispHour = now.hour;
      String ampm = "";
      if (db.is12HourFormat) {
        dispHour = now.hour % 12;
        if (dispHour == 0) dispHour = 12;
        ampm = now.hour >= 12 ? "PM" : "AM";
      }
      final timeStr = "${dispHour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
      final secStr = now.second.toString().padLeft(2, '0');

      clockBody = Container(
        margin: const EdgeInsets.all(8),
        padding: const EdgeInsets.all(12),
        decoration: BoxDecoration(
          color: cardBg,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: accent, width: 1.5),
        ),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.baseline,
              textBaseline: TextBaseline.alphabetic,
              children: [
                Text(
                  timeStr,
                  style: GoogleFonts.outfit(
                    color: textCol,
                    fontSize: 38,
                    fontWeight: FontWeight.w900,
                    letterSpacing: -1.0,
                  ),
                ),
                const SizedBox(width: 4),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    if (ampm.isNotEmpty)
                      Text(
                        ampm,
                        style: GoogleFonts.outfit(color: accent, fontSize: 11, fontWeight: FontWeight.bold),
                      ),
                    Text(
                      secStr,
                      style: GoogleFonts.outfit(color: textCol.withOpacity(0.5), fontSize: 11, fontWeight: FontWeight.bold),
                    ),
                  ],
                ),
              ],
            ),
            const SizedBox(height: 6),
            Divider(color: textCol.withOpacity(0.12), height: 1),
            const SizedBox(height: 10),
            Text(
              dayDateStr,
              style: GoogleFonts.outfit(color: textCol, fontSize: 12, fontWeight: FontWeight.bold, letterSpacing: 0.5),
            ),
            const SizedBox(height: 12),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
              decoration: BoxDecoration(
                color: accent.withOpacity(0.12),
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent.withOpacity(0.25)),
              ),
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Icon(Icons.directions_walk, size: 12, color: accent),
                  const SizedBox(width: 4),
                  Text(
                    "TOUCH: ${ble.touchCount}",
                    style: GoogleFonts.outfit(color: accent, fontSize: 9.5, fontWeight: FontWeight.bold),
                  ),
                ],
              ),
            ),
          ],
        ),
      );
    }

    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(child: clockBody),
        ],
      ),
    );
  }

  // ── Notification Screen Card (matching firmware drawNotificationPanel) ──
  Widget _buildNotificationsScreen(DatabaseService db, BLEService ble, Color accent, Color textCol, Color bgCol, Color cardBg) {
    final now = DateTime.now();
    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent.withOpacity(0.5), width: 1.5),
              ),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Stack(
                    alignment: Alignment.center,
                    children: [
                      Container(
                        width: 54,
                        height: 54,
                        decoration: BoxDecoration(
                          shape: BoxShape.circle,
                          color: accent.withOpacity(0.12),
                        ),
                      ),
                      Icon(Icons.nights_stay, size: 34, color: accent),
                    ],
                  ),
                  const SizedBox(height: 12),
                  Text(
                    "INBOX CLEAR",
                    style: GoogleFonts.outfit(
                      color: textCol,
                      fontSize: 15,
                      fontWeight: FontWeight.w900,
                      letterSpacing: 1.2,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    "No new notifications",
                    style: GoogleFonts.outfit(
                      color: textCol.withOpacity(0.6),
                      fontSize: 11,
                    ),
                  ),
                  const SizedBox(height: 14),
                  Container(
                    padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                    decoration: BoxDecoration(
                      color: accent.withOpacity(0.15),
                      borderRadius: BorderRadius.circular(12),
                      border: Border.all(color: accent.withOpacity(0.3)),
                    ),
                    child: Text(
                      "ALL CAUGHT UP",
                      style: GoogleFonts.outfit(color: accent, fontSize: 9, fontWeight: FontWeight.bold),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  // ── Calendar Card (matching firmware drawCalendarEvents) ──
  Widget _buildCalendarScreen(DatabaseService db, BLEService ble, Color accent, Color textCol, Color bgCol, Color cardBg) {
    final now = DateTime.now();
    final events = db.events;

    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent.withOpacity(0.5), width: 1.5),
              ),
              child: events.isEmpty
                  ? Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Icon(Icons.calendar_month, size: 40, color: accent.withOpacity(0.8)),
                        const SizedBox(height: 12),
                        Text(
                          "No Events",
                          style: GoogleFonts.outfit(
                            color: textCol,
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          "Sync events via BLE",
                          style: GoogleFonts.outfit(color: textCol.withOpacity(0.6), fontSize: 11),
                        ),
                        const SizedBox(height: 14),
                        Container(
                          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                          decoration: BoxDecoration(
                            color: accent.withOpacity(0.15),
                            borderRadius: BorderRadius.circular(12),
                            border: Border.all(color: accent.withOpacity(0.3)),
                          ),
                          child: Text(
                            "CALENDAR ACTIVE",
                            style: GoogleFonts.outfit(color: accent, fontSize: 9, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ],
                    )
                  : Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Container(
                              padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 3),
                              decoration: BoxDecoration(
                                color: (events.first.type.toLowerCase().contains('bday') || events.first.type.toLowerCase().contains('birthday'))
                                    ? Colors.pinkAccent
                                    : Colors.amber.shade700,
                                borderRadius: BorderRadius.circular(6),
                              ),
                              child: Text(
                                events.first.type.toUpperCase(),
                                style: GoogleFonts.outfit(color: Colors.white, fontSize: 9, fontWeight: FontWeight.w800),
                              ),
                            ),
                            Builder(
                              builder: (context) {
                                final dt = events.first.dateTime;
                                int h = dt.hour;
                                final m = dt.minute.toString().padLeft(2, '0');
                                final timeStr = db.is12HourFormat
                                    ? '${h % 12 == 0 ? 12 : h % 12}:$m ${h >= 12 ? 'PM' : 'AM'}'
                                    : '${h.toString().padLeft(2, '0')}:$m';
                                return Text(
                                  timeStr,
                                  style: GoogleFonts.outfit(color: accent, fontSize: 13, fontWeight: FontWeight.bold),
                                );
                              },
                            ),
                          ],
                        ),
                        const SizedBox(height: 8),
                        Divider(color: textCol.withOpacity(0.12), height: 1),
                        const SizedBox(height: 10),
                        Text(
                          events.first.title,
                          style: GoogleFonts.outfit(
                            color: textCol,
                            fontSize: 15,
                            fontWeight: FontWeight.bold,
                          ),
                          maxLines: 2,
                          overflow: TextOverflow.ellipsis,
                        ),
                        const SizedBox(height: 4),
                        Builder(
                          builder: (context) {
                            const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
                            final dt = events.first.dateTime;
                            final dateStr = '${months[dt.month - 1]} ${dt.day}, ${dt.year}';
                            return Text(
                              dateStr,
                              style: GoogleFonts.outfit(color: textCol.withOpacity(0.6), fontSize: 11),
                              maxLines: 2,
                              overflow: TextOverflow.ellipsis,
                            );
                          },
                        ),
                        const Spacer(),
                        Center(
                          child: Text(
                            "[ 1 / ${events.length} ]",
                            style: GoogleFonts.outfit(color: textCol.withOpacity(0.4), fontSize: 10, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ],
                    ),
            ),
          ),
        ],
      ),
    );
  }

  // ── Arcade Card (matching firmware SCREEN_GAMES "LUNA ARCADE") ──
  Widget _buildArcadeScreen(Color accent, Color textCol, Color bgCol, Color cardBg, BLEService ble) {
    final now = DateTime.now();
    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent, width: 1.5),
              ),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Icon(Icons.sports_esports, size: 36, color: accent),
                  const SizedBox(height: 8),
                  Text(
                    "LUNA ARCADE",
                    style: GoogleFonts.outfit(
                      color: accent,
                      fontSize: 17,
                      fontWeight: FontWeight.w900,
                      letterSpacing: 1.5,
                    ),
                  ),
                  const SizedBox(height: 6),
                  Divider(color: textCol.withOpacity(0.12), height: 1),
                  const SizedBox(height: 14),
                  Container(
                    width: double.infinity,
                    padding: const EdgeInsets.symmetric(vertical: 10),
                    decoration: BoxDecoration(
                      color: accent,
                      borderRadius: BorderRadius.circular(8),
                      boxShadow: [
                        BoxShadow(
                          color: accent.withOpacity(0.4),
                          blurRadius: 8,
                          offset: const Offset(0, 2),
                        ),
                      ],
                    ),
                    child: Center(
                      child: Text(
                        "START GAME",
                        style: GoogleFonts.outfit(
                          color: Colors.white,
                          fontSize: 13,
                          fontWeight: FontWeight.w900,
                          letterSpacing: 1.0,
                        ),
                      ),
                    ),
                  ),
                  const SizedBox(height: 10),
                  Text(
                    "Tap to Launch Arcade",
                    style: GoogleFonts.outfit(
                      color: textCol.withOpacity(0.6),
                      fontSize: 10,
                    ),
                  ),
                  const SizedBox(height: 10),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      _buildMiniBadge("Racer", accent),
                      const SizedBox(width: 4),
                      _buildMiniBadge("Flappy", accent),
                      const SizedBox(width: 4),
                      _buildMiniBadge("Space", accent),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildMiniBadge(String title, Color accent) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
      decoration: BoxDecoration(
        color: accent.withOpacity(0.12),
        borderRadius: BorderRadius.circular(6),
        border: Border.all(color: accent.withOpacity(0.25)),
      ),
      child: Text(
        title,
        style: GoogleFonts.outfit(color: accent, fontSize: 8.5, fontWeight: FontWeight.bold),
      ),
    );
  }

  // ── Settings Card (matching firmware drawSettingsMenuLandscape) ──
  Widget _buildSettingsScreen(DatabaseService db, BLEService ble, Color accent, Color textCol, Color bgCol, Color cardBg) {
    final now = DateTime.now();
    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.all(10),
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent.withOpacity(0.5), width: 1.5),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Row(
                    children: [
                      Icon(Icons.settings, size: 16, color: accent),
                      const SizedBox(width: 6),
                      Text(
                        "SETTINGS",
                        style: GoogleFonts.outfit(color: accent, fontSize: 13, fontWeight: FontWeight.w900, letterSpacing: 1.0),
                      ),
                    ],
                  ),
                  const SizedBox(height: 6),
                  Divider(color: textCol.withOpacity(0.12), height: 1),
                  const SizedBox(height: 6),
                  _buildSettingRow(Icons.bluetooth, "Bluetooth", ble.isConnected ? "ACTIVE" : "STANDBY", Colors.blue, textCol),
                  const SizedBox(height: 6),
                  _buildSettingRow(Icons.speed, "GIF Speed", "169ms", Colors.orange, textCol),
                  const SizedBox(height: 6),
                  _buildSettingRow(Icons.watch_later, "Clock Style", "Style ${db.clockStyle}", accent, textCol),
                  const SizedBox(height: 6),
                  _buildSettingRow(Icons.brightness_6, "Brightness", "HIGH", Colors.teal, textCol),
                  const Spacer(),
                  Center(
                    child: Text(
                      "Luna OS v2.0",
                      style: GoogleFonts.outfit(color: textCol.withOpacity(0.4), fontSize: 9, fontWeight: FontWeight.w600),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSettingRow(IconData icon, String label, String value, Color badgeCol, Color textCol) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 6),
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.04),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Row(
        children: [
          Icon(icon, size: 14, color: textCol.withOpacity(0.7)),
          const SizedBox(width: 8),
          Expanded(
            child: Text(
              label,
              style: GoogleFonts.outfit(color: textCol, fontSize: 11, fontWeight: FontWeight.w600),
            ),
          ),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
            decoration: BoxDecoration(
              color: badgeCol.withOpacity(0.18),
              borderRadius: BorderRadius.circular(6),
            ),
            child: Text(
              value,
              style: GoogleFonts.outfit(color: badgeCol, fontSize: 9, fontWeight: FontWeight.w800),
            ),
          ),
        ],
      ),
    );
  }

  // ── Pomodoro Timer Card (matching firmware drawPomodoroScreen) ──
  Widget _buildPomodoroScreen(Color accent, Color textCol, Color bgCol, Color cardBg, BLEService ble) {
    final now = DateTime.now();
    final mins = (_pomoRemainingSec ~/ 60).toString().padLeft(2, '0');
    final secs = (_pomoRemainingSec % 60).toString().padLeft(2, '0');
    final progress = 1.0 - (_pomoRemainingSec / _pomoTotalSec.toDouble()).clamp(0.0, 1.0);

    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, textCol, bgCol),
          Expanded(
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.all(10),
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: accent.withOpacity(0.5), width: 1.5),
              ),
              child: Column(
                children: [
                  // Mode capsule pill
                  GestureDetector(
                    onTap: () {
                      setState(() {
                        _pomoMode = (_pomoMode + 1) % 3;
                        if (_pomoMode == 0) {
                          _pomoTotalSec = 1500;
                        } else if (_pomoMode == 1) {
                          _pomoTotalSec = 300;
                        } else {
                          _pomoTotalSec = 900;
                        }
                        _pomoRemainingSec = _pomoTotalSec;
                        _pomoRunning = false;
                      });
                    },
                    child: Container(
                      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
                      decoration: BoxDecoration(
                        color: _pomoMode == 1 ? Colors.green : (_pomoMode == 2 ? Colors.purple : accent),
                        borderRadius: BorderRadius.circular(10),
                      ),
                      child: Text(
                        _pomoMode == 1 ? "SHORT BREAK 5M" : (_pomoMode == 2 ? "LONG BREAK 15M" : "FOCUS 25M"),
                        style: GoogleFonts.outfit(color: Colors.white, fontSize: 9.5, fontWeight: FontWeight.w900),
                      ),
                    ),
                  ),
                  const SizedBox(height: 10),

                  // Circular Clock Dial with tick marks and radial progress
                  Expanded(
                    child: Center(
                      child: SizedBox(
                        width: 116,
                        height: 116,
                        child: CustomPaint(
                          painter: PomodoroClockPainter(
                            progress: progress,
                            accentColor: _pomoMode == 1 ? Colors.green : (_pomoMode == 2 ? Colors.purple : accent),
                            borderColor: textCol.withOpacity(0.4),
                          ),
                          child: Center(
                            child: Column(
                              mainAxisSize: MainAxisSize.min,
                              children: [
                                Text(
                                  "$mins:$secs",
                                  style: GoogleFonts.outfit(
                                    color: textCol,
                                    fontSize: 22,
                                    fontWeight: FontWeight.w900,
                                  ),
                                ),
                                Text(
                                  _pomoRunning ? "RUNNING" : "PAUSED",
                                  style: GoogleFonts.outfit(
                                    color: _pomoRunning ? Colors.green : textCol.withOpacity(0.5),
                                    fontSize: 8.5,
                                    fontWeight: FontWeight.bold,
                                  ),
                                ),
                              ],
                            ),
                          ),
                        ),
                      ),
                    ),
                  ),

                  // Play / Pause pill button
                  GestureDetector(
                    onTap: () {
                      setState(() {
                        _pomoRunning = !_pomoRunning;
                      });
                    },
                    child: Container(
                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 6),
                      decoration: BoxDecoration(
                        color: _pomoRunning ? Colors.orange : accent,
                        borderRadius: BorderRadius.circular(12),
                      ),
                      child: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          Icon(_pomoRunning ? Icons.pause : Icons.play_arrow, size: 14, color: Colors.white),
                          const SizedBox(width: 4),
                          Text(
                            _pomoRunning ? "PAUSE" : "START",
                            style: GoogleFonts.outfit(color: Colors.white, fontSize: 10, fontWeight: FontWeight.bold),
                          ),
                        ],
                      ),
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    "🍅 2 Sessions Done",
                    style: GoogleFonts.outfit(color: textCol.withOpacity(0.6), fontSize: 9),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  // ── Card Screen (QR Code Digital Card) ──
  Widget _buildCardScreen(bool isMiss, Color accent, Color textCol, Color bgCol, BLEService ble) {
    final now = DateTime.now();
    return Container(
      width: 240,
      height: 280,
      color: Colors.white,
      child: Column(
        children: [
          _buildStatusBar(now, ble.batteryPercentage, ble.isConnected, accent, Colors.black87, Colors.white),
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(12),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.qr_code_2, size: 130, color: Colors.black),
                  const SizedBox(height: 6),
                  Text(
                    isMiss ? "MS. LUNA" : "MR. LUNA",
                    style: GoogleFonts.outfit(color: Colors.black, fontSize: 15, fontWeight: FontWeight.bold),
                  ),
                  Text(
                    "Digital Business Card",
                    style: GoogleFonts.outfit(color: Colors.black54, fontSize: 10),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  // ── Navigation Map Screen (Canonical 240x280 Design - Full Screen) ──
  Widget _buildMapScreen(BLEService ble, DatabaseService db, Color accent, Color textCol, Color bgCol, Color cardBg) {
    final now = DateTime.now();
    final isNavActive = ble.isNavActive;
    final dirUpper = ble.navDirection.toUpperCase();
    final distStr = (ble.navDistance.isEmpty || ble.navDistance == "--")
        ? (isNavActive ? "---" : "250 m")
        : ble.navDistance;

    IconData dirIcon = Icons.navigation_rounded;
    String dirLabel = ble.navRoad.isNotEmpty ? ble.navRoad : "Go Straight";

    if (dirUpper.contains("LEFT")) {
      dirIcon = Icons.turn_left_rounded;
      if (ble.navRoad.isEmpty) dirLabel = "Turn Left";
    } else if (dirUpper.contains("RIGHT")) {
      dirIcon = Icons.turn_right_rounded;
      if (ble.navRoad.isEmpty) dirLabel = "Turn Right";
    } else if (dirUpper.contains("UTURN") || dirUpper.contains("U-TURN")) {
      dirIcon = Icons.u_turn_left_rounded;
      if (ble.navRoad.isEmpty) dirLabel = "Make U-Turn";
    } else if (dirUpper.contains("ROUNDABOUT") || dirUpper.contains("ROUND")) {
      dirIcon = Icons.roundabout_right_rounded;
      if (ble.navRoad.isEmpty) dirLabel = "Roundabout";
    }

    String summaryStr = "";
    if (ble.navTotalTime.isNotEmpty && ble.navTotalDist.isNotEmpty) {
      summaryStr = "${ble.navTotalTime} · ${ble.navTotalDist}";
    } else if (ble.navTotalTime.isNotEmpty) {
      summaryStr = ble.navTotalTime;
    } else if (ble.navTotalDist.isNotEmpty) {
      summaryStr = ble.navTotalDist;
    } else if (ble.navDescription.isNotEmpty) {
      summaryStr = ble.navDescription;
    } else {
      summaryStr = "GPS SYNC";
    }

    final etaStr = ble.navEta.isNotEmpty ? "ETA ${ble.navEta}" : "ON ROUTE";

    // Format watch clock
    String clockStr;
    if (db.is12HourFormat) {
      int h12 = now.hour % 12;
      if (h12 == 0) h12 = 12;
      clockStr = "$h12:${now.minute.toString().padLeft(2, '0')} ${now.hour >= 12 ? 'PM' : 'AM'}";
    } else {
      clockStr = "${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}";
    }

    return Container(
      width: 240,
      height: 280,
      color: bgCol,
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 6),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // ── 1. Upper Maneuver & Distance Card ──
          Expanded(
            flex: 13,
            child: Container(
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: textCol.withOpacity(0.08), width: 1.2),
              ),
              padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 6),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  Container(
                    padding: const EdgeInsets.all(8),
                    decoration: BoxDecoration(
                      color: accent.withOpacity(0.12),
                      shape: BoxShape.circle,
                    ),
                    child: Icon(dirIcon, color: accent, size: 36),
                  ),
                  Text(
                    distStr,
                    style: GoogleFonts.outfit(
                      color: textCol,
                      fontSize: 26,
                      fontWeight: FontWeight.w900,
                      letterSpacing: -0.5,
                    ),
                  ),
                  Text(
                    dirLabel,
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: GoogleFonts.outfit(
                      color: accent,
                      fontSize: 14,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: 6),

          // ── 2. Route Telemetry Card ──
          Expanded(
            flex: 10,
            child: Container(
              decoration: BoxDecoration(
                color: cardBg,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: textCol.withOpacity(0.08), width: 1.2),
              ),
              padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 8),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  Text(
                    "REMAINING TRIP",
                    style: GoogleFonts.outfit(
                      color: textCol.withOpacity(0.5),
                      fontSize: 9,
                      fontWeight: FontWeight.bold,
                      letterSpacing: 0.8,
                    ),
                  ),
                  Text(
                    summaryStr,
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: GoogleFonts.outfit(
                      color: textCol,
                      fontSize: 14,
                      fontWeight: FontWeight.w800,
                    ),
                  ),
                  Divider(height: 1, color: textCol.withOpacity(0.08)),
                  Text(
                    etaStr,
                    style: GoogleFonts.outfit(
                      color: accent,
                      fontSize: 14,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
            ),
          ),
          const SizedBox(height: 6),

          // ── 3. Bottom Live & Watch Time Bar ──
          Container(
            height: 28,
            decoration: BoxDecoration(
              color: cardBg,
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: textCol.withOpacity(0.08), width: 1.2),
            ),
            padding: const EdgeInsets.symmetric(horizontal: 10),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Container(
                      width: 6,
                      height: 6,
                      decoration: const BoxDecoration(
                        color: Colors.green,
                        shape: BoxShape.circle,
                      ),
                    ),
                    const SizedBox(width: 6),
                    Text(
                      "MAPS LIVE",
                      style: GoogleFonts.outfit(
                        color: textCol.withOpacity(0.5),
                        fontSize: 9,
                        fontWeight: FontWeight.bold,
                        letterSpacing: 0.6,
                      ),
                    ),
                  ],
                ),
                Text(
                  clockStr,
                  style: GoogleFonts.outfit(
                    color: textCol,
                    fontSize: 10,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);
    final ble = Provider.of<BLEService>(context);

    // Apply brightness, contrast (opacity/blending), invert, and rotation
    final invertVal = widget.invertColor ?? db.oledInvert;
    final rotationVal = db.oledRotation; // in degrees (0, 90, 180, 270)

    final isMiss = db.primaryRobot?.variant == 'ms_luna';
    final oledThemeColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFF0074D9);
    final textCol = const Color(0xFF0F172A);
    final bgCol = Colors.white;
    final cardBg = isMiss ? const Color(0xFFFDF2F8) : const Color(0xFFF8FAFC);

    // Transform degrees to quarter turns for RotatedBox
    int quarterTurns = (rotationVal / 90.0).round() % 4;

    final String mode = widget.activeGifId.toLowerCase();
    final String label = widget.activeLabel.toLowerCase();

    Widget screenContent;

    if ((mode == 'map' || label.contains('map')) && ble.isNavActive) {
      screenContent = _buildMapScreen(ble, db, oledThemeColor, textCol, bgCol, cardBg);
    } else if (mode == 'card' || label.contains('card')) {
      screenContent = _buildCardScreen(isMiss, oledThemeColor, textCol, bgCol, ble);
    } else if (mode == 'clock' || label.contains('clock')) {
      screenContent = _buildClockScreen(db, ble, oledThemeColor, textCol, bgCol, cardBg);
    } else if (mode == 'notif' || mode == 'notifications' || label.contains('notif')) {
      screenContent = _buildNotificationsScreen(db, ble, oledThemeColor, textCol, bgCol, cardBg);
    } else if (mode == 'calendar' || mode == 'cal' || label.contains('calendar')) {
      screenContent = _buildCalendarScreen(db, ble, oledThemeColor, textCol, bgCol, cardBg);
    } else if (mode == 'games' || mode == 'arcade' || label.contains('arcade') || label.contains('game')) {
      screenContent = _buildArcadeScreen(oledThemeColor, textCol, bgCol, cardBg, ble);
    } else if (mode == 'settings' || mode == 'setting' || label.contains('setting')) {
      screenContent = _buildSettingsScreen(db, ble, oledThemeColor, textCol, bgCol, cardBg);
    } else if (mode == 'pomodoro' || mode == 'pomo' || label.contains('pomodoro')) {
      screenContent = _buildPomodoroScreen(oledThemeColor, textCol, bgCol, cardBg, ble);
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
                  final offset = _marqueeAnimation.value * (screenWidth + _textWidth) / 2 + (screenWidth - _textWidth) / 2;
                  return Positioned(
                    left: offset,
                    child: Text(
                      widget.marqueeText!.toUpperCase(),
                      softWrap: false,
                      maxLines: 1,
                      overflow: TextOverflow.visible,
                      style: GoogleFonts.pressStart2p(
                        color: textCol,
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
        // Built-in Sprite AI animation rendering (Exact pixels from sprite_ai_data.h)
        faceWidget = Gif(
          key: ValueKey('${currentGif.id}_$_gifResetCounter'),
          image: AssetImage('assets/animations/${currentGif.id}.gif'),
          controller: _gifController,
          autostart: Autostart.loop,
          fit: BoxFit.contain,
        );
      }

      screenContent = Container(
        color: Colors.black,
        child: SizedBox.expand(
          child: faceWidget,
        ),
      );
    }

    return RotatedBox(
      quarterTurns: quarterTurns,
      child: Container(
        width: 240,
        height: 280, // Exact 240x280 ST7789 screen size
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
          child: invertVal
              ? ColorFiltered(
                  colorFilter: const ColorFilter.matrix([
                    -1,  0,  0, 0, 255,
                     0, -1,  0, 0, 255,
                     0,  0, -1, 0, 255,
                     0,  0,  0, 1,   0,
                  ]),
                  child: screenContent,
                )
              : screenContent,
        ),
      ),
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

class PomodoroClockPainter extends CustomPainter {
  final double progress; // 0.0 to 1.0
  final Color accentColor;
  final Color borderColor;

  PomodoroClockPainter({
    required this.progress,
    required this.accentColor,
    required this.borderColor,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width / 2;

    // Dial background
    final bgPaint = Paint()
      ..color = borderColor.withOpacity(0.08)
      ..style = PaintingStyle.fill;
    canvas.drawCircle(center, radius - 4, bgPaint);

    // Outer ring
    final ringPaint = Paint()
      ..color = borderColor.withOpacity(0.3)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.5;
    canvas.drawCircle(center, radius - 4, ringPaint);

    // 12 hour tick marks
    final tickPaint = Paint()
      ..color = borderColor
      ..strokeWidth = 1.5;
    for (int i = 0; i < 12; i++) {
      final angle = i * (2.0 * pi / 12.0) - (pi / 2.0);
      final p1 = Offset(center.dx + cos(angle) * (radius - 10), center.dy + sin(angle) * (radius - 10));
      final p2 = Offset(center.dx + cos(angle) * (radius - 5), center.dy + sin(angle) * (radius - 5));
      canvas.drawLine(p1, p2, tickPaint);
    }

    // Radial Progress Arc
    final arcPaint = Paint()
      ..color = accentColor
      ..style = PaintingStyle.stroke
      ..strokeWidth = 4.0
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(
      Rect.fromCircle(center: center, radius: radius - 4),
      -pi / 2,
      2 * pi * progress,
      false,
      arcPaint,
    );
  }

  @override
  bool shouldRepaint(covariant PomodoroClockPainter oldDelegate) =>
      oldDelegate.progress != progress || oldDelegate.accentColor != accentColor;
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
    } else if (animId == 'sprite_ai_3') {
      final rectLeft = RRect.fromRectAndRadius(
        Rect.fromCenter(center: centerLeft, width: eyeWidth * 1.2, height: eyeHeight * 0.4),
        const Radius.circular(8),
      );
      final rectRight = RRect.fromRectAndRadius(
        Rect.fromCenter(center: centerRight, width: eyeWidth * 1.2, height: eyeHeight * 0.4),
        const Radius.circular(8),
      );

      canvas.drawRRect(rectLeft, glowPaint);
      canvas.drawRRect(rectRight, glowPaint);
      canvas.drawRRect(rectLeft, paint);
      canvas.drawRRect(rectRight, paint);
    } else if (animId == 'sprite_ai_4') {
      final circleLeft = Rect.fromCenter(center: centerLeft, width: eyeWidth * 1.1, height: eyeWidth * 1.1);
      final circleRight = Rect.fromCenter(center: centerRight, width: eyeWidth * 1.1, height: eyeWidth * 1.1);

      canvas.drawOval(circleLeft, glowPaint);
      canvas.drawOval(circleRight, glowPaint);
      canvas.drawOval(circleLeft, paint);
      canvas.drawOval(circleRight, paint);

      final pupilPaint = Paint()..color = Colors.black;
      canvas.drawCircle(Offset(centerLeft.dx + 4, centerLeft.dy - 4), eyeWidth * 0.25, pupilPaint);
      canvas.drawCircle(Offset(centerRight.dx + 4, centerRight.dy - 4), eyeWidth * 0.25, pupilPaint);
    } else if (animId == 'sprite_ai_5') {
      final rectLeft = RRect.fromRectAndRadius(
        Rect.fromCenter(center: centerLeft, width: eyeWidth, height: eyeHeight),
        const Radius.circular(20),
      );
      canvas.drawRRect(rectLeft, glowPaint);
      canvas.drawRRect(rectLeft, paint);

      final winkPaint = Paint()
        ..color = eyeColor
        ..style = PaintingStyle.stroke
        ..strokeWidth = 10
        ..strokeCap = StrokeCap.round;
      canvas.drawLine(
        Offset(centerRight.dx - eyeWidth * 0.5, centerRight.dy),
        Offset(centerRight.dx + eyeWidth * 0.5, centerRight.dy),
        winkPaint,
      );
    } else {
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
