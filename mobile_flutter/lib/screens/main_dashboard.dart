import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../services/bluetooth_service.dart';
import '../services/database_service.dart';
import '../services/audio_synth_service.dart';
import '../models/gif_model.dart';
import '../models/robot_profile.dart';
import '../models/calendar_event.dart';
import '../widgets/glass_card.dart';
import '../widgets/oled_simulator.dart';
import '../widgets/pixel_editor.dart';

class MainDashboard extends StatefulWidget {
  const MainDashboard({Key? key}) : super(key: key);

  @override
  State<MainDashboard> createState() => _MainDashboardState();
}

class _MainDashboardState extends State<MainDashboard> {
  int _activeTabIdx = 0;
  String _currentSettingsSection = 'categories';
  final AudioSynthService _audioSynth = AudioSynthService();

  late bool _isMissMario;
  late Color _accentColor;
  late Color _accentColorLight;
  
  // Dynamic light theme text colors
  static const Color textColor = Color(0xFF0F172A);
  static const Color textColor70 = Color(0xFF334155);
  static const Color textColor60 = Color(0xFF475569);
  static const Color textColor54 = Color(0xFF64748B);
  static const Color textColor38 = Color(0xFF94A3B8);
  static const Color textColor30 = Color(0xFF94A3B8);
  static const Color textColor24 = Color(0xFFCBD5E1);
  static const Color textColor12 = Color(0xFFE2E8F0);
  static const Color textColor10 = Color(0xFFF1F5F9);

  // Local state for UI inputs
  final TextEditingController _marqueeController = TextEditingController();
  final TextEditingController _customMelodyController = TextEditingController();
  final TextEditingController _aiPromptController = TextEditingController();
  final TextEditingController _searchController = TextEditingController();
  final TextEditingController _calendarTitleController = TextEditingController();
  
  String _selectedCategory = 'ALL';
  String _selectedSort = 'name';
  String _selectedEventType = 'meeting';
  DateTime _selectedEventDateTime = DateTime.now();
  bool _isSettingsSaving = false;
  bool _isCompiling = false;
  String _localActiveGifId = 'relaxed';
  String _localActiveLabel = 'Idle';

  Future<void> _compileFirmware() async {
    setState(() {
      _isCompiling = true;
    });
    try {
      final response = await http.post(
        Uri.parse('http://localhost:8000/api/compile'),
        headers: {'Content-Type': 'application/json'},
      ).timeout(const Duration(seconds: 120));

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        if (data['success'] == true) {
          ScaffoldMessenger.of(context).showSnackBar(
            const SnackBar(
              content: Text("Firmware compiled successfully! Binary exported to mobile_app/bin/"),
              backgroundColor: Colors.green,
            ),
          );
        } else {
          final errorMsg = data['stderr'] ?? data['error'] ?? "Unknown compilation error";
          _showErrorDialog("Compilation Failed", errorMsg);
        }
      } else {
        _showErrorDialog("Server Error", "Server returned HTTP status code ${response.statusCode}");
      }
    } catch (e) {
      _showErrorDialog(
        "Connection Failed",
        "Could not connect to local compilation server at http://localhost:8000.\n\n"
        "Make sure 'python server.py' is running on your machine.\n\n"
        "Error details: $e"
      );
    } finally {
      setState(() {
        _isCompiling = false;
      });
    }
  }

  void _showErrorDialog(String title, String message) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: Colors.white,
        title: Text(title, style: GoogleFonts.outfit(color: Colors.redAccent, fontWeight: FontWeight.bold)),
        content: SingleChildScrollView(
          child: Text(message, style: GoogleFonts.firaCode(color: textColor70, fontSize: 12)),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: Text("OK", style: GoogleFonts.outfit(color: _accentColor)),
          ),
        ],
      ),
    );
  }

  // File upload fields
  String? _uploadedFileName;
  Timer? _cloudPollTimer;
  Timer? _calendarSchedulerTimer;
  final Set<String> _notifiedEventIds = {};
  String? _uploadedFileBase64;
  int _uploadedFrameCount = 8;
  double _uploadCompression = 30.0;

  @override
  void initState() {
    super.initState();
    // Default melody notation (Mario Power-up)
    _customMelodyController.text =
        "E5 50 10\nE5 50 10\nE5 50 30\nC5 50 10\nE5 50 30\nG5 50 50\nG4 50 50\nC5 50 10\nG4 50 30\nE4 50 10\nA4 50 10\nB4 50 10\nAS4 50 10\nA4 50 30\nG4 50 20\nE5 50 10\nG5 50 10\nA5 50 10\nF5 50 10\nG5 50 10\nE5 50 10\nC5 50 10\nD5 50 10\nB4 50 50";
    
    // Poll the cloud connectivity status of all paired robots
    _cloudPollTimer = Timer.periodic(const Duration(seconds: 4), (timer) {
      if (mounted) {
        _pollCloudStatus();
      }
    });

    // Check calendar scheduled meetings/birthdays every 5 seconds
    _calendarSchedulerTimer = Timer.periodic(const Duration(seconds: 5), (timer) {
      if (mounted) {
        _checkCalendarScheduledEvents();
      }
    });
  }

  @override
  void dispose() {
    _cloudPollTimer?.cancel();
    _calendarSchedulerTimer?.cancel();
    _marqueeController.dispose();
    _customMelodyController.dispose();
    _aiPromptController.dispose();
    _searchController.dispose();
    _audioSynth.stop();
    super.dispose();
  }

  Future<void> _pollCloudStatus() async {
    try {
      final db = Provider.of<DatabaseService>(context, listen: false);
      final response = await http.get(Uri.parse('http://localhost:8000/api/robots'))
          .timeout(const Duration(seconds: 2));
      if (response.statusCode == 200) {
        final List<dynamic> list = jsonDecode(response.body);
        
        // Track which MACs are online from the response
        final Set<String> onlineMacs = {};
        for (var robotData in list) {
          if (robotData['status'] == 'online' && robotData['mac'] != null) {
            onlineMacs.add((robotData['mac'] as String).toUpperCase());
          }
        }
        
        // Update each robot's cloudStatus in db
        for (var robot in db.robots) {
          final robotMacClean = robot.remoteId.replaceAll(':', '').toUpperCase();
          final isOnline = onlineMacs.contains(robotMacClean) || onlineMacs.contains(robot.id.toUpperCase());
          final targetStatus = isOnline ? 'online' : 'offline';
          if (robot.cloudStatus != targetStatus) {
            await db.updateRobotCloudStatus(robot.id, targetStatus);
          }
        }
      }
    } catch (e) {
      // Quietly ignore connection errors if local server is down
    }
  }

  void _checkCalendarScheduledEvents() {
    final db = Provider.of<DatabaseService>(context, listen: false);
    final ble = Provider.of<BLEService>(context, listen: false);
    final now = DateTime.now();

    for (final event in db.events) {
      if (!_notifiedEventIds.contains(event.id)) {
        final diff = now.difference(event.dateTime);
        // Trigger if scheduled time is in the past, but not more than 5 minutes old
        if (diff.inSeconds >= 0 && diff.inMinutes < 5) {
          _notifiedEventIds.add(event.id);
          if (ble.isConnected) {
            final hh = event.dateTime.hour.toString().padLeft(2, '0');
            final mm = event.dateTime.minute.toString().padLeft(2, '0');
            final timeStr = "$hh:$mm";
            ble.transmitCalendarEvent(event.type, timeStr, event.title);
            
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(
                content: Text("Event '${event.title}' automatically pushed to robot!"),
                backgroundColor: _accentColor,
              ),
            );
          }
        }
      }
    }
  }

  // Pick GIF from local phone storage
  Future<void> _pickCustomGif() async {
    final result = await FilePicker.platform.pickFiles(
      type: FileType.custom,
      allowedExtensions: ['gif'],
    );
    if (result != null && result.files.single.path != null) {
      final file = result.files.single;
      final bytes = await file.xFile.readAsBytes();
      final base64String = 'data:image/gif;base64,${base64Encode(bytes)}';
      setState(() {
        _uploadedFileName = file.name;
        _uploadedFileBase64 = base64String;
        _uploadedFrameCount = 6 + (file.size % 8); // mock dynamic frame count
      });
    }
  }

  void _saveCustomGifToLibrary(DatabaseService db) {
    if (_uploadedFileBase64 == null || _uploadedFileName == null) return;
    
    final id = _uploadedFileName!
        .toLowerCase()
        .replaceAll(RegExp(r'\.[^/.]+$'), '')
        .replaceAll(RegExp(r'\s+'), '_');

    final sizeBytes = (_uploadedFileBase64!.length * 0.75).toInt();
    final flashSizeKb = '${((sizeBytes * (1 - (_uploadCompression / 150))) / 1024.0).toStringAsFixed(1)}KB';

    final newGif = GifModel(
      id: id,
      name: _uploadedFileName!.replaceAll(RegExp(r'\.[^/.]+$'), ''),
      category: 'Custom',
      favorite: false,
      selected: true,
      hidden: false,
      size: sizeBytes,
      flashSize: flashSizeKb,
      customData: _uploadedFileBase64,
    );

    db.addCustomGif(newGif);
    setState(() {
      _uploadedFileName = null;
      _uploadedFileBase64 = null;
    });

    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text("Custom GIF '${newGif.name}' saved to Database!"),
        backgroundColor: const Color(0xFF8B5CF6),
      ),
    );
  }

  // Sync settings helper
  Future<void> _syncSettingsToRobot(DatabaseService db, BLEService ble) async {
    setState(() {
      _isSettingsSaving = true;
    });

    final defaultGifVal = db.defaultGif;
    final introGifVal = db.introGif;
    final touchSingleVal = db.touchSingle;
    final touchDoubleVal = db.touchDouble;
    final touchLongVal = db.touchLong;

    // Helper map matches JS index calculation
    int getExpressionValue(String val) {
      if (val == 'default') return 0;
      final idx = DatabaseService.animMapping.keys.toList().indexOf(val);
      return idx != -1 ? 100 + idx : 0;
    }

    int getTouchActionValue(String val) {
      if (val == 'default') return 0;
      if (val == 'clock') return 1;
      if (val == 'skip_anim') return 2;
      if (val == 'bt_toggle') return 3;
      final idx = DatabaseService.animMapping.keys.toList().indexOf(val);
      return idx != -1 ? 20 + idx : 0;
    }

    await ble.transmitSaveSettings(
      bleEnabled: db.bleName.isNotEmpty,
      speedMs: db.gifSpeed.toInt(),
      defaultGif: getExpressionValue(defaultGifVal),
      introGif: getExpressionValue(introGifVal),
      touchSingle: getTouchActionValue(touchSingleVal),
      touchDouble: getTouchActionValue(touchDoubleVal),
      touchLong: getTouchActionValue(touchLongVal),
      negativeEnabled: db.negativeEnabled,
      introSpeedMs: db.gifIntroSpeed.toInt(),
      introSoundSpeed: db.introSoundSpeed.toInt(),
    );

    setState(() {
      _isSettingsSaving = false;
    });
  }

  // Show bluetooth scanner modal bottom sheet
  void _showBleScanner(DatabaseService db, BLEService ble) {
    ble.startScan();
    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.white,
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.vertical(top: Radius.circular(20)),
      ),
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return Consumer<BLEService>(
              builder: (context, bleState, _) {
                return Padding(
                  padding: const EdgeInsets.all(20),
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          Text(
                            "SCANNING DEVICES",
                            style: GoogleFonts.outfit(
                              color: textColor,
                              fontSize: 16,
                              fontWeight: FontWeight.bold,
                              letterSpacing: 0.5,
                            ),
                          ),
                          if (bleState.isScanning)
                            SizedBox(
                              width: 16,
                              height: 16,
                              child: CircularProgressIndicator(
                                strokeWidth: 2,
                                valueColor: AlwaysStoppedAnimation<Color>(_accentColor),
                              ),
                            )
                          else
                            TextButton.icon(
                              onPressed: () {
                                bleState.startScan();
                                setModalState(() {});
                              },
                              icon: Icon(Icons.refresh, size: 16, color: _accentColor),
                              label: Text(
                                "Rescan",
                                style: GoogleFonts.outfit(color: _accentColor),
                              ),
                            ),
                        ],
                      ),
                      const SizedBox(height: 16),
                      if (bleState.scanResults.isEmpty)
                        Container(
                          height: 120,
                          alignment: Alignment.center,
                          child: Text(
                            bleState.isScanning
                                ? "Searching for Mr. Mario companion robot..."
                                : "No devices found.",
                            style: GoogleFonts.outfit(color: textColor60),
                          ),
                        )
                      else
                        ConstrainedBox(
                          constraints: const BoxConstraints(maxHeight: 250),
                          child: ListView.builder(
                            shrinkWrap: true,
                            itemCount: bleState.scanResults.length,
                            itemBuilder: (context, index) {
                              final result = bleState.scanResults[index];
                              return Card(
                                color: const Color(0xFFF1F5F9),
                                shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(8),
                                  side: BorderSide(color: Colors.black.withOpacity(0.06)),
                                ),
                                margin: const EdgeInsets.only(bottom: 8),
                                child: ListTile(
                                  title: Text(
                                    result.device.platformName.isNotEmpty
                                        ? result.device.platformName
                                        : "Unknown Device",
                                    style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold),
                                  ),
                                  subtitle: Text(
                                    result.device.remoteId.str,
                                    style: GoogleFonts.firaCode(color: textColor38, fontSize: 11),
                                  ),
                                  trailing: ElevatedButton(
                                    onPressed: () async {
                                      Navigator.pop(context);
                                      await bleState.connect(result.device);
                                      
                                      final id = result.device.remoteId.str;
                                      final name = result.device.platformName.isNotEmpty
                                          ? result.device.platformName
                                          : "Mr. Mario Robot";
                                      final isMiss = name.toLowerCase().contains("miss");
                                      final newRobot = RobotProfile(
                                        id: id,
                                        name: name,
                                        variant: isMiss ? 'miss_mario' : 'mr_mario',
                                        remoteId: id,
                                        lastConnected: DateTime.now(),
                                      );
                                      await db.addRobot(newRobot);
                                    },
                                    style: ElevatedButton.styleFrom(
                                      backgroundColor: _accentColor,
                                      foregroundColor: Colors.white,
                                    ),
                                    child: const Text("PAIR"),
                                  ),
                                ),
                              );
                            },
                          ),
                        ),
                      const SizedBox(height: 12),
                      OutlinedButton(
                        onPressed: () {
                          bleState.stopScan();
                          Navigator.pop(context);
                        },
                        style: OutlinedButton.styleFrom(
                          foregroundColor: textColor60,
                          side: BorderSide(color: Colors.black.withOpacity(0.1)),
                        ),
                        child: const Text("CANCEL"),
                      ),
                    ],
                  ),
                );
              },
            );
          },
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);
    final ble = Provider.of<BLEService>(context);

    _accentColor = const Color(0xFF0284C7); // Premium blue
    _accentColorLight = const Color(0x1F0284C7); // Light blue

    // Resolve current active expression details from BLE or local simulation
    String activeGifId = _localActiveGifId;
    String activeLabel = _localActiveLabel;

    if (ble.isConnected) {
      final exprId = ble.activeExpressionId;
      switch (exprId) {
        case 0:
          activeGifId = 'relaxed';
          activeLabel = 'Idle';
          break;
        case 1:
          activeGifId = 'happy';
          activeLabel = 'Happy';
          break;
        case 2:
          activeGifId = 'crying';
          activeLabel = 'Sad';
          break;
        case 3:
          activeGifId = 'angry';
          activeLabel = 'Angry';
          break;
        case 4:
          activeGifId = 'surprised';
          activeLabel = 'Surprised';
          break;
        case 5:
          activeGifId = 'sleepy';
          activeLabel = 'Sleeping';
          break;
        case 6:
          activeGifId = 'wink';
          activeLabel = 'Wink';
          break;
        case 7:
          activeGifId = 'clock';
          activeLabel = 'Clock';
          break;
        case 8:
          activeGifId = _localActiveGifId;
          activeLabel = _localActiveLabel;
          break;
        default:
          if (exprId == 9) {
            activeLabel = 'Cycling All GIFs';
          }
          break;
      }
    }

    return Scaffold(
      backgroundColor: const Color(0xFFF8FAFC),
      body: Stack(
        children: [
          // Premium Glowing Radial Gradient Backgrounds
          Positioned(
            top: -100,
            left: -100,
            child: Container(
              width: 300,
              height: 300,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                gradient: RadialGradient(
                  colors: [
                    _accentColor.withOpacity(0.08),
                    Colors.transparent,
                  ],
                ),
              ),
            ),
          ),
          Positioned(
            bottom: -100,
            right: -100,
            child: Container(
              width: 300,
              height: 300,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                gradient: RadialGradient(
                  colors: [
                    _accentColor.withOpacity(0.05),
                    Colors.transparent,
                  ],
                ),
              ),
            ),
          ),

          SafeArea(
            child: Column(
              children: [
                // Top sticky navigation bar
                _buildTopNavigation(ble),

                // Main Scrollable Panel Content
                Expanded(
                  child: SingleChildScrollView(
                    padding: const EdgeInsets.all(20),
                    child: _buildPanelContent(db, ble, activeGifId, activeLabel),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
      bottomNavigationBar: _buildBottomNavigationBar(),
    );
  }

  // Floating capsule Bottom Navigation Bar
  Widget _buildBottomNavigationBar() {
    final List<Map<String, dynamic>> items = [
      {'icon': Icons.home, 'label': 'Home'},
      {'icon': Icons.calendar_month, 'label': 'Calendar'},
      {'icon': Icons.settings, 'label': 'Settings'},
    ];

    return Container(
      margin: const EdgeInsets.only(left: 20, right: 20, bottom: 20),
      height: 64,
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.9),
        border: Border.all(color: Colors.black.withOpacity(0.06)),
        borderRadius: BorderRadius.circular(20),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.04),
            blurRadius: 16,
            offset: const Offset(0, 4),
          ),
        ],
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceAround,
        children: List.generate(items.length, (idx) {
          final isSelected = _activeTabIdx == idx;
          return InkWell(
            onTap: () {
              setState(() {
                _activeTabIdx = idx;
                if (idx == 2) {
                  _currentSettingsSection = 'categories';
                }
              });
            },
            borderRadius: BorderRadius.circular(12),
            child: AnimatedContainer(
              duration: const Duration(milliseconds: 200),
              padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 8),
              decoration: BoxDecoration(
                color: isSelected ? _accentColorLight : Colors.transparent,
                border: isSelected
                    ? Border.all(color: _accentColor.withOpacity(0.3))
                    : null,
                borderRadius: BorderRadius.circular(12),
              ),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Icon(
                    items[idx]['icon'] as IconData,
                    color: isSelected ? _accentColor : textColor54,
                    size: 20,
                  ),
                  const SizedBox(height: 2),
                  Text(
                    items[idx]['label'] as String,
                    style: GoogleFonts.outfit(
                      color: isSelected ? _accentColor : textColor38,
                      fontSize: 10,
                      fontWeight: FontWeight.w600,
                    ),
                  ),
                ],
              ),
            ),
          );
        }),
      ),
    );
  }

  // Top header navbar
  Widget _buildTopNavigation(BLEService ble) {
    _isMissMario = Provider.of<DatabaseService>(context, listen: false).primaryRobot?.variant == 'miss_mario';
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.95),
        border: Border(bottom: BorderSide(color: Colors.black.withOpacity(0.06))),
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Row(
            children: [
              Container(
                width: 32,
                height: 32,
                decoration: BoxDecoration(
                  borderRadius: BorderRadius.circular(8),
                  boxShadow: [
                    BoxShadow(
                      color: _accentColor.withOpacity(0.2),
                      blurRadius: 8,
                    ),
                  ],
                  image: const DecorationImage(
                    image: AssetImage('assets/logo.png'),
                    fit: BoxFit.cover,
                  ),
                ),
              ),
              const SizedBox(width: 10),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    _isMissMario ? "Ms. Mario" : "Mr. Mario",
                    style: GoogleFonts.outfit(
                      color: textColor,
                      fontSize: 16,
                      fontWeight: FontWeight.w800,
                      letterSpacing: 0.5,
                    ),
                  ),
                  if (ble.pairedDeviceId != null) ...[
                    const SizedBox(height: 2),
                    Text(
                      "ID: ${ble.pairedDeviceId}",
                      style: GoogleFonts.outfit(
                        color: textColor54,
                        fontSize: 10,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                  ],
                ],
              ),
            ],
          ),
          
          // Connection Status button with icon and no wording
          GestureDetector(
            onTap: () {
              final db = Provider.of<DatabaseService>(context, listen: false);
              if (ble.isConnected) {
                ble.disconnect();
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(
                    content: Text("Disconnected from robot."),
                    duration: Duration(seconds: 2),
                  ),
                );
              } else {
                _showBleScanner(db, ble);
              }
            },
            child: Container(
              padding: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: ble.isConnected
                    ? Colors.green.shade50
                    : Colors.red.shade50,
                shape: BoxShape.circle,
                border: Border.all(
                  color: ble.isConnected
                      ? const Color(0xFF2ECC40).withOpacity(0.3)
                      : const Color(0xFFFF4136).withOpacity(0.3),
                ),
              ),
              child: Icon(
                ble.isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                color: ble.isConnected ? const Color(0xFF2ECC40) : const Color(0xFFFF4136),
                size: 18,
              ),
            ),
          ),
        ],
      ),
    );
  }

  // Active Tab content router
  Widget _buildPanelContent(DatabaseService db, BLEService ble, String activeGifId, String activeLabel) {
    switch (_activeTabIdx) {
      case 0:
        return _buildHomeDashboardPanel(db, ble);
      case 1:
        return _buildCalendarPanel(db, ble);
      case 2:
        return _buildSettingsPanel(db, ble);
      default:
        return const SizedBox();
    }
  }

  // ================= TAB 0: EXPRESSIONS & LIBRARY =================
  Widget _buildExpressionsPanel(DatabaseService db, BLEService ble, String activeGifId, String activeLabel) {
    // Filter lists
    final search = _searchController.text.toLowerCase();
    List<GifModel> filteredGifs = db.gifs.where((gif) {
      final matchesSearch = gif.name.toLowerCase().contains(search) || gif.id.contains(search);
      final matchesCategory = _selectedCategory == 'ALL' || gif.category == _selectedCategory;
      return matchesSearch && matchesCategory;
    }).toList();

    // Sort list
    if (_selectedSort == 'name') {
      filteredGifs.sort((a, b) => a.name.compareTo(b.name));
    } else if (_selectedSort == 'favorites') {
      filteredGifs.sort((a, b) => (b.favorite ? 1 : 0).compareTo(a.favorite ? 1 : 0));
    }

    return Column(
      children: [
        // Top section: OLED Simulator & Diagnostics
        LayoutBuilder(
          builder: (context, constraints) {
            final isWide = constraints.maxWidth > 600;
            return Flex(
              direction: isWide ? Axis.horizontal : Axis.vertical,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Expanded(
                  flex: isWide ? 1 : 0,
                  child: Center(
                    child: OLEDSimulator(
                      activeGifId: activeGifId,
                      activeLabel: activeLabel,
                      marqueeText: _marqueeController.text,
                    ),
                  ),
                ),
                if (!isWide) const SizedBox(height: 20),
                Expanded(
                  flex: isWide ? 1 : 0,
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      // BLE Connect indicator card
                      _buildBLEConnectBar(ble),
                      const SizedBox(height: 12),
                      // Diagnostics Panel Card
                      _buildDiagnosticsCard(ble),
                    ],
                  ),
                ),
              ],
            );
          },
        ),
        const SizedBox(height: 20),

        // Live Notification Marquee Center
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "NOTIFICATION CENTER",
                style: GoogleFonts.outfit(
                  color: const Color(0xFFD8B4FE),
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                  letterSpacing: 1,
                ),
              ),
              const SizedBox(height: 10),
              Row(
                children: [
                  Expanded(
                    child: TextField(
                      controller: _marqueeController,
                      style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                      decoration: InputDecoration(
                        hintText: "Type marquee banner text...",
                        hintStyle: GoogleFonts.outfit(color: textColor30),
                        filled: true,
                        fillColor: Colors.black38,
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(8),
                          borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                        ),
                        contentPadding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                      ),
                    ),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () async {
                      if (_marqueeController.text.isNotEmpty) {
                        setState(() {}); // refresh OLED Simulator marquee
                        await ble.transmitMarqueeText(_marqueeController.text);
                      }
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color(0xFF8B5CF6),
                      foregroundColor: Colors.white,
                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                    ),
                    child: const Text("SEND"),
                  ),
                ],
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // Terminal Log Box
        _buildTerminalLogsCard(ble),
        const SizedBox(height: 20),

        // Custom GIF conversion uploader
        _buildUploadZoneCard(db),
        const SizedBox(height: 20),

        // GIF Library Header (filters/sorting)
        _buildLibraryControlsHeader(db),
        const SizedBox(height: 12),

        // GIF Library Grid list
        GridView.builder(
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
            maxCrossAxisExtent: 160,
            crossAxisSpacing: 12,
            mainAxisSpacing: 12,
            childAspectRatio: 0.9,
          ),
          itemCount: filteredGifs.length,
          itemBuilder: (context, index) {
            final gif = filteredGifs[index];
            return _buildGifCard(db, ble, gif);
          },
        ),
      ],
    );
  }

  Widget _buildBLEConnectBar(BLEService ble) {
    final db = Provider.of<DatabaseService>(context, listen: false);
    String statusWording = "Connection Required";
    if (ble.isConnected) {
      statusWording = "Connected: ${ble.pairedDeviceId ?? 'Robot'}";
    } else if (ble.pairedDeviceId != null) {
      statusWording = "Paired ID: ${ble.pairedDeviceId}";
    }

    return GlassCard(
      padding: const EdgeInsets.all(12),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Expanded(
            child: Row(
              children: [
                Icon(
                  ble.isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                  color: ble.isConnected ? const Color(0xFF2ECC40) : const Color(0xFFFF4136),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: Text(
                    statusWording,
                    overflow: TextOverflow.ellipsis,
                    style: GoogleFonts.outfit(color: textColor70, fontWeight: FontWeight.bold, fontSize: 13),
                  ),
                ),
              ],
            ),
          ),
          const SizedBox(width: 8),
          ElevatedButton.icon(
            onPressed: () {
              if (ble.isConnected) {
                ble.disconnect();
              } else {
                _showBleScanner(db, ble);
              }
            },
            icon: Icon(ble.isConnected ? Icons.close : Icons.link, size: 14),
            label: Text(ble.isConnected ? "DISCONNECT" : "CONNECT"),
            style: ElevatedButton.styleFrom(
              backgroundColor: ble.isConnected ? Colors.red.shade900 : _accentColor,
              foregroundColor: Colors.white,
              textStyle: GoogleFonts.outfit(fontWeight: FontWeight.bold, fontSize: 11),
              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildDiagnosticsCard(BLEService ble) {
    final batteryPct = ble.isConnected
        ? (((ble.batteryVoltage - 3.3) / 0.9) * 100).clamp(0.0, 100.0).toInt()
        : 0;

    String formatUptime(int seconds) {
      if (seconds <= 0) return "--";
      if (seconds < 60) return "${seconds}s";
      final mins = seconds ~/ 60;
      final secs = seconds % 60;
      return "${mins}m ${secs}s";
    }

    return GlassCard(
      padding: const EdgeInsets.all(14),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            children: [
              const Icon(Icons.analytics, size: 14, color: textColor60),
              const SizedBox(width: 6),
              Text(
                "ROBOT DIAGNOSTICS",
                style: GoogleFonts.outfit(
                  color: textColor60,
                  fontSize: 10,
                  fontWeight: FontWeight.bold,
                  letterSpacing: 1,
                ),
              ),
            ],
          ),
          const SizedBox(height: 12),
          
          // Diagnostics values
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text("Uptime", style: GoogleFonts.outfit(color: textColor38, fontSize: 11)),
                    Text(
                      formatUptime(ble.uptimeSeconds),
                      style: GoogleFonts.outfit(color: textColor, fontSize: 15, fontWeight: FontWeight.bold),
                    ),
                  ],
                ),
              ),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text("Gestures Tap", style: GoogleFonts.outfit(color: textColor38, fontSize: 11)),
                    Text(
                      ble.isConnected ? "${ble.touchCount}" : "--",
                      style: GoogleFonts.outfit(color: textColor, fontSize: 15, fontWeight: FontWeight.bold),
                    ),
                  ],
                ),
              ),
            ],
          ),
          const SizedBox(height: 12),

          // Battery bar indicator
          Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Battery Level", style: GoogleFonts.outfit(color: textColor38, fontSize: 11)),
                  Text(
                    ble.isConnected ? "${ble.batteryVoltage.toStringAsFixed(2)}V ($batteryPct%)" : "--",
                    style: GoogleFonts.outfit(
                      color: ble.isConnected
                          ? (batteryPct < 20 ? Colors.red : (batteryPct < 55 ? Colors.yellow : Colors.green))
                          : Colors.white38,
                      fontSize: 11,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 6),
              ClipRRect(
                borderRadius: BorderRadius.circular(4),
                child: LinearProgressIndicator(
                  value: ble.isConnected ? (batteryPct / 100.0) : 0.0,
                  minHeight: 6,
                  backgroundColor: Colors.white.withOpacity(0.05),
                  valueColor: AlwaysStoppedAnimation<Color>(
                    batteryPct < 20 ? Colors.red : (batteryPct < 55 ? Colors.yellow : Colors.green),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildTerminalLogsCard(BLEService ble) {
    final ScrollController logScrollController = ScrollController();
    
    // Auto scroll logs to bottom
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (logScrollController.hasClients) {
        logScrollController.jumpTo(logScrollController.position.maxScrollExtent);
      }
    });

    return GlassCard(
      padding: const EdgeInsets.all(12),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                "DEVICE CONSOLE LOGS",
                style: GoogleFonts.outfit(
                  color: const Color(0xFF22D3EE),
                  fontWeight: FontWeight.bold,
                  fontSize: 11,
                  letterSpacing: 1,
                ),
              ),
              InkWell(
                onTap: () => ble.clearLogs(),
                child: Text(
                  "CLEAR",
                  style: GoogleFonts.outfit(color: textColor38, fontSize: 10, fontWeight: FontWeight.bold),
                ),
              ),
            ],
          ),
          const SizedBox(height: 8),
          Container(
            height: 120,
            padding: const EdgeInsets.all(8),
            decoration: BoxDecoration(
              color: const Color(0xFF05040A),
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: Colors.white.withOpacity(0.05)),
            ),
            child: ListView.builder(
              controller: logScrollController,
              itemCount: ble.consoleLogs.length,
              itemBuilder: (context, idx) {
                final logLine = ble.consoleLogs[idx];
                Color textColor = const Color(0xFF34D399); // default emerald
                if (logLine.contains('[ERROR]')) textColor = Colors.redAccent;
                if (logLine.contains('[BLE]')) textColor = Colors.lightBlue;
                if (logLine.contains('[SETTINGS]')) textColor = Colors.amber;
                if (logLine.contains('[CLOCK]')) textColor = Colors.purpleAccent;

                return Text(
                  logLine,
                  style: GoogleFonts.firaCode(color: textColor, fontSize: 11),
                );
              },
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildUploadZoneCard(DatabaseService db) {
    return GlassCard(
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Text(
            "COMPACT GIF TO FLASH MEMORY",
            style: GoogleFonts.outfit(
              color: textColor70,
              fontWeight: FontWeight.bold,
              fontSize: 12,
              letterSpacing: 1,
            ),
          ),
          const SizedBox(height: 12),

          // Upload Preview or Box
          if (_uploadedFileBase64 == null)
            GestureDetector(
              onTap: _pickCustomGif,
              child: Container(
                height: 100,
                decoration: BoxDecoration(
                  color: Colors.black26,
                  borderRadius: BorderRadius.circular(8),
                  border: Border.all(color: const Color(0x66A855F7), width: 1.5, style: BorderStyle.values[1]),
                ),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Icon(Icons.cloud_upload, color: Color(0xFFD8B4FE), size: 28),
                    const SizedBox(height: 4),
                    Text(
                      "Click to choose custom GIF file",
                      style: GoogleFonts.outfit(color: textColor60, fontSize: 12),
                    ),
                  ],
                ),
              ),
            )
          else ...[
            // Loaded image details
            Row(
              children: [
                ClipRRect(
                  borderRadius: BorderRadius.circular(8),
                  child: Container(
                    width: 72,
                    height: 72,
                    color: Colors.black54,
                    child: Image.memory(
                      base64Decode(_uploadedFileBase64!.split(',').last),
                      fit: BoxFit.contain,
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        _uploadedFileName ?? "custom.gif",
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                        style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 13),
                      ),
                      const SizedBox(height: 2),
                      Row(
                        children: [
                          Text("Frames: ", style: GoogleFonts.outfit(color: textColor38, fontSize: 11)),
                          Text("$_uploadedFrameCount", style: GoogleFonts.outfit(color: Colors.yellow, fontSize: 11, fontWeight: FontWeight.bold)),
                          const SizedBox(width: 8),
                          Text("Est. Size: ", style: GoogleFonts.outfit(color: textColor38, fontSize: 11)),
                          Text(
                            "${(((_uploadedFrameCount * 1024) * (1 - (_uploadCompression / 150))) / 1024.0).toStringAsFixed(1)} KB",
                            style: GoogleFonts.outfit(color: Colors.green, fontSize: 11, fontWeight: FontWeight.bold),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            // Compression slider
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text("Sprite Compression Ratio", style: GoogleFonts.outfit(color: textColor54, fontSize: 12)),
                    Text("${_uploadCompression.toInt()}%", style: GoogleFonts.firaCode(color: const Color(0xFFFFDC00), fontSize: 12, fontWeight: FontWeight.bold)),
                  ],
                ),
                Slider(
                  value: _uploadCompression,
                  min: 0,
                  max: 100,
                  activeColor: const Color(0xFFA855F7),
                  onChanged: (val) {
                    setState(() {
                      _uploadCompression = val;
                    });
                  },
                ),
              ],
            ),
            Row(
              children: [
                Expanded(
                  child: TextButton(
                    onPressed: () {
                      setState(() {
                        _uploadedFileBase64 = null;
                        _uploadedFileName = null;
                      });
                    },
                    style: TextButton.styleFrom(foregroundColor: Colors.white54),
                    child: const Text("CANCEL"),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: ElevatedButton(
                    onPressed: () => _saveCustomGifToLibrary(db),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color(0xFF8B5CF6),
                      foregroundColor: Colors.white,
                    ),
                    child: const Text("SAVE TO LIBRARY"),
                  ),
                ),
              ],
            ),
          ],
        ],
      ),
    );
  }

  Widget _buildLibraryControlsHeader(DatabaseService db) {
    // Unique categories
    final cats = ['ALL'] + db.gifs.map((g) => g.category).toSet().toList();

    return Column(
      children: [
        // Search bar
        TextField(
          controller: _searchController,
          style: GoogleFonts.outfit(color: textColor, fontSize: 14),
          onChanged: (_) => setState(() {}),
          decoration: InputDecoration(
            hintText: "Search eye expressions...",
            hintStyle: GoogleFonts.outfit(color: textColor30),
            prefixIcon: const Icon(Icons.search, color: textColor38, size: 18),
            filled: true,
            fillColor: const Color(0xFFF1F5F9),
            border: OutlineInputBorder(
              borderRadius: BorderRadius.circular(10),
              borderSide: BorderSide(color: Colors.black.withOpacity(0.06)),
            ),
            enabledBorder: OutlineInputBorder(
              borderRadius: BorderRadius.circular(10),
              borderSide: BorderSide(color: Colors.black.withOpacity(0.06)),
            ),
            contentPadding: const EdgeInsets.symmetric(horizontal: 12),
          ),
        ),
        const SizedBox(height: 10),

        // Dropdowns row
        Row(
          children: [
            Expanded(
              child: Container(
                padding: const EdgeInsets.symmetric(horizontal: 10),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(10),
                  border: Border.all(color: Colors.black.withOpacity(0.06)),
                ),
                child: DropdownButtonHideUnderline(
                  child: DropdownButton<String>(
                    value: _selectedCategory,
                    dropdownColor: Colors.white,
                    style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                    items: cats.map((cat) {
                      return DropdownMenuItem<String>(
                        value: cat,
                        child: Text(cat == 'ALL' ? "All Animations" : cat),
                      );
                    }).toList(),
                    onChanged: (val) {
                      if (val != null) {
                        setState(() {
                          _selectedCategory = val;
                        });
                      }
                    },
                  ),
                ),
              ),
            ),
            const SizedBox(width: 8),
            Expanded(
              child: Container(
                padding: const EdgeInsets.symmetric(horizontal: 10),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(10),
                  border: Border.all(color: Colors.black.withOpacity(0.06)),
                ),
                child: DropdownButtonHideUnderline(
                  child: DropdownButton<String>(
                    value: _selectedSort,
                    dropdownColor: Colors.white,
                    style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                    items: const [
                      DropdownMenuItem(value: 'name', child: Text("Sort by Name")),
                      DropdownMenuItem(value: 'favorites', child: Text("Sort by Favorites")),
                    ],
                    onChanged: (val) {
                      if (val != null) {
                        setState(() {
                          _selectedSort = val;
                        });
                      }
                    },
                  ),
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: 10),

        // Select All / Deselect All
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              "EYE LIBRARY LIST",
              style: TextStyle(
                color: Colors.white.withOpacity(0.6),
                fontSize: 11,
                fontWeight: FontWeight.bold,
                letterSpacing: 1,
              ),
            ),
            Row(
              children: [
                InkWell(
                  onTap: () => db.selectAll(true),
                  child: Text(
                    "SELECT ALL",
                    style: GoogleFonts.outfit(color: const Color(0xFFD8B4FE), fontSize: 11, fontWeight: FontWeight.bold),
                  ),
                ),
                const SizedBox(width: 12),
                InkWell(
                  onTap: () => db.selectAll(false),
                  child: Text(
                    "CLEAR ALL",
                    style: GoogleFonts.outfit(color: textColor38, fontSize: 11, fontWeight: FontWeight.bold),
                  ),
                ),
              ],
            ),
          ],
        ),
      ],
    );
  }

  // Individual Card widget representing GIF
  Widget _buildGifCard(DatabaseService db, BLEService ble, GifModel gif) {
    final isFav = gif.favorite;
    final isSelected = gif.selected;
    final isHidden = gif.hidden;
    
    // Resolve GIF image widget source
    Widget imagePreview;
    if (gif.customData != null && gif.customData!.isNotEmpty) {
      try {
        final rawBytes = base64Decode(gif.customData!.split(',').last);
        imagePreview = Image.memory(rawBytes, fit: BoxFit.contain);
      } catch (e) {
        imagePreview = const Icon(Icons.broken_image, color: Colors.red);
      }
    } else {
      imagePreview = Image.asset('assets/animations/${gif.id}.gif', fit: BoxFit.contain);
    }

    return Container(
      decoration: BoxDecoration(
        color: const Color(0x66161526),
        border: Border.all(color: Colors.white.withOpacity(0.06)),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Stack(
        children: [
          // Select Checkbox Top Left
          Positioned(
            top: 2,
            left: 2,
            child: SizedBox(
              width: 24,
              height: 24,
              child: Checkbox(
                value: isSelected,
                activeColor: const Color(0xFFA855F7),
                onChanged: (val) {
                  db.toggleSelected(gif.id);
                },
              ),
            ),
          ),
          
          // Favorite Star Top Right
          Positioned(
            top: 4,
            right: 4,
            child: InkWell(
              onTap: () => db.toggleFavorite(gif.id),
              child: Icon(
                isFav ? Icons.star : Icons.star_border,
                color: isFav ? const Color(0xFFFFDC00) : Colors.white38,
                size: 16,
              ),
            ),
          ),

          // Main Preview click triggers Play
          Align(
            alignment: Alignment.center,
            child: GestureDetector(
              onTap: () async {
                // Determine expr index and sound index
                final mapping = DatabaseService.animMapping[gif.id] ?? { 'expr': 0, 'sound': 0, 'label': gif.name };
                final exprVal = mapping['expr'] as int;
                final soundVal = mapping['sound'] as int;
                
                setState(() {
                  _localActiveGifId = gif.id;
                  _localActiveLabel = gif.name;
                });
                
                ble.addLog("Executing expression: ${gif.name}", "ANIM");
                await ble.transmitExpression(exprVal, gif.name);
                
                // Play melody trigger after short delay if sound enabled
                if (soundVal > 0) {
                  Future.delayed(const Duration(milliseconds: 150), () {
                    ble.transmitAudio(soundVal);
                  });
                }
              },
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const SizedBox(height: 24),
                  Container(
                    height: 50,
                    width: 100,
                    alignment: Alignment.center,
                    child: ColorFiltered(
                      colorFilter: const ColorFilter.mode(Color(0xFF00F0FF), BlendMode.modulate),
                      child: Opacity(
                        opacity: isHidden ? 0.3 : 1.0,
                        child: imagePreview,
                      ),
                    ),
                  ),
                  const SizedBox(height: 8),
                  Padding(
                    padding: const EdgeInsets.symmetric(horizontal: 4),
                    child: Text(
                      gif.name,
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: GoogleFonts.outfit(
                        color: isHidden ? Colors.white38 : Colors.white,
                        fontSize: 12,
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),

          // Action menu bottom row
          Positioned(
            bottom: 2,
            right: 2,
            child: Row(
              children: [
                // Visibility Toggle
                InkWell(
                  onTap: () => db.toggleHidden(gif.id),
                  child: Padding(
                    padding: const EdgeInsets.all(4),
                    child: Icon(
                      isHidden ? Icons.visibility_off : Icons.visibility,
                      color: textColor38,
                      size: 12,
                    ),
                  ),
                ),
                // Delete button for custom assets
                if (!DatabaseService.animMapping.containsKey(gif.id))
                  InkWell(
                    onTap: () => db.deleteCustomGif(gif.id),
                    child: const Padding(
                      padding: EdgeInsets.all(4),
                      child: Icon(
                        Icons.delete,
                        color: Colors.redAccent,
                        size: 12,
                      ),
                    ),
                  ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  // ================= TAB 1: SOUND BOARD PANEL =================
  Widget _buildSoundBoardPanel(DatabaseService db, BLEService ble) {
    final sfxList = [
      {'id': 1, 'name': 'Coin Collect', 'color': const Color(0xFFFFDC00)},
      {'id': 2, 'name': 'Super Mushroom', 'color': const Color(0xFF2ECC40)},
      {'id': 3, 'name': '1-Up Melody', 'color': const Color(0xFF0074D9)},
      {'id': 4, 'name': 'Stomp SFX', 'color': const Color(0xFFFFA500)},
      {'id': 5, 'name': 'Player Shrink', 'color': const Color(0xFFFF4136)},
      {'id': 6, 'name': 'Surprise Warp', 'color': const Color(0xFF8B5CF6)},
    ];

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Custom Synth Preview area
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "8-BIT MUSIC COMPOSER",
                style: GoogleFonts.outfit(
                  color: const Color(0xFFD8B4FE),
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                  letterSpacing: 1,
                ),
              ),
              const SizedBox(height: 10),
              TextField(
                controller: _customMelodyController,
                maxLines: 4,
                style: GoogleFonts.firaCode(color: const Color(0xFF2ECC40), fontSize: 12),
                decoration: InputDecoration(
                  filled: true,
                  fillColor: Colors.black45,
                  border: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(8),
                    borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                  ),
                ),
              ),
              const SizedBox(height: 12),
              Row(
                children: [
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: () {
                        if (_audioSynth.isPlaying) {
                          _audioSynth.stop();
                          setState(() {});
                        } else {
                          setState(() {});
                          _audioSynth.playMelody(
                            _customMelodyController.text,
                            onCompleted: () => setState(() {}),
                          );
                        }
                      },
                      icon: Icon(_audioSynth.isPlaying ? Icons.stop : Icons.play_arrow, size: 16),
                      label: Text(_audioSynth.isPlaying ? "STOP PREVIEW" : "PLAY PREVIEW"),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: _audioSynth.isPlaying ? Colors.red.shade900 : const Color(0xFF8B5CF6),
                        foregroundColor: Colors.white,
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // AI Music composer prompt
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "AI MUSIC GENERATOR",
                style: GoogleFonts.outfit(
                  color: textColor70,
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                  letterSpacing: 1,
                ),
              ),
              const SizedBox(height: 10),
              Row(
                children: [
                  Expanded(
                    child: TextField(
                      controller: _aiPromptController,
                      style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                      decoration: InputDecoration(
                        hintText: "Enter theme (e.g. victory, coin, sad)...",
                        hintStyle: GoogleFonts.outfit(color: textColor24),
                        filled: true,
                        fillColor: Colors.black26,
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(8)),
                        contentPadding: const EdgeInsets.symmetric(horizontal: 10),
                      ),
                    ),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      if (_aiPromptController.text.isNotEmpty) {
                        final notes = _audioSynth.generateAiMelody(_aiPromptController.text);
                        setState(() {
                          _customMelodyController.text = notes;
                        });
                        ScaffoldMessenger.of(context).showSnackBar(
                          const SnackBar(content: Text("Melody generated and loaded!")),
                        );
                      }
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color(0xFF8B5CF6),
                      foregroundColor: Colors.white,
                    ),
                    child: const Text("COMPOSE"),
                  ),
                ],
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // Soundboard Grid triggers
        Text(
          "SFX SOUND BOARD BOARD",
          style: TextStyle(
            color: Colors.white.withOpacity(0.6),
            fontSize: 11,
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
          ),
        ),
        const SizedBox(height: 10),
        GridView.builder(
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
            crossAxisCount: 2,
            crossAxisSpacing: 12,
            mainAxisSpacing: 12,
            childAspectRatio: 2.2,
          ),
          itemCount: sfxList.length,
          itemBuilder: (context, index) {
            final sfx = sfxList[index];
            final color = sfx['color'] as Color;
            return Card(
              color: const Color(0x66161526),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(10),
                side: BorderSide(color: color.withOpacity(0.2)),
              ),
              child: InkWell(
                onTap: () async {
                  final sfxId = sfx['id'] as int;
                  await ble.transmitAudio(sfxId);
                },
                borderRadius: BorderRadius.circular(10),
                child: Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 12),
                  child: Row(
                    children: [
                      Container(
                        width: 8,
                        height: 8,
                        decoration: BoxDecoration(shape: BoxShape.circle, color: color),
                      ),
                      const SizedBox(width: 10),
                      Expanded(
                        child: Text(
                          sfx['name'] as String,
                          style: GoogleFonts.outfit(
                            color: textColor,
                            fontWeight: FontWeight.bold,
                            fontSize: 12,
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            );
          },
        ),
      ],
    );
  }

  // ================= TAB 2: CHRONOS TIME SYNC =================
  Widget _buildChronosPanel(BLEService ble) {
    final now = DateTime.now();
    final timeStr = "${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}:${now.second.toString().padLeft(2, '0')}";

    return Container(
      height: 300,
      alignment: Alignment.center,
      child: GlassCard(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Icon(Icons.watch_later, color: Color(0xFFC084FC)),
                const SizedBox(width: 8),
                Text(
                  "CHRONOS SOFTWARE RTC",
                  style: GoogleFonts.outfit(
                    color: const Color(0xFFC084FC),
                    fontWeight: FontWeight.bold,
                    fontSize: 14,
                    letterSpacing: 1,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 24),
            Center(
              child: Text(
                timeStr,
                style: GoogleFonts.firaCode(
                  color: textColor,
                  fontSize: 36,
                  fontWeight: FontWeight.bold,
                  shadows: [
                    Shadow(color: Colors.purple.withOpacity(0.5), blurRadius: 10),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 12),
            Center(
              child: Text(
                "Syncs local smartphone time to Mr. Mario's OLED display module clock.",
                textAlign: TextAlign.center,
                style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
              ),
            ),
            const SizedBox(height: 24),
            ElevatedButton.icon(
              onPressed: () => ble.syncClockToHardware(),
              icon: const Icon(Icons.sync),
              label: const Text("SYNC TIME CLOCK"),
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFF8B5CF6),
                foregroundColor: Colors.white,
                padding: const EdgeInsets.symmetric(vertical: 14),
              ),
            ),
          ],
        ),
      ),
    );
  }

  // ================= TAB 3: HARDWARE SETTINGS =================
  Widget _buildHardwarePanel(DatabaseService db, BLEService ble) {
    // Collect active GIFs for gesture list mapping
    final gifOptions = ['default', 'clock', 'skip_anim', 'bt_toggle'] +
        db.gifs.where((g) => !g.hidden).map((g) => g.id).toList();

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // System Settings
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "SYSTEM HARDWARE OPTIONS",
                style: GoogleFonts.outfit(color: const Color(0xFFD8B4FE), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
              ),
              const SizedBox(height: 12),

              // BLE Device Name
              Text("BLE Server Broadcast Name", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
              const SizedBox(height: 6),
              TextField(
                style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                controller: TextEditingController(text: db.bleName),
                onSubmitted: (val) async {
                  await db.updateBleName(val);
                  _syncSettingsToRobot(db, ble);
                },
                decoration: const InputDecoration(
                  filled: true,
                  fillColor: Colors.black26,
                  border: OutlineInputBorder(),
                  contentPadding: EdgeInsets.symmetric(horizontal: 10),
                ),
              ),
              const SizedBox(height: 16),

              // GIF frame delay ms
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Eye Frame Delay Duration", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Text("${db.gifSpeed.toInt()} ms", style: GoogleFonts.firaCode(color: Colors.yellow, fontSize: 12, fontWeight: FontWeight.bold)),
                ],
              ),
              Slider(
                value: db.gifSpeed,
                min: 20,
                max: 300,
                activeColor: const Color(0xFF8B5CF6),
                onChanged: (val) => db.updateGifSpeed(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const SizedBox(height: 12),

              // Intro GIF Speed
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Intro GIF Playback Speed", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Text("${db.gifIntroSpeed.toInt()} ms", style: GoogleFonts.firaCode(color: Colors.yellow, fontSize: 12, fontWeight: FontWeight.bold)),
                ],
              ),
              Slider(
                value: db.gifIntroSpeed,
                min: 20,
                max: 300,
                activeColor: const Color(0xFF8B5CF6),
                onChanged: (val) => db.updateGifIntroSpeed(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const SizedBox(height: 12),

              // Intro Sound Speed
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Intro Sound Speed", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Text("${db.introSoundSpeed.toInt()}%", style: GoogleFonts.firaCode(color: Colors.yellow, fontSize: 12, fontWeight: FontWeight.bold)),
                ],
              ),
              Slider(
                value: db.introSoundSpeed,
                min: 20,
                max: 300,
                activeColor: const Color(0xFF8B5CF6),
                onChanged: (val) => db.updateIntroSoundSpeed(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // OLED Display Adjustments
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "OLED GLASS PANEL CONFIGS",
                style: GoogleFonts.outfit(color: const Color(0xFFD8B4FE), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
              ),
              const SizedBox(height: 16),

              // Rotation dropdown
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Screen Rotation Angle", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Container(
                    padding: const EdgeInsets.symmetric(horizontal: 10),
                    decoration: BoxDecoration(
                      color: Colors.white,
                      borderRadius: BorderRadius.circular(6),
                      border: Border.all(color: Colors.black.withOpacity(0.06)),
                    ),
                    child: DropdownButton<double>(
                      value: db.oledRotation,
                      dropdownColor: Colors.white,
                      style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                      underline: const SizedBox(),
                      items: const [
                        DropdownMenuItem(value: 0.0, child: Text("0° Normal")),
                        DropdownMenuItem(value: 90.0, child: Text("90° Right")),
                        DropdownMenuItem(value: 180.0, child: Text("180° Inverted")),
                        DropdownMenuItem(value: 270.0, child: Text("270° Left")),
                      ],
                      onChanged: (val) async {
                        if (val != null) {
                          await db.updateOledRotation(val);
                          _syncSettingsToRobot(db, ble);
                        }
                      },
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),

              // Contrast slider
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Screen Pixel Contrast", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Text(db.oledContrast.toStringAsFixed(1), style: GoogleFonts.firaCode(color: Colors.yellow, fontSize: 12, fontWeight: FontWeight.bold)),
                ],
              ),
              Slider(
                value: db.oledContrast,
                min: 0.5,
                max: 3.0,
                activeColor: const Color(0xFF8B5CF6),
                onChanged: (val) => db.updateOledContrast(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const SizedBox(height: 12),

              // Invert option
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Invert OLED Display (Negative)", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Switch(
                    value: db.oledInvert,
                    activeColor: const Color(0xFF8B5CF6),
                    onChanged: (val) async {
                      await db.updateOledInvert(val);
                      await db.updateNegativeEnabled(val);
                      _syncSettingsToRobot(db, ble);
                    },
                  ),
                ],
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // Gestures mappings
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "HARDWARE TTP233 GESTURES",
                style: GoogleFonts.outfit(color: const Color(0xFFD8B4FE), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
              ),
              const SizedBox(height: 16),

              // Helper gesture dropdown build
              _buildGestureDropdown("Single Tap Action", db.touchSingle, gifOptions, (val) async {
                await db.updateTouchSingle(val);
                _syncSettingsToRobot(db, ble);
              }),
              const SizedBox(height: 12),
              _buildGestureDropdown("Double Tap Action", db.touchDouble, gifOptions, (val) async {
                await db.updateTouchDouble(val);
                _syncSettingsToRobot(db, ble);
              }),
              const SizedBox(height: 12),
              _buildGestureDropdown("Long Press Action", db.touchLong, gifOptions, (val) async {
                await db.updateTouchLong(val);
                _syncSettingsToRobot(db, ble);
              }),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildGestureDropdown(String label, String value, List<String> options, Function(String) onChanged) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text(label, style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
        Container(
          width: 140,
          padding: const EdgeInsets.symmetric(horizontal: 10),
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.circular(6),
            border: Border.all(color: Colors.black.withOpacity(0.06)),
          ),
          child: DropdownButtonHideUnderline(
            child: DropdownButton<String>(
              value: value,
              dropdownColor: Colors.white,
              style: GoogleFonts.outfit(color: textColor, fontSize: 12),
              isExpanded: true,
              items: options.map((opt) {
                // Formatting display name
                String displayName = opt;
                if (opt == 'default') displayName = "Default reaction";
                if (opt == 'clock') displayName = "Show clock";
                if (opt == 'skip_anim') displayName = "Skip Eye Animation";
                if (opt == 'bt_toggle') displayName = "BLE Broadcast Toggle";

                return DropdownMenuItem<String>(
                  value: opt,
                  child: Text(displayName, overflow: TextOverflow.ellipsis),
                );
              }).toList(),
              onChanged: (val) {
                if (val != null) onChanged(val);
              },
            ),
          ),
        ),
      ],
    );
  }

  // ================= TAB 4: PIXEL ART DRAW PANEL =================
  Widget _buildPixelArtPanel() {
    return const GlassCard(
      child: PixelEditor(),
    );
  }

  // ================= NEW TAB 0: HOME DASHBOARD PANEL =================
  Widget _buildHomeDashboardPanel(DatabaseService db, BLEService ble) {
    final primary = db.primaryRobot ?? RobotProfile(
      id: 'mr_mario',
      name: 'Mr. Mario',
      variant: 'mr_mario',
      remoteId: '',
      lastConnected: DateTime.now(),
    );
    
    final isMiss = primary.variant == 'miss_mario';
    final accentColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFF8B5CF6);
    final personality = isMiss ? "Softer & Calmer Personality" : "Friendly & Energetic Personality";
    
    String activeGifId = _localActiveGifId;
    String activeLabel = _localActiveLabel;
    
    if (ble.isConnected) {
      final exprId = ble.activeExpressionId;
      if (exprId != 9) {
        switch (exprId) {
          case 0: activeGifId = 'relaxed'; activeLabel = 'Idle'; break;
          case 1: activeGifId = 'happy'; activeLabel = 'Happy'; break;
          case 2: activeGifId = 'crying'; activeLabel = 'Sad'; break;
          case 3: activeGifId = 'angry'; activeLabel = 'Angry'; break;
          case 4: activeGifId = 'surprised'; activeLabel = 'Surprised'; break;
          case 5: activeGifId = 'sleepy'; activeLabel = 'Sleeping'; break;
          case 6: activeGifId = 'wink'; activeLabel = 'Wink'; break;
        }
      }
    }
    
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Variant header card
        GlassCard(
          child: Row(
            children: [
              Container(
                width: 48,
                height: 48,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  gradient: LinearGradient(
                    colors: isMiss ? [const Color(0xFFEC4899), const Color(0xFFF472B6)] : [const Color(0xFF8B5CF6), const Color(0xFFA78BFA)],
                  ),
                ),
                child: const Icon(Icons.face, color: textColor, size: 28),
              ),
              const SizedBox(width: 14),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      primary.name,
                      style: GoogleFonts.outfit(color: textColor, fontSize: 18, fontWeight: FontWeight.bold),
                    ),
                    Text(
                      personality,
                      style: GoogleFonts.outfit(color: textColor60, fontSize: 11),
                    ),
                  ],
                ),
              ),
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                decoration: BoxDecoration(
                  color: accentColor.withOpacity(0.15),
                  border: Border.all(color: accentColor.withOpacity(0.3)),
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Text(
                  isMiss ? "MISS MARIO" : "MR. MARIO",
                  style: GoogleFonts.outfit(color: accentColor, fontSize: 9, fontWeight: FontWeight.bold, letterSpacing: 0.5),
                ),
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // OLED Simulator Container
        Center(
          child: OLEDSimulator(
            activeGifId: activeGifId,
            activeLabel: activeLabel,
            marqueeText: _marqueeController.text.isNotEmpty ? _marqueeController.text : null,
            invertColor: isMiss || db.oledInvert, 
          ),
        ),
        const SizedBox(height: 20),

        // Robot Status Details Grid
        _buildRobotStatusGrid(primary, ble),
        const SizedBox(height: 20),

        // Relationship Status Banner
        if (primary.companionDeviceId != null)
          _buildRelationshipBanner(primary, db)
        else
          GestureDetector(
            onTap: () {
              setState(() {
                _activeTabIdx = 2; // Settings tab
                _currentSettingsSection = 'companions';
              });
            },
            child: GlassCard(
              padding: const EdgeInsets.all(12),
              border: Border.all(color: Colors.white.withOpacity(0.05)),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.people, color: textColor38, size: 16),
                  const SizedBox(width: 8),
                  Text(
                    "No paired companion. Pair with another robot!",
                    style: GoogleFonts.outfit(color: textColor38, fontSize: 12, fontWeight: FontWeight.w600),
                  ),
                ],
              ),
            ),
          ),
        const SizedBox(height: 20),

        // Quick Actions panel
        _buildQuickActionsPanel(ble),
      ],
    );
  }

  Widget _buildRobotStatusGrid(RobotProfile robot, BLEService ble) {
    final batteryPct = ble.isConnected
        ? (((ble.batteryVoltage - 3.3) / 0.9) * 100).clamp(0.0, 100.0).toInt()
        : 0;

    return GridView.count(
      crossAxisCount: 2,
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      crossAxisSpacing: 12,
      mainAxisSpacing: 12,
      childAspectRatio: 1.8,
      children: [
        // Battery status card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  const Icon(Icons.battery_std, color: Colors.green, size: 18),
                  Text(
                    ble.isConnected ? "$batteryPct%" : "--",
                    style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 16),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text("Battery Level", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
              if (ble.isConnected) ...[
                const SizedBox(height: 4),
                ClipRRect(
                  borderRadius: BorderRadius.circular(2),
                  child: LinearProgressIndicator(
                    value: batteryPct / 100.0,
                    minHeight: 4,
                    backgroundColor: Colors.white.withOpacity(0.05),
                    valueColor: AlwaysStoppedAnimation<Color>(
                      batteryPct < 20 ? Colors.red : (batteryPct < 55 ? Colors.yellow : Colors.green),
                    ),
                  ),
                ),
              ],
            ],
          ),
        ),
        // WiFi Status card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Icon(
                    robot.wifiSSID.isNotEmpty ? Icons.wifi : Icons.wifi_off,
                    color: robot.wifiSSID.isNotEmpty ? const Color(0xFF10B981) : Colors.white38,
                    size: 18,
                  ),
                  Container(
                    width: 8,
                    height: 8,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      color: robot.wifiSSID.isNotEmpty ? const Color(0xFF10B981) : Colors.transparent,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                robot.wifiSSID.isNotEmpty ? robot.wifiSSID : "Not Configured",
                style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 13, textStyle: const TextStyle(overflow: TextOverflow.ellipsis)),
              ),
              Text("Wi-Fi Connection", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
            ],
          ),
        ),
        // BLE connection card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Icon(
                    ble.isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                    color: ble.isConnected ? const Color(0xFF8B5CF6) : Colors.white38,
                    size: 18,
                  ),
                  Text(
                    ble.isConnected ? "Active" : "Offline",
                    style: GoogleFonts.outfit(
                      color: ble.isConnected ? const Color(0xFF8B5CF6) : Colors.white38,
                      fontWeight: FontWeight.bold,
                      fontSize: 10,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                ble.isConnected ? "Bluetooth" : "Disconnected",
                style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
              ),
              Text("BLE Status", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
            ],
          ),
        ),
        // Cloud Link card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Icon(
                    robot.cloudStatus == 'online' ? Icons.cloud_done : Icons.cloud_off,
                    color: robot.cloudStatus == 'online' ? const Color(0xFFD8B4FE) : Colors.white38,
                    size: 18,
                  ),
                  Container(
                    width: 8,
                    height: 8,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      color: robot.cloudStatus == 'online' ? const Color(0xFFD8B4FE) : Colors.transparent,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                robot.cloudStatus == 'online' ? "Connected" : "Offline",
                style: GoogleFonts.outfit(
                  color: robot.cloudStatus == 'online' ? const Color(0xFFD8B4FE) : Colors.white70,
                  fontWeight: FontWeight.bold,
                  fontSize: 14,
                ),
              ),
              Text("Cloud Link", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
            ],
          ),
        ),
        // Companion status card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Icon(
                    robot.companionDeviceId != null ? Icons.people : Icons.people_outline,
                    color: robot.companionDeviceId != null ? const Color(0xFFEC4899) : Colors.white38,
                    size: 18,
                  ),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                robot.companionDeviceId != null
                    ? (robot.relationshipType == 'couple' ? "Couple ❤️" : "Friends 🤝")
                    : "Single",
                style: GoogleFonts.outfit(
                  color: robot.companionDeviceId != null ? const Color(0xFFEC4899) : Colors.white70,
                  fontWeight: FontWeight.bold,
                  fontSize: 14,
                ),
              ),
              Text("Companion Link", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
            ],
          ),
        ),
        // Uptime card
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Icon(Icons.timer, color: Colors.amber, size: 18),
                ],
              ),
              const SizedBox(height: 8),
              Text(
                ble.isConnected ? _formatUptime(ble.uptimeSeconds) : "--",
                style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
              ),
              Text("Robot Uptime", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
            ],
          ),
        ),
      ],
    );
  }

  String _formatUptime(int seconds) {
    if (seconds <= 0) return "--";
    if (seconds < 60) return "${seconds}s";
    final mins = seconds ~/ 60;
    final secs = seconds % 60;
    return "${mins}m ${secs}s";
  }

  Widget _buildRelationshipBanner(RobotProfile primary, DatabaseService db) {
    final companion = db.robots.firstWhere((r) => r.id == primary.companionDeviceId, orElse: () => primary);
    final isCouple = primary.relationshipType == 'couple';
    final heartEmoji = isCouple ? "❤️" : "🤝";
    final relText = isCouple ? "Couple" : "Friends";
    final relationshipColor = isCouple ? const Color(0xFFEC4899) : const Color(0xFF10B981);

    return GlassCard(
      padding: const EdgeInsets.all(14),
      border: Border.all(color: relationshipColor.withOpacity(0.2)),
      child: Row(
        children: [
          Text(
            heartEmoji,
            style: const TextStyle(fontSize: 24),
          ),
          const SizedBox(width: 14),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  "$relText Mode Active",
                  style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
                ),
                Text(
                  "Paired companion: ${companion.name}",
                  style: GoogleFonts.outfit(color: textColor60, fontSize: 11),
                ),
              ],
            ),
          ),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
            decoration: BoxDecoration(
              color: relationshipColor.withOpacity(0.1),
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: relationshipColor.withOpacity(0.3)),
            ),
            child: Text(
              isCouple ? "PARTNERS" : "BESTIES",
              style: GoogleFonts.outfit(color: relationshipColor, fontSize: 9, fontWeight: FontWeight.bold),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildQuickActionsPanel(BLEService ble) {
    final List<Map<String, dynamic>> actions = [
      {
        'icon': Icons.face,
        'label': 'Expression',
        'color': const Color(0xFF8B5CF6),
        'onTap': () => setState(() {
          _activeTabIdx = 2;
          _currentSettingsSection = 'expressions';
        }),
      },
      {
        'icon': Icons.audiotrack,
        'label': 'Play Sound',
        'color': const Color(0xFF10B981),
        'onTap': () => setState(() {
          _activeTabIdx = 2;
          _currentSettingsSection = 'sounds';
        }),
      },
      {
        'icon': Icons.watch_later,
        'label': 'Show Clock',
        'color': const Color(0xFF8B5CF6),
        'onTap': () => ble.transmitExpression(8, ""), // 8 is EXPR_CLOCK
      },
      {
        'icon': Icons.calendar_month,
        'label': 'Calendar',
        'color': const Color(0xFF0074D9),
        'onTap': () => setState(() => _activeTabIdx = 1),
      },
      {
        'icon': Icons.message,
        'label': 'Send Msg',
        'color': const Color(0xFF3B82F6),
        'onTap': () => _showSendMessageDialog(ble),
      },
      {
        'icon': Icons.wb_sunny,
        'label': 'Wake Up',
        'color': Colors.amber,
        'onTap': () => ble.transmitWake(),
      },
      {
        'icon': Icons.nights_stay,
        'label': 'Sleep',
        'color': const Color(0xFF6B7280),
        'onTap': () => ble.transmitSleep(),
      },
      {
        'icon': Icons.link,
        'label': 'Pair Comp',
        'color': const Color(0xFFEC4899),
        'onTap': () => setState(() {
          _activeTabIdx = 2;
          _currentSettingsSection = 'companions';
        }),
      },
    ];

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          "ROBOT QUICK ACTIONS",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 11,
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
          ),
        ),
        const SizedBox(height: 12),
        GridView.builder(
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
            crossAxisCount: 4,
            crossAxisSpacing: 10,
            mainAxisSpacing: 10,
            childAspectRatio: 0.9,
          ),
          itemCount: actions.length,
          itemBuilder: (context, index) {
            final act = actions[index];
            final color = act['color'] as Color;
            return InkWell(
              onTap: act['onTap'] as VoidCallback,
              borderRadius: BorderRadius.circular(12),
              child: GlassCard(
                padding: const EdgeInsets.all(8),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(act['icon'] as IconData, color: color, size: 22),
                    const SizedBox(height: 6),
                    Text(
                      act['label'] as String,
                      textAlign: TextAlign.center,
                      style: GoogleFonts.outfit(
                        color: textColor,
                        fontSize: 9,
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                  ],
                ),
              ),
            );
          },
        ),
      ],
    );
  }

  void _showSendMessageDialog(BLEService ble) {
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          backgroundColor: Colors.white,
          title: Text("Send Message Banner", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
          content: TextField(
            controller: _marqueeController,
            style: GoogleFonts.outfit(color: textColor),
            decoration: InputDecoration(
              hintText: "Enter scrolling message text...",
              hintStyle: GoogleFonts.outfit(color: textColor38),
              enabledBorder: UnderlineInputBorder(borderSide: BorderSide(color: Colors.black.withOpacity(0.1))),
              focusedBorder: UnderlineInputBorder(borderSide: BorderSide(color: _accentColor)),
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: Text("CANCEL", style: TextStyle(color: textColor60)),
            ),
            ElevatedButton(
              onPressed: () async {
                if (_marqueeController.text.isNotEmpty) {
                  Navigator.pop(context);
                  setState(() {}); // Updates OLED preview
                  await ble.transmitMarqueeText(_marqueeController.text);
                }
              },
              style: ElevatedButton.styleFrom(
                backgroundColor: _accentColor,
                foregroundColor: Colors.white,
              ),
              child: const Text("SEND"),
            ),
          ],
        );
      },
    );
  }

  void _showAiChatDialog(BLEService ble) {
    final TextEditingController chatInputController = TextEditingController();
    showDialog(
      context: context,
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return AlertDialog(
              backgroundColor: Colors.white,
              title: Text("AI Companion Chat", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
              content: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Container(
                    height: 120,
                    width: 300,
                    padding: const EdgeInsets.all(8),
                    decoration: BoxDecoration(
                      color: const Color(0xFFF1F5F9),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(color: Colors.black.withOpacity(0.06)),
                    ),
                    child: SingleChildScrollView(
                      child: Text(
                        "Mr. Mario: Hello! How is your day going? Let's write some code together!",
                        style: GoogleFonts.outfit(color: textColor70, fontSize: 13),
                      ),
                    ),
                  ),
                  const SizedBox(height: 12),
                  TextField(
                    controller: chatInputController,
                    style: GoogleFonts.outfit(color: textColor),
                    decoration: InputDecoration(
                      hintText: "Chat with your AI Companion...",
                      hintStyle: GoogleFonts.outfit(color: textColor24),
                      enabledBorder: UnderlineInputBorder(borderSide: BorderSide(color: Colors.black.withOpacity(0.1))),
                      focusedBorder: UnderlineInputBorder(borderSide: BorderSide(color: _accentColor)),
                    ),
                  ),
                ],
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(context),
                  child: Text("CLOSE", style: TextStyle(color: textColor60)),
                ),
                ElevatedButton(
                  onPressed: () {
                    if (chatInputController.text.isNotEmpty) {
                      ble.transmitMarqueeText("AI CHAT...");
                      ScaffoldMessenger.of(context).showSnackBar(
                        SnackBar(content: const Text("Transmitting AI Prompt..."), backgroundColor: _accentColor),
                      );
                      Navigator.pop(context);
                    }
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: _accentColor,
                    foregroundColor: Colors.white,
                  ),
                  child: const Text("SEND"),
                ),
              ],
            );
          },
        );
      },
    );
  }

  void _showNotificationCenterDialog(BLEService ble) {
    final List<Map<String, dynamic>> notifs = [
      {'label': 'Birthday Reminder 🎂', 'text': 'HAPPY BIRTHDAY!'},
      {'label': 'Meeting Reminder 📅', 'text': 'MEETING IN 5 MINS'},
      {'label': 'Task Reminder ✅', 'text': 'DRINK WATER / STAND UP'},
      {'label': 'Weather Alert ⛈️', 'text': 'HEAVY RAIN EXPECTED'},
    ];

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          backgroundColor: const Color(0xFF0F0E1A),
          title: Text("Notification Center", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            children: notifs.map((n) {
              return ListTile(
                title: Text(n['label'] as String, style: GoogleFonts.outfit(color: textColor, fontSize: 13)),
                trailing: const Icon(Icons.send, color: Color(0xFF8B5CF6), size: 16),
                onTap: () async {
                  Navigator.pop(context);
                  await ble.transmitMarqueeText(n['text'] as String);
                },
              );
            }).toList(),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text("CLOSE"),
            ),
          ],
        );
      },
    );
  }

  // ================= NEW TAB 3: COMPANION MANAGEMENT PANEL =================
  Widget _buildCompanionsPanel(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Connected Robots list
        Text(
          "PAIRED ROBOTS",
          style: GoogleFonts.outfit(color: textColor60, fontSize: 11, fontWeight: FontWeight.bold, letterSpacing: 1),
        ),
        const SizedBox(height: 12),

        if (db.robots.isEmpty)
          Container(
            padding: const EdgeInsets.symmetric(vertical: 24),
            alignment: Alignment.center,
            child: Text("No paired robots. Tap below to pair!", style: GoogleFonts.outfit(color: textColor38)),
          )
        else
          ListView.builder(
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            itemCount: db.robots.length,
            itemBuilder: (context, index) {
              final robot = db.robots[index];
              final isPrimary = robot.isPrimary;
              final isMiss = robot.variant == 'miss_mario';
              final accent = isMiss ? const Color(0xFFEC4899) : const Color(0xFF8B5CF6);

              return Card(
                color: const Color(0x66161526),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                  side: BorderSide(color: isPrimary ? accent : Colors.white.withOpacity(0.05)),
                ),
                margin: const EdgeInsets.only(bottom: 12),
                child: ListTile(
                  leading: Container(
                    width: 36,
                    height: 36,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      border: Border.all(color: accent.withOpacity(0.3)),
                      image: const DecorationImage(
                        image: AssetImage('assets/logo.png'),
                        fit: BoxFit.cover,
                      ),
                    ),
                  ),
                  title: Row(
                    children: [
                      Text(
                        robot.name,
                        style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
                      ),
                      if (isPrimary) ...[
                        const SizedBox(width: 8),
                        Container(
                          padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                          decoration: BoxDecoration(
                            color: const Color(0x3310B981),
                            borderRadius: BorderRadius.circular(10),
                          ),
                          child: Text(
                            "PRIMARY",
                            style: GoogleFonts.outfit(color: const Color(0xFF10B981), fontSize: 8, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ],
                    ],
                  ),
                  subtitle: Text(
                    isMiss ? "Miss Mario variant" : "Mr. Mario variant",
                    style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
                  ),
                  trailing: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      IconButton(
                        icon: const Icon(Icons.edit, color: textColor60, size: 16),
                        onPressed: () => _showEditRobotDialog(db, robot),
                      ),
                      if (!isPrimary)
                        IconButton(
                          icon: const Icon(Icons.star_border, color: textColor60, size: 16),
                          onPressed: () => db.setPrimaryRobot(robot.id),
                        ),
                      IconButton(
                        icon: const Icon(Icons.delete_outline, color: Colors.redAccent, size: 16),
                        onPressed: () => db.removeRobot(robot.id),
                      ),
                    ],
                  ),
                ),
              );
            },
          ),

        const SizedBox(height: 12),
        ElevatedButton.icon(
          onPressed: () {
            _showBleScanner(db, ble);
          },
          icon: const Icon(Icons.add),
          label: const Text("PAIR NEW ROBOT"),
          style: ElevatedButton.styleFrom(
            backgroundColor: const Color(0xFF8B5CF6),
            foregroundColor: Colors.white,
            padding: const EdgeInsets.symmetric(vertical: 14),
          ),
        ),

        const SizedBox(height: 24),
        // Wifi Setup Panel
        Text(
          "NETWORK CONFIGURATION",
          style: GoogleFonts.outfit(color: textColor60, fontSize: 11, fontWeight: FontWeight.bold, letterSpacing: 1),
        ),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "Configure Robot Wi-Fi",
                style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
              ),
              const SizedBox(height: 6),
              Text(
                "Send local Wi-Fi router name and password securely to the companion robot over Bluetooth.",
                style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
              ),
              const SizedBox(height: 16),
              ElevatedButton.icon(
                onPressed: ble.isConnected ? () => _showWifiConfigDialog(db, ble) : null,
                icon: const Icon(Icons.wifi),
                label: const Text("CONFIGURE WI-FI"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF10B981),
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(vertical: 12),
                ),
              ),
            ],
          ),
        ),

        const SizedBox(height: 24),
        // Relationship Setup Panel
        Text(
          "ROBOT RELATIONSHIP SETTINGS",
          style: GoogleFonts.outfit(color: textColor60, fontSize: 11, fontWeight: FontWeight.bold, letterSpacing: 1),
        ),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "Companion Pairing Mode",
                style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14),
              ),
              const SizedBox(height: 6),
              Text(
                "Manage relationship status (Friends 🤝 vs Couple ❤️) between Mr. Mario and Miss Mario companions.",
                style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
              ),
              const SizedBox(height: 16),
              ElevatedButton.icon(
                onPressed: db.robots.length >= 2 ? () => _showRelationshipSetupDialog(db, ble) : null,
                icon: const Icon(Icons.favorite),
                label: const Text("MANAGE RELATIONSHIP"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFFEC4899),
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(vertical: 12),
                ),
              ),
              if (db.robots.length < 2) ...[
                const SizedBox(height: 8),
                Text(
                  "Pair at least 2 robots to configure relationship settings.",
                  textAlign: TextAlign.center,
                  style: GoogleFonts.outfit(color: Colors.redAccent.withOpacity(0.7), fontSize: 10),
                ),
              ],
            ],
          ),
        ),
      ],
    );
  }

  void _showEditRobotDialog(DatabaseService db, RobotProfile robot) {
    final nameController = TextEditingController(text: robot.name);
    String selectedVariant = robot.variant;

    showDialog(
      context: context,
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return AlertDialog(
              backgroundColor: Colors.white,
              title: Text("Edit Robot Profile", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
              content: Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  TextField(
                    controller: nameController,
                    style: GoogleFonts.outfit(color: textColor),
                    decoration: InputDecoration(
                      labelText: "Robot Name",
                      labelStyle: GoogleFonts.outfit(color: textColor60),
                      enabledBorder: UnderlineInputBorder(borderSide: BorderSide(color: Colors.black.withOpacity(0.1))),
                      focusedBorder: UnderlineInputBorder(borderSide: BorderSide(color: _accentColor)),
                    ),
                  ),
                  const SizedBox(height: 16),
                  Text("Robot Character Variant", style: GoogleFonts.outfit(color: textColor54, fontSize: 12)),
                  const SizedBox(height: 6),
                  Row(
                    children: [
                      Expanded(
                        child: ChoiceChip(
                          label: const Text("Mr. Mario"),
                          selected: selectedVariant == 'mr_mario',
                          onSelected: (val) {
                            if (val) setModalState(() => selectedVariant = 'mr_mario');
                          },
                          selectedColor: _accentColor.withOpacity(0.2),
                          checkmarkColor: _accentColor,
                        ),
                      ),
                      const SizedBox(width: 8),
                      Expanded(
                        child: ChoiceChip(
                          label: const Text("Ms. Mario"),
                          selected: selectedVariant == 'miss_mario',
                          onSelected: (val) {
                            if (val) setModalState(() => selectedVariant = 'miss_mario');
                          },
                          selectedColor: _accentColor.withOpacity(0.2),
                          checkmarkColor: _accentColor,
                        ),
                      ),
                    ],
                  ),
                ],
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(context),
                  child: Text("CANCEL", style: TextStyle(color: textColor60)),
                ),
                ElevatedButton(
                  onPressed: () async {
                    if (nameController.text.isNotEmpty) {
                      final updated = robot.copyWith(
                        name: nameController.text,
                        variant: selectedVariant,
                      );
                      await db.addRobot(updated);
                      Navigator.pop(context);
                    }
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: _accentColor,
                    foregroundColor: Colors.white,
                  ),
                  child: const Text("SAVE"),
                ),
              ],
            );
          },
        );
      },
    );
  }

  void _showWifiConfigDialog(DatabaseService db, BLEService ble) {
    final primary = db.primaryRobot;
    if (primary == null) return;

    final ssidController = TextEditingController(text: primary.wifiSSID);
    final passController = TextEditingController();

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          backgroundColor: Colors.white,
          title: Text("Configure Wi-Fi", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              TextField(
                controller: ssidController,
                style: GoogleFonts.outfit(color: textColor),
                decoration: InputDecoration(
                  labelText: "Wi-Fi SSID (Network Name)",
                  labelStyle: GoogleFonts.outfit(color: textColor60),
                  enabledBorder: UnderlineInputBorder(borderSide: BorderSide(color: Colors.black.withOpacity(0.1))),
                  focusedBorder: UnderlineInputBorder(borderSide: BorderSide(color: _accentColor)),
                ),
              ),
              const SizedBox(height: 12),
              TextField(
                controller: passController,
                style: GoogleFonts.outfit(color: textColor),
                obscureText: true,
                decoration: InputDecoration(
                  labelText: "Password",
                  labelStyle: GoogleFonts.outfit(color: textColor60),
                  enabledBorder: UnderlineInputBorder(borderSide: BorderSide(color: Colors.black.withOpacity(0.1))),
                  focusedBorder: UnderlineInputBorder(borderSide: BorderSide(color: _accentColor)),
                ),
              ),
            ],
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: Text("CANCEL", style: TextStyle(color: textColor60)),
            ),
            ElevatedButton(
              onPressed: () async {
                if (ssidController.text.isNotEmpty) {
                  Navigator.pop(context);
                  await db.updateRobotWifi(primary.id, ssidController.text);
                  await ble.transmitWifiConfig(ssidController.text, passController.text);
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(
                      content: Text("Wi-Fi SSID '${ssidController.text}' sent successfully!"),
                      backgroundColor: Colors.green,
                    ),
                  );
                }
              },
              style: ElevatedButton.styleFrom(
                backgroundColor: _accentColor,
                foregroundColor: Colors.white,
              ),
              child: const Text("CONNECT"),
            ),
          ],
        );
      },
    );
  }

  void _showRelationshipSetupDialog(DatabaseService db, BLEService ble) {
    final robots = db.robots;
    if (robots.length < 2) return;

    String r1Id = robots[0].id;
    String r2Id = robots[1].id;
    String selectedRel = 'friends';

    showDialog(
      context: context,
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return AlertDialog(
              backgroundColor: Colors.white,
              title: Text("Manage Relationship", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold)),
              content: Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Text(
                    "Choose relationship type between your robots:",
                    style: GoogleFonts.outfit(color: textColor70, fontSize: 13),
                  ),
                  const SizedBox(height: 16),
                  
                  Card(
                    color: selectedRel == 'friends' ? Colors.green.shade50 : const Color(0xFFF1F5F9),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                      side: BorderSide(color: selectedRel == 'friends' ? const Color(0xFF10B981) : Colors.black.withOpacity(0.06)),
                    ),
                    child: ListTile(
                      title: Text("🤝 Friends Mode", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14)),
                      subtitle: Text("Casual talk, synchronized friendly expressions.", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
                      onTap: () => setModalState(() => selectedRel = 'friends'),
                    ),
                  ),
                  const SizedBox(height: 10),

                  Card(
                    color: selectedRel == 'couple' ? Colors.pink.shade50 : const Color(0xFFF1F5F9),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                      side: BorderSide(color: selectedRel == 'couple' ? const Color(0xFFEC4899) : Colors.black.withOpacity(0.06)),
                    ),
                    child: ListTile(
                      title: Text("❤️ Couple Mode", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14)),
                      subtitle: Text("Romantic expressions, shared hearts, anniversary reminders.", style: GoogleFonts.outfit(color: textColor54, fontSize: 11)),
                      onTap: () => setModalState(() => selectedRel = 'couple'),
                    ),
                  ),
                ],
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(context),
                  child: Text("CANCEL", style: TextStyle(color: textColor60)),
                ),
                ElevatedButton(
                  onPressed: () async {
                    Navigator.pop(context);
                    await db.updateRobotRelationship(r1Id, r2Id, selectedRel);
                    if (ble.isConnected) {
                      await ble.transmitRelationship(selectedRel);
                      await ble.transmitCompanionPairing(robots[1].remoteId, selectedRel);
                    }
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(
                        content: Text("Robots relationship configured as $selectedRel!"),
                        backgroundColor: selectedRel == 'couple' ? const Color(0xFFEC4899) : const Color(0xFF10B981),
                      ),
                    );
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: _accentColor,
                    foregroundColor: Colors.white,
                  ),
                  child: const Text("SAVE"),
                ),
              ],
            );
          },
        );
      },
    );
  }

  // ================= NEW TAB 3: CALENDAR MANAGEMENT PANEL =================
  Widget _buildCalendarPanel(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          "ROBOT CALENDAR & EVENT CENTER",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 11,
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
          ),
        ),
        const SizedBox(height: 16),

        // Add Event Card
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "Add Calendar Event",
                style: GoogleFonts.outfit(
                  color: textColor,
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
              const SizedBox(height: 12),
              
              // Event Type Toggle
              Row(
                children: [
                  Expanded(
                    child: InkWell(
                      onTap: () {
                        setState(() {
                          _selectedEventType = 'meeting';
                        });
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 8),
                        decoration: BoxDecoration(
                          color: _selectedEventType == 'meeting'
                              ? const Color(0x33A855F7)
                              : Colors.white.withOpacity(0.05),
                          border: Border.all(
                            color: _selectedEventType == 'meeting'
                                ? const Color(0x80A855F7)
                                : Colors.white.withOpacity(0.1),
                          ),
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            const Icon(Icons.groups, color: Color(0xFFD8B4FE), size: 18),
                            const SizedBox(width: 6),
                            Text(
                              "Meeting",
                              style: GoogleFonts.outfit(
                                color: textColor,
                                fontWeight: FontWeight.bold,
                                fontSize: 13,
                              ),
                            ),
                          ],
                        ),
                      ),
                    ),
                  ),
                  const SizedBox(width: 10),
                  Expanded(
                    child: InkWell(
                      onTap: () {
                        setState(() {
                          _selectedEventType = 'birthday';
                        });
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 8),
                        decoration: BoxDecoration(
                          color: _selectedEventType == 'birthday'
                              ? const Color(0x33EC4899)
                              : Colors.white.withOpacity(0.05),
                          border: Border.all(
                            color: _selectedEventType == 'birthday'
                                ? const Color(0x80EC4899)
                                : Colors.white.withOpacity(0.1),
                          ),
                          borderRadius: BorderRadius.circular(8),
                        ),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            const Icon(Icons.cake, color: Color(0xFFF9A8D4), size: 18),
                            const SizedBox(width: 6),
                            Text(
                              "Birthday",
                              style: GoogleFonts.outfit(
                                color: textColor,
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
              ),
              const SizedBox(height: 12),

              // Title input
              TextField(
                controller: _calendarTitleController,
                style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                decoration: InputDecoration(
                  labelText: "Event Title",
                  labelStyle: GoogleFonts.outfit(color: textColor38),
                  hintText: _selectedEventType == 'meeting'
                      ? "e.g., Team Sync"
                      : "e.g., Mr. Mario's Birthday",
                  hintStyle: GoogleFonts.outfit(color: textColor24, fontSize: 13),
                  filled: true,
                  fillColor: Colors.white.withOpacity(0.03),
                  border: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(10),
                    borderSide: BorderSide(color: Colors.white.withOpacity(0.1)),
                  ),
                  enabledBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(10),
                    borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                  ),
                  focusedBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(10),
                    borderSide: const BorderSide(color: Color(0xFFA855F7)),
                  ),
                ),
              ),
              const SizedBox(height: 12),

              // Date/Time Selection Row
              Row(
                children: [
                  Expanded(
                    child: InkWell(
                      onTap: () async {
                        final picked = await showDatePicker(
                          context: context,
                          initialDate: _selectedEventDateTime,
                          firstDate: DateTime(2025),
                          lastDate: DateTime(2030),
                          builder: (context, child) {
                            return Theme(
                              data: ThemeData.dark().copyWith(
                                colorScheme: const ColorScheme.dark(
                                  primary: Color(0xFFA855F7),
                                  onPrimary: Colors.white,
                                  surface: Color(0xFF0F0E1A),
                                  onSurface: Colors.white,
                                ),
                                dialogBackgroundColor: const Color(0xFF080710),
                              ),
                              child: child!,
                            );
                          },
                        );
                        if (picked != null) {
                          setState(() {
                            _selectedEventDateTime = DateTime(
                              picked.year,
                              picked.month,
                              picked.day,
                              _selectedEventDateTime.hour,
                              _selectedEventDateTime.minute,
                            );
                          });
                        }
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 12),
                        decoration: BoxDecoration(
                          color: Colors.white.withOpacity(0.05),
                          border: Border.all(color: Colors.white.withOpacity(0.07)),
                          borderRadius: BorderRadius.circular(10),
                        ),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              "${_selectedEventDateTime.day}/${_selectedEventDateTime.month}/${_selectedEventDateTime.year}",
                              style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                            ),
                            const Icon(Icons.calendar_today, color: textColor54, size: 16),
                          ],
                        ),
                      ),
                    ),
                  ),
                  const SizedBox(width: 10),
                  Expanded(
                    child: InkWell(
                      onTap: () async {
                        final picked = await showTimePicker(
                          context: context,
                          initialTime: TimeOfDay.fromDateTime(_selectedEventDateTime),
                          builder: (context, child) {
                            return Theme(
                              data: ThemeData.dark().copyWith(
                                colorScheme: const ColorScheme.dark(
                                  primary: Color(0xFFA855F7),
                                  onPrimary: Colors.white,
                                  surface: Color(0xFF0F0E1A),
                                  onSurface: Colors.white,
                                ),
                                dialogBackgroundColor: const Color(0xFF080710),
                              ),
                              child: child!,
                            );
                          },
                        );
                        if (picked != null) {
                          setState(() {
                            _selectedEventDateTime = DateTime(
                              _selectedEventDateTime.year,
                              _selectedEventDateTime.month,
                              _selectedEventDateTime.day,
                              picked.hour,
                              picked.minute,
                            );
                          });
                        }
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 12),
                        decoration: BoxDecoration(
                          color: Colors.white.withOpacity(0.05),
                          border: Border.all(color: Colors.white.withOpacity(0.07)),
                          borderRadius: BorderRadius.circular(10),
                        ),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              TimeOfDay.fromDateTime(_selectedEventDateTime).format(context),
                              style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                            ),
                            const Icon(Icons.access_time, color: textColor54, size: 16),
                          ],
                        ),
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),

              // Action button to save
              ElevatedButton(
                onPressed: () async {
                  if (_calendarTitleController.text.trim().isEmpty) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(
                        content: Text("Please enter event title"),
                        backgroundColor: Colors.redAccent,
                      ),
                    );
                    return;
                  }

                  final newEvent = CalendarEvent(
                    id: DateTime.now().millisecondsSinceEpoch.toString(),
                    title: _calendarTitleController.text.trim(),
                    dateTime: _selectedEventDateTime,
                    type: _selectedEventType,
                  );

                  await db.addEvent(newEvent);

                  // Send event notification directly if connected
                  if (ble.isConnected) {
                    final hh = newEvent.dateTime.hour.toString().padLeft(2, '0');
                    final mm = newEvent.dateTime.minute.toString().padLeft(2, '0');
                    final timeStr = "$hh:$mm";
                    await ble.transmitCalendarEvent(
                      newEvent.type,
                      timeStr,
                      newEvent.title,
                    );
                  }

                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(
                      content: Text("Event '${newEvent.title}' scheduled!"),
                      backgroundColor: const Color(0xFF10B981),
                    ),
                  );

                  _calendarTitleController.clear();
                  setState(() {
                    _selectedEventDateTime = DateTime.now();
                  });
                },
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF8B5CF6),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(10),
                  ),
                  padding: const EdgeInsets.symmetric(vertical: 12),
                ),
                child: Text(
                  "Schedule & Push to Robot",
                  style: GoogleFonts.outfit(fontWeight: FontWeight.bold, fontSize: 14, color: Colors.white),
                ),
              ),
            ],
          ),
        ),
        const SizedBox(height: 20),

        // Scheduled Events List Header
        Text(
          "SCHEDULED EVENTS (${db.events.length})",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 11,
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
          ),
        ),
        const SizedBox(height: 10),

        if (db.events.isEmpty)
          Container(
            padding: const EdgeInsets.symmetric(vertical: 36),
            alignment: Alignment.center,
            child: Text(
              "No upcoming meetings or birthdays.",
              style: GoogleFonts.outfit(color: textColor38),
            ),
          )
        else
          ListView.builder(
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            itemCount: db.events.length,
            itemBuilder: (context, index) {
              final event = db.events[index];
              final isMeeting = event.type == 'meeting';
              final hh = event.dateTime.hour.toString().padLeft(2, '0');
              final mm = event.dateTime.minute.toString().padLeft(2, '0');
              final timeStr = "$hh:$mm";
              final dateStr = "${event.dateTime.day}/${event.dateTime.month}/${event.dateTime.year}";

              return Container(
                margin: const EdgeInsets.only(bottom: 10),
                padding: const EdgeInsets.all(12),
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.03),
                  border: Border.all(color: Colors.white.withOpacity(0.05)),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: Row(
                  children: [
                    // Icon inside circle
                    Container(
                      width: 36,
                      height: 36,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        gradient: LinearGradient(
                          colors: isMeeting
                              ? [const Color(0xFF8B5CF6), const Color(0xFF6366F1)]
                              : [const Color(0xFFEC4899), const Color(0xFFF43F5E)],
                        ),
                      ),
                      child: Icon(
                        isMeeting ? Icons.groups : Icons.cake,
                        color: textColor,
                        size: 18,
                      ),
                    ),
                    const SizedBox(width: 12),

                    // Event Details
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            event.title,
                            style: GoogleFonts.outfit(
                              color: textColor,
                              fontSize: 14,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 2),
                          Text(
                            isMeeting ? "Meeting @ $timeStr ($dateStr)" : "Birthday ($dateStr)",
                            style: GoogleFonts.outfit(
                              color: textColor54,
                              fontSize: 11,
                            ),
                          ),
                        ],
                      ),
                    ),

                    // Action buttons
                    Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        IconButton(
                          onPressed: () async {
                            if (!ble.isConnected) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                const SnackBar(
                                  content: Text("Bluetooth not connected"),
                                  backgroundColor: Colors.orangeAccent,
                                ),
                              );
                              return;
                            }
                            await ble.transmitCalendarEvent(event.type, timeStr, event.title);
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(
                                content: Text("Pushed '${event.title}' to robot!"),
                                backgroundColor: const Color(0xFF10B981),
                              ),
                            );
                          },
                          icon: const Icon(Icons.send, color: Color(0xFF10B981), size: 18),
                          tooltip: "Push notification to robot",
                        ),
                        IconButton(
                          onPressed: () async {
                            await db.deleteEvent(event.id);
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(
                                content: Text("Event '${event.title}' deleted"),
                                backgroundColor: Colors.redAccent,
                              ),
                            );
                          },
                          icon: const Icon(Icons.delete, color: Color(0xFFEF4444), size: 18),
                          tooltip: "Delete event",
                        ),
                      ],
                    ),
                  ],
                ),
              );
            },
          ),
      ],
    );
  }

  // ================= NEW TAB 4: ADVANCED SETTINGS PANEL =================
  // ================= NEW TAB 4: ADVANCED SETTINGS PANEL =================
  Widget _buildSettingsCategories(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          "Settings",
          style: GoogleFonts.outfit(
            color: textColor,
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 4),
        Text(
          "Select a category below to configure your companion robot.",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 13,
          ),
        ),
        const SizedBox(height: 20),
        _buildCategoryCard(
          icon: Icons.people,
          iconColor: Colors.blue.shade600,
          title: "Companion Profiles",
          subtitle: "Manage and pair blue (Mr. Mario) and pink (Ms. Mario) variants.",
          onTap: () {
            setState(() {
              _currentSettingsSection = 'companions';
            });
          },
        ),
        const SizedBox(height: 12),
        _buildCategoryCard(
          icon: Icons.face,
          iconColor: Colors.teal.shade600,
          title: "Face Expressions",
          subtitle: "Trigger animations, RLE bitmaps, and custom face expressions.",
          onTap: () {
            setState(() {
              _currentSettingsSection = 'expressions';
            });
          },
        ),
        const SizedBox(height: 12),
        _buildCategoryCard(
          icon: Icons.audiotrack,
          iconColor: Colors.pink.shade600,
          title: "Sound & Melody Board",
          subtitle: "Play preloaded melodies or compose custom 8-bit sound effects.",
          onTap: () {
            setState(() {
              _currentSettingsSection = 'sounds';
            });
          },
        ),
        const SizedBox(height: 12),
        _buildCategoryCard(
          icon: Icons.settings,
          iconColor: Colors.purple.shade600,
          title: "Device Configuration",
          subtitle: "Configure clock sync, pixel art editor, orientation, and NVS preferences.",
          onTap: () {
            setState(() {
              _currentSettingsSection = 'device';
            });
          },
        ),
      ],
    );
  }

  Widget _buildCategoryCard({
    required IconData icon,
    required Color iconColor,
    required String title,
    required String subtitle,
    required VoidCallback onTap,
  }) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 18),
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(16),
          border: Border.all(color: Colors.black.withOpacity(0.06)),
          boxShadow: [
            BoxShadow(
              color: Colors.black.withOpacity(0.02),
              blurRadius: 10,
              offset: const Offset(0, 4),
            ),
          ],
        ),
        child: Row(
          children: [
            Container(
              width: 48,
              height: 48,
              decoration: BoxDecoration(
                color: iconColor.withOpacity(0.1),
                shape: BoxShape.circle,
              ),
              child: Icon(icon, color: iconColor, size: 24),
            ),
            const SizedBox(width: 16),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    title,
                    style: GoogleFonts.outfit(
                      color: textColor,
                      fontWeight: FontWeight.bold,
                      fontSize: 15,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    subtitle,
                    style: GoogleFonts.outfit(
                      color: textColor54,
                      fontSize: 12,
                    ),
                  ),
                ],
              ),
            ),
            Icon(
              Icons.chevron_right,
              color: textColor38,
              size: 20,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSettingsSubHeader(String title) {
    return Row(
      children: [
        IconButton(
          onPressed: () {
            setState(() {
              _currentSettingsSection = 'categories';
            });
          },
          icon: const Icon(Icons.arrow_back),
        ),
        const SizedBox(width: 8),
        Text(
          title,
          style: GoogleFonts.outfit(
            color: textColor,
            fontSize: 18,
            fontWeight: FontWeight.bold,
          ),
        ),
      ],
    );
  }

  Widget _buildSettingsCompanions(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSettingsSubHeader("Companion Profiles"),
        const SizedBox(height: 16),
        _buildCompanionsPanel(db, ble),
      ],
    );
  }

  Widget _buildSettingsExpressions(DatabaseService db, BLEService ble, String activeGifId, String activeLabel) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSettingsSubHeader("Face Expressions"),
        const SizedBox(height: 16),
        _buildExpressionsPanel(db, ble, activeGifId, activeLabel),
      ],
    );
  }

  Widget _buildSettingsSounds(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSettingsSubHeader("Sound Board"),
        const SizedBox(height: 16),
        _buildSoundBoardPanel(db, ble),
      ],
    );
  }

  Widget _buildSettingsDevice(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSettingsSubHeader("Device Configuration"),
        const SizedBox(height: 16),
        _buildChronosPanel(ble),
        const SizedBox(height: 20),
        _buildPixelArtPanel(),
        const SizedBox(height: 20),
        _buildHardwarePanel(db, ble),
      ],
    );
  }

  Widget _buildSettingsPanel(DatabaseService db, BLEService ble) {
    final activeGifId = _localActiveGifId;
    final activeLabel = _localActiveLabel;
    
    switch (_currentSettingsSection) {
      case 'companions':
        return _buildSettingsCompanions(db, ble);
      case 'expressions':
        return _buildSettingsExpressions(db, ble, activeGifId, activeLabel);
      case 'sounds':
        return _buildSettingsSounds(db, ble);
      case 'device':
        return _buildSettingsDevice(db, ble);
      case 'categories':
      default:
        return _buildSettingsCategories(db, ble);
    }
  }
}
