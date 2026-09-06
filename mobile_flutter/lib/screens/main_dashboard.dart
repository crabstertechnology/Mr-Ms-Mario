import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:ui' as ui;
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:http/http.dart' as http;
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../services/bluetooth_service.dart';
import '../services/database_service.dart';
import '../services/audio_synth_service.dart';
import '../services/audio_stream_service.dart';
import '../models/gif_model.dart';
import '../models/robot_profile.dart';
import '../models/calendar_event.dart';
import '../models/alarm_model.dart';
import '../services/notification_service.dart';
import '../services/firebase_service.dart';
import '../widgets/glass_card.dart';
import '../widgets/oled_simulator.dart';
import '../widgets/pixel_editor.dart';
import '../widgets/luna_background.dart';
import 'package:gif/gif.dart';
import 'login_screen.dart';
import 'business_card_screen.dart';


class MainDashboard extends StatefulWidget {
  const MainDashboard({Key? key}) : super(key: key);

  @override
  State<MainDashboard> createState() => _MainDashboardState();
}

class _MainDashboardState extends State<MainDashboard> {
  int _activeTabIdx = 0;
  String _currentSettingsSection = 'categories';
  final AudioSynthService _audioSynth = AudioSynthService();

  late bool _isMsLuna;
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
  final TextEditingController _homeMessageController = TextEditingController();
  final TextEditingController _marqueeController = TextEditingController();
  final TextEditingController _customMelodyController = TextEditingController();
  final TextEditingController _aiPromptController = TextEditingController();
  final TextEditingController _searchController = TextEditingController();
  final TextEditingController _calendarTitleController = TextEditingController();
  final TextEditingController _lunaLinkSearchController = TextEditingController();
  final TextEditingController _profileDisplayNameController = TextEditingController();
  final TextEditingController _profileRobotNameController = TextEditingController();
  String? _profileSelectedVariant;
  bool _isProfileInitialized = false;
  StreamSubscription? _remoteTriggerSub;
  
  String _selectedCategory = 'ALL';
  String _selectedSort = 'name';
  String _selectedEventType = 'meeting';
  DateTime _selectedEventDateTime = DateTime.now();
  DateTime _calendarViewDate = DateTime.now();
  bool _isSettingsSaving = false;
  bool _isCompiling = false;
  String _localActiveGifId = 'sprite_ai_0';
  String _localActiveLabel = 'Sprite AI 1';
  bool _isStreamingMap = false;

  List<Map<String, String>> _localAudioFiles = [];
  bool _isLoadingAudioFiles = false;
  Map<String, String>? _currentlyPlayingFile;
  String _audioSearchQuery = "";
  final TextEditingController _audioSearchController = TextEditingController();

  void _checkAlarms(DatabaseService db, BLEService ble) {
    final now = DateTime.now();
    if (now.second == 0) {
      for (final alarm in db.alarms) {
        if (alarm.isEnabled && alarm.hour == now.hour && alarm.minute == now.minute) {
          _triggerAlarm(alarm, ble);
          break;
        }
      }
    }
  }

  void _triggerAlarm(AlarmModel alarm, BLEService ble) {
    if (_ringingAlarm != null) return;
    setState(() {
      _ringingAlarm = alarm;
    });

    if (ble.isConnected) {
      ble.transmitMarqueeText("ALARM: ${alarm.label.toUpperCase()}");
      ble.transmitAudio(3);
    }

    _alarmSoundTimer?.cancel();
    _alarmSoundTimer = Timer.periodic(const Duration(seconds: 2), (timer) {
      if (ble.isConnected) {
        ble.transmitAudio(3);
      }
    });

    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (ctx) => WillPopScope(
        onWillPop: () async => false,
        child: AlertDialog(
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
          backgroundColor: const Color(0xFF1E293B),
          title: Row(
            children: const [
              Icon(Icons.alarm, color: Colors.redAccent),
              SizedBox(width: 8),
              Text("Hardware Alarm Ringing!", style: TextStyle(color: Colors.white)),
            ],
          ),
          content: Text(
            "Alarm pushed to Luna hardware: ${alarm.formatTime(Provider.of<DatabaseService>(context, listen: false).is12HourFormat)}",
            style: const TextStyle(color: Colors.white70, fontSize: 14),
          ),
          actions: [
            ElevatedButton(
              style: ElevatedButton.styleFrom(backgroundColor: Colors.redAccent),
              onPressed: () {
                _dismissAlarm();
                Navigator.of(ctx).pop();
              },
              child: const Text("Dismiss"),
            ),
          ],
        ),
      ),
    );
  }

  void _dismissAlarm() {
    _alarmSoundTimer?.cancel();
    _alarmSoundTimer = null;
    setState(() {
      _ringingAlarm = null;
    });
  }

  Future<void> _checkNotificationPermission() async {
    final service = Provider.of<PhoneNotificationService>(context, listen: false);
    final granted = await service.isPermissionGranted();
    if (mounted && _isNotificationPermissionGranted != granted) {
      setState(() {
        _isNotificationPermissionGranted = granted;
      });
    }
    final postGranted = await service.isPostNotificationsPermissionGranted();
    if (mounted && _isPostNotificationsPermissionGranted != postGranted) {
      setState(() {
        _isPostNotificationsPermissionGranted = postGranted;
      });
    }
  }

  Future<void> _requestNotificationPermission() async {
    final service = Provider.of<PhoneNotificationService>(context, listen: false);
    final postGranted = await service.isPostNotificationsPermissionGranted();
    if (!postGranted) {
      await service.requestPostNotificationsPermission();
    }
    final listenerGranted = await service.isPermissionGranted();
    if (!listenerGranted) {
      await service.openSettings();
    }
    await service.startBackgroundService();
  }

  Future<void> _compileFirmware() async {
    setState(() {
      _isCompiling = true;
    });
    final ble = Provider.of<BLEService>(context, listen: false);
    try {
      final response = await http.post(
        Uri.parse('http://${ble.serverIp}:8000/api/compile'),
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
        "Could not connect to local compilation server at http://${ble.serverIp}:8000.\n\n"
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
  Timer? _udpDiscoveryTimer;
  Timer? _clockTickerTimer;
  final Set<String> _notifiedEventIds = {};
  String? _uploadedFileBase64;
  int _uploadedFrameCount = 8;
  double _uploadCompression = 30.0;
  Uint8List? _selectedWallpaperBytes;
  String? _selectedWallpaperName;
  bool _isUploadingWallpaper = false;
  double _wallpaperUploadProgress = 0.0;
  AlarmModel? _ringingAlarm;
  Timer? _alarmSoundTimer;
  StreamSubscription? _robotEventsSub;
  bool _isNotificationPermissionGranted = false;
  bool _isPostNotificationsPermissionGranted = false;
  final TextEditingController _appSearchController = TextEditingController();
  List<Map<String, String>> _installedApps = [];
  bool _isLoadingApps = false;

  @override
  void initState() {
    super.initState();
    
    _clockTickerTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      if (mounted) {
        final db = Provider.of<DatabaseService>(context, listen: false);
        final ble = Provider.of<BLEService>(context, listen: false);

        // Dynamically detect and persist firmware version updates
        for (final robot in db.robots) {
          if (ble.isConnected && ble.connectedDevice?.remoteId.str == robot.remoteId) {
            final version = ble.hasSpeaker ? 2 : 1;
            if (robot.firmwareVersion != version) {
              db.updateRobotFirmwareVersion(robot.id, version);
            }
          } else if (ble.isCompanionConnected && ble.companionDevice?.remoteId.str == robot.remoteId) {
            final version = ble.companionHasSpeaker ? 2 : 1;
            if (robot.firmwareVersion != version) {
              db.updateRobotFirmwareVersion(robot.id, version);
            }
          }
        }

        _checkAlarms(db, ble);
        _checkNotificationPermission();
        setState(() {});
      }
    });
    // Default melody notation (Luna Power-up)
    _customMelodyController.text =
        "E5 50 10\nE5 50 10\nE5 50 30\nC5 50 10\nE5 50 30\nG5 50 50\nG4 50 50\nC5 50 10\nG4 50 30\nE4 50 10\nA4 50 10\nB4 50 10\nAS4 50 10\nA4 50 30\nG4 50 20\nE5 50 10\nG5 50 10\nA5 50 10\nF5 50 10\nG5 50 10\nE5 50 10\nC5 50 10\nD5 50 10\nB4 50 50";
    
    // Poll the cloud connectivity status of all paired robots (every 15s)
    _cloudPollTimer = Timer.periodic(const Duration(seconds: 15), (timer) {
      if (mounted) {
        _pollCloudStatus();
      }
    });

    // Check calendar scheduled meetings/birthdays every 30 seconds
    _calendarSchedulerTimer = Timer.periodic(const Duration(seconds: 30), (timer) {
      if (mounted) {
        _checkCalendarScheduledEvents();
      }
    });

    // Discover the server IP immediately
    _discoverServer();
    _scanLocalAudioFiles();

    // Periodically run UDP discovery every 30 seconds to detect server IP changes
    _udpDiscoveryTimer = Timer.periodic(const Duration(seconds: 30), (timer) {
      if (mounted) {
        _discoverServer();
      }
    });

    final ble = Provider.of<BLEService>(context, listen: false);
    _robotEventsSub = ble.robotEvents.listen((event) {
      if (event == "ALARM:DISMISS") {
        if (_ringingAlarm != null) {
          _dismissAlarm();
          Navigator.of(context).popUntil((route) => route.isFirst);
        }
      }
    });

    final firebase = Provider.of<FirebaseService>(context, listen: false);
    final db = Provider.of<DatabaseService>(context, listen: false);
    ble.onPrimaryTouchTriggered = (eventType, expr, sound) {
      if (firebase.pairedFriendUid != null) {
        String customLabel = eventType;
        if (expr == 0) customLabel = 'IDLE';
        else if (expr == 1) customLabel = 'HAPPY';
        else if (expr == 2) customLabel = 'SAD';
        else if (expr == 3) customLabel = 'ANGRY';
        else if (expr == 4) customLabel = 'SURPRISED';
        else if (expr == 5) customLabel = 'SLEEPING';
        else if (expr == 6) customLabel = 'WINK';
        else if (expr >= 100) {
          final idx = expr - 100;
          final keys = DatabaseService.animMapping.keys.toList();
          if (idx >= 0 && idx < keys.length) {
            customLabel = DatabaseService.animMapping[keys[idx]]!['label'] ?? eventType;
          }
        }
        firebase.sendCloudTrigger(eventType, expr, sound, customLabel: customLabel);
      }
    };

    ble.onSettingsSyncedFromWatch = (clockStyle, brightness, negative, silent) async {
      if (mounted) {
        final db = Provider.of<DatabaseService>(context, listen: false);
        await db.updateClockStyle(clockStyle);
        await db.updateOledBrightness(brightness.toDouble());
        await db.updateNegativeEnabled(negative);
        await db.updateSilentMode(silent);
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text("Settings synchronized from watch (Buzzer: ${silent ? 'Muted' : 'Sound On'})"),
            backgroundColor: Colors.green.shade700,
          ),
        );
      }
    };

    _remoteTriggerSub = firebase.remoteTriggers.listen((data) {
      final senderName = data['senderName'] ?? "Friend";
      final eventType = data['eventType'] ?? "TAP";
      final exprLabel = data['exprLabel'] ?? "HAPPY";
      final soundId = data['soundId'] ?? 2;
      
      int exprId = 1;
      switch (exprLabel.toUpperCase()) {
        case "IDLE": case "IDLE/BLANK": exprId = 0; break;
        case "HAPPY": exprId = 1; break;
        case "SAD": exprId = 2; break;
        case "ANGRY": exprId = 3; break;
        case "SURPRISED": case "SURPRISE": exprId = 4; break;
        case "SLEEPING": case "SLEEP": exprId = 5; break;
        case "WINK": exprId = 6; break;
        default:
          final lowKey = exprLabel.toLowerCase();
          int foundIdx = -1;
          final keysList = DatabaseService.animMapping.keys.toList();
          for (int i = 0; i < keysList.length; i++) {
            if (keysList[i] == lowKey || DatabaseService.animMapping[keysList[i]]!['label'].toString().toLowerCase() == lowKey) {
              foundIdx = i;
              break;
            }
          }
          if (foundIdx != -1) {
            exprId = 100 + foundIdx;
          } else {
            exprId = 1;
          }
      }
      
      ble.handleRemoteCloudTrigger(exprId, soundId, senderName, eventType, customLabel: exprLabel);
      
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Row(
            children: [
              const Icon(Icons.cloud_sync, color: Colors.white),
              const SizedBox(width: 10),
              Text("$senderName sent '$exprLabel'!"),
            ],
          ),
          backgroundColor: _accentColor,
          duration: const Duration(seconds: 4),
          behavior: SnackBarBehavior.floating,
        ),
      );
    });

    _loadInstalledApps();
  }

  @override
  void dispose() {
    _cloudPollTimer?.cancel();
    _calendarSchedulerTimer?.cancel();
    _udpDiscoveryTimer?.cancel();
    _clockTickerTimer?.cancel();
    _robotEventsSub?.cancel();
    _remoteTriggerSub?.cancel();
    _homeMessageController.dispose();
    _marqueeController.dispose();
    _customMelodyController.dispose();
    _aiPromptController.dispose();
    _searchController.dispose();
    _lunaLinkSearchController.dispose();
    _appSearchController.dispose();
    _audioSearchController.dispose();
    _audioSynth.stop();
    super.dispose();
  }

  Future<void> _pollCloudStatus() async {
    try {
      final db = Provider.of<DatabaseService>(context, listen: false);
      final ble = Provider.of<BLEService>(context, listen: false);
      final response = await http.get(Uri.parse('http://${ble.serverIp}:8000/api/robots'))
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

            // Hardware sound triggers for Calendar Reminders / Birthdays / Alarms
            int sfxId = 1;
            if (event.type == 'birthday') {
              sfxId = 3; // 1-Up Melody for Birthday
            } else if (event.type == 'alarm') {
              sfxId = 2; // Super Mushroom / Alarm
            } else if (event.type == 'reminder') {
              sfxId = 4; // Stomp SFX / Reminder
            }
            ble.transmitAudio(sfxId);

            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(
                content: Text("Event '${event.title}' pushed to Luna hardware!"),
                backgroundColor: _accentColor,
              ),
            );
          }
        }
      }
    }
  }

  void _discoverServer() async {
    try {
      final ble = Provider.of<BLEService>(context, listen: false);
      final socket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);
      socket.broadcastEnabled = true;
      socket.send(utf8.encode("MR_LUNA_DISCOVER"), InternetAddress("255.255.255.255"), 8002);
      
      await for (final event in socket.timeout(const Duration(seconds: 2), onTimeout: (sink) => sink.close())) {
        if (event == RawSocketEvent.read) {
          final datagram = socket.receive();
          if (datagram != null) {
            final response = utf8.decode(datagram.data);
            if (response == "MR_LUNA_SERVER_HERE") {
              final ip = datagram.address.address;
              ble.setServerIp(ip);
              break;
            }
          }
        }
      }
      socket.close();
    } catch (e) {
      // Quietly ignore UDP binding/network errors
    }
  }

  Future<void> _scanLocalAudioFiles() async {
    if (_isLoadingAudioFiles) return;
    setState(() {
      _isLoadingAudioFiles = true;
    });

    try {
      if (Platform.isAndroid) {
        if (await Permission.audio.request().isGranted || await Permission.storage.request().isGranted) {
          const channel = MethodChannel('com.mrmsluna/notifications');
          final List<dynamic>? files = await channel.invokeMethod<List<dynamic>>('getLocalAudioFiles');
          if (files != null) {
            final List<Map<String, String>> list = files.map((f) => Map<String, String>.from(f as Map)).toList();
            setState(() {
              _localAudioFiles = list;
              _isLoadingAudioFiles = false;
            });
            return;
          }
        }
      }

      // Fallback/Legacy directory listing
      final List<Map<String, String>> files = [];
      final searchPaths = [
        '/sdcard/Music',
        '/sdcard/Download',
        '/storage/emulated/0/Music',
        '/storage/emulated/0/Download',
      ];

      for (final path in searchPaths) {
        final dir = Directory(path);
        if (await dir.exists()) {
          try {
            final entities = dir.listSync(recursive: true, followLinks: false);
            for (final entity in entities) {
              if (entity is File) {
                final ext = entity.path.split('.').last.toLowerCase();
                if (ext == 'mp3' || ext == 'wav') {
                  files.add({
                    'path': entity.path,
                    'name': entity.path.split('/').last,
                    'title': entity.path.split('/').last.split('.').first,
                  });
                }
              }
            }
          } catch (e) {
            // Ignore directory errors
          }
        }
      }

      setState(() {
        _localAudioFiles = files;
        _isLoadingAudioFiles = false;
      });
    } catch (e) {
      setState(() {
        _isLoadingAudioFiles = false;
      });
    }
  }

  Future<bool> _ensureServerIpConfigured(BLEService ble) async {
    if (ble.serverIp != 'localhost' && ble.serverIp.isNotEmpty) {
      return true;
    }
    
    // Attempt UDP discovery immediately
    _discoverServer();
    
    // Give it a brief delay to receive response
    await Future.delayed(const Duration(milliseconds: 1200));
    
    if (ble.serverIp != 'localhost' && ble.serverIp.isNotEmpty) {
      return true;
    }
    
    ScaffoldMessenger.of(context).showSnackBar(
      const SnackBar(
        content: Text("Robot IP not found. Ensure both Phone and Robot are connected to the same Wi-Fi network."),
        backgroundColor: Colors.redAccent,
      ),
    );
    return false;
  }

  void _showLocalAudioFilesDialog(BLEService ble, AudioStreamService audioStream) {
    final isMiss = _isMsLuna;
    final accentColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFFE53935);

    showGeneralDialog(
      context: context,
      barrierDismissible: true,
      barrierLabel: "Local Audio Library",
      transitionDuration: const Duration(milliseconds: 300),
      transitionBuilder: (context, anim1, anim2, child) {
        return SlideTransition(
          position: Tween<Offset>(
            begin: const Offset(0, 1),
            end: Offset.zero,
          ).animate(CurvedAnimation(parent: anim1, curve: Curves.easeOut)),
          child: child,
        );
      },
      pageBuilder: (ctx, anim1, anim2) {
        return StatefulBuilder(
          builder: (context, setDialogState) {
            return AnimatedBuilder(
              animation: audioStream,
              builder: (context, _) {
                final filteredFiles = _localAudioFiles.where((file) {
                  final name = (file['name'] ?? '').toLowerCase();
                  final title = (file['title'] ?? '').toLowerCase();
                  return name.contains(_audioSearchQuery) || title.contains(_audioSearchQuery);
                }).toList();

                return Scaffold(
                  backgroundColor: Colors.white,
                  appBar: AppBar(
                    elevation: 0.5,
                    backgroundColor: Colors.white,
                    leading: IconButton(
                      icon: const Icon(Icons.arrow_back, color: Color(0xFF0F172A)),
                      onPressed: () => Navigator.pop(ctx),
                    ),
                    title: Text(
                      "LOCAL AUDIO LIBRARY",
                      style: GoogleFonts.outfit(
                        color: const Color(0xFF0F172A),
                        fontWeight: FontWeight.bold,
                        fontSize: 16,
                      ),
                    ),
                    actions: [
                      IconButton(
                        icon: Icon(Icons.refresh, color: accentColor),
                        onPressed: () async {
                          setState(() => _isLoadingAudioFiles = true);
                          setDialogState(() {});
                          await _scanLocalAudioFiles();
                          setState(() => _isLoadingAudioFiles = false);
                          setDialogState(() {});
                        },
                        tooltip: "Scan files",
                      ),
                    ],
                  ),
                  body: SafeArea(
                    child: Column(
                      children: [
                        // Search bar
                        Padding(
                          padding: const EdgeInsets.all(16.0),
                          child: Container(
                            padding: const EdgeInsets.symmetric(horizontal: 12),
                            decoration: BoxDecoration(
                              color: const Color(0xFFF1F5F9),
                              borderRadius: BorderRadius.circular(12),
                              border: Border.all(color: const Color(0xFFE2E8F0)),
                            ),
                            child: Row(
                              children: [
                                const Icon(Icons.search, color: Color(0xFF64748B), size: 20),
                                const SizedBox(width: 8),
                                Expanded(
                                  child: TextField(
                                    controller: _audioSearchController,
                                    style: GoogleFonts.outfit(color: const Color(0xFF0F172A), fontSize: 14),
                                    decoration: InputDecoration(
                                      hintText: "Search local files...",
                                      hintStyle: GoogleFonts.outfit(color: const Color(0xFF94A3B8), fontSize: 14),
                                      border: InputBorder.none,
                                      isDense: true,
                                      contentPadding: const EdgeInsets.symmetric(vertical: 12),
                                    ),
                                    onChanged: (val) {
                                      setDialogState(() {
                                        _audioSearchQuery = val.trim().toLowerCase();
                                      });
                                    },
                                  ),
                                ),
                                if (_audioSearchQuery.isNotEmpty)
                                  GestureDetector(
                                    onTap: () {
                                      _audioSearchController.clear();
                                      setDialogState(() {
                                        _audioSearchQuery = "";
                                      });
                                    },
                                    child: const Icon(Icons.close, color: Color(0xFF64748B), size: 18),
                                  ),
                              ],
                            ),
                          ),
                        ),
                        const Divider(height: 1, color: Color(0xFFE2E8F0)),
                        Expanded(
                          child: _isLoadingAudioFiles
                              ? Center(
                                  child: CircularProgressIndicator(color: accentColor),
                                )
                              : filteredFiles.isEmpty
                                  ? Center(
                                      child: Padding(
                                        padding: const EdgeInsets.all(24.0),
                                        child: Text(
                                          _audioSearchQuery.isNotEmpty
                                              ? "No matching tracks found."
                                              : "No MP3 or WAV files found on phone.\nPlace files in /sdcard/Music or /sdcard/Download.",
                                          textAlign: TextAlign.center,
                                          style: GoogleFonts.outfit(
                                            color: const Color(0xFF64748B),
                                            fontSize: 14,
                                          ),
                                        ),
                                      ),
                                    )
                                  : ListView.builder(
                                      itemCount: filteredFiles.length,
                                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                                      itemBuilder: (ctx, idx) {
                                        final file = filteredFiles[idx];
                                        final filename = file['name'] ?? '';
                                        final cleanTitle = file['title'] ?? filename.split('.').first;
                                        final isWav = (file['path'] ?? '').endsWith('.wav');
                                        final isCurrent = _currentlyPlayingFile == file && audioStream.isStreamingMusic;

                                        return Card(
                                          color: isCurrent ? accentColor.withOpacity(0.06) : Colors.white,
                                          elevation: isCurrent ? 0 : 0.5,
                                          margin: const EdgeInsets.symmetric(vertical: 6),
                                          shape: RoundedRectangleBorder(
                                            borderRadius: BorderRadius.circular(12),
                                            side: BorderSide(
                                              color: isCurrent ? accentColor.withOpacity(0.4) : const Color(0xFFE2E8F0),
                                              width: isCurrent ? 1.5 : 1,
                                            ),
                                          ),
                                          child: InkWell(
                                            onTap: () async {
                                              await _playLocalAudioFile(file, ble, audioStream);
                                              setDialogState(() {});
                                            },
                                            borderRadius: BorderRadius.circular(12),
                                            child: Padding(
                                              padding: const EdgeInsets.all(12.0),
                                              child: Row(
                                                children: [
                                                  Container(
                                                    width: 42,
                                                    height: 42,
                                                    decoration: BoxDecoration(
                                                      color: isCurrent ? accentColor.withOpacity(0.15) : const Color(0xFFF1F5F9),
                                                      borderRadius: BorderRadius.circular(8),
                                                    ),
                                                    child: Icon(
                                                      isCurrent
                                                          ? (audioStream.isMusicPaused ? Icons.play_arrow : Icons.equalizer)
                                                          : (isWav ? Icons.audiotrack : Icons.music_note),
                                                      color: isCurrent ? accentColor : const Color(0xFF64748B),
                                                      size: 20,
                                                    ),
                                                  ),
                                                  const SizedBox(width: 12),
                                                  Expanded(
                                                    child: Column(
                                                      crossAxisAlignment: CrossAxisAlignment.start,
                                                      children: [
                                                        Text(
                                                          cleanTitle,
                                                          style: GoogleFonts.outfit(
                                                            color: isCurrent ? accentColor : const Color(0xFF0F172A),
                                                            fontSize: 14,
                                                            fontWeight: isCurrent ? FontWeight.bold : FontWeight.w600,
                                                          ),
                                                          maxLines: 1,
                                                          overflow: TextOverflow.ellipsis,
                                                        ),
                                                        const SizedBox(height: 4),
                                                        Text(
                                                          "${isWav ? 'WAV Audio' : 'MP3 Audio'} • $filename",
                                                          style: GoogleFonts.outfit(
                                                            color: isCurrent ? accentColor.withOpacity(0.7) : const Color(0xFF64748B),
                                                            fontSize: 11,
                                                          ),
                                                          maxLines: 1,
                                                          overflow: TextOverflow.ellipsis,
                                                        ),
                                                      ],
                                                    ),
                                                  ),
                                                  Icon(
                                                    isCurrent
                                                        ? (audioStream.isMusicPaused ? Icons.play_arrow : Icons.pause_circle_filled)
                                                        : Icons.play_circle_filled,
                                                    color: isCurrent ? accentColor : const Color(0xFFCBD5E1),
                                                    size: 28,
                                                  ),
                                                ],
                                              ),
                                            ),
                                          ),
                                        );
                                      },
                                    ),
                        ),
                        // Bottom Now Playing Dock
                        if (audioStream.isStreamingMusic && _currentlyPlayingFile != null)
                          Container(
                            padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 16),
                            decoration: BoxDecoration(
                              color: Colors.white,
                              boxShadow: [
                                BoxShadow(
                                  color: Colors.black.withOpacity(0.06),
                                  blurRadius: 10,
                                  offset: const Offset(0, -3),
                                )
                              ],
                              border: const Border(
                                top: BorderSide(color: Color(0xFFE2E8F0), width: 1),
                              ),
                            ),
                            child: Column(
                              mainAxisSize: MainAxisSize.min,
                              children: [
                                Row(
                                  children: [
                                    Expanded(
                                      child: Column(
                                        crossAxisAlignment: CrossAxisAlignment.start,
                                        children: [
                                          Text(
                                            _currentlyPlayingFile?['title'] ?? _currentlyPlayingFile?['name'] ?? "Now Playing",
                                            style: GoogleFonts.outfit(
                                              color: const Color(0xFF0F172A),
                                              fontWeight: FontWeight.bold,
                                              fontSize: 14,
                                            ),
                                            maxLines: 1,
                                            overflow: TextOverflow.ellipsis,
                                          ),
                                          const SizedBox(height: 2),
                                          Text(
                                            audioStream.statusMessage ?? "Streaming to Companion...",
                                            style: GoogleFonts.outfit(
                                              color: const Color(0xFF64748B),
                                              fontSize: 11,
                                            ),
                                          ),
                                        ],
                                      ),
                                    ),
                                  ],
                                ),
                                const SizedBox(height: 8),
                                Row(
                                  children: [
                                    Text(
                                      _formatPcmDuration(audioStream.musicStreamOffset),
                                      style: GoogleFonts.firaCode(
                                        color: const Color(0xFF64748B),
                                        fontSize: 11,
                                      ),
                                    ),
                                    Expanded(
                                      child: Slider(
                                        value: audioStream.musicStreamOffset.toDouble().clamp(
                                          0.0,
                                          audioStream.musicStreamTotalSize.toDouble() > 0
                                              ? audioStream.musicStreamTotalSize.toDouble()
                                              : 1.0,
                                        ),
                                        min: 0.0,
                                        max: audioStream.musicStreamTotalSize.toDouble() > 0
                                            ? audioStream.musicStreamTotalSize.toDouble()
                                            : 1.0,
                                        activeColor: accentColor,
                                        inactiveColor: const Color(0xFFE2E8F0),
                                        onChanged: (val) {
                                          audioStream.seekMusic(val.toInt());
                                        },
                                      ),
                                    ),
                                    Text(
                                      _formatPcmDuration(audioStream.musicStreamTotalSize),
                                      style: GoogleFonts.firaCode(
                                        color: const Color(0xFF64748B),
                                        fontSize: 11,
                                      ),
                                    ),
                                  ],
                                ),
                                Row(
                                  mainAxisAlignment: MainAxisAlignment.center,
                                  children: [
                                    IconButton(
                                      icon: Icon(
                                        audioStream.isMusicPaused ? Icons.play_arrow : Icons.pause,
                                        color: const Color(0xFF0F172A),
                                        size: 32,
                                      ),
                                      onPressed: () {
                                        if (audioStream.isMusicPaused) {
                                          audioStream.resumeMusic();
                                        } else {
                                          audioStream.pauseMusic();
                                        }
                                      },
                                    ),
                                    const SizedBox(width: 24),
                                    IconButton(
                                      icon: const Icon(
                                        Icons.stop,
                                        color: Color(0xFFE53935),
                                        size: 32,
                                      ),
                                      onPressed: () async {
                                        await _stopMusic(audioStream, ble);
                                        setDialogState(() {});
                                      },
                                    ),
                                  ],
                                )
                              ],
                            ),
                          ),
                      ],
                    ),
                  ),
                );
              },
            );
          },
        );
      },
    );
  }

  Future<void> _playLocalAudioFile(Map<String, String> fileMap, BLEService ble, AudioStreamService audioStream) async {
    final path = fileMap['path'] ?? '';
    if (path.isEmpty) return;

    // If already playing, stop first
    if (audioStream.isStreamingMusic) {
      await _stopMusic(audioStream, ble);
      if (_currentlyPlayingFile == fileMap) return; // toggle off
    }

    setState(() => _currentlyPlayingFile = fileMap);
    audioStream.statusMessage = "Starting...";

    try {
      // Stream-decode + BLE-send simultaneously. No full-file decode wait.
      final success = await audioStream.startMusicStreamBLEFromPath(ble, path);
      if (!success) setState(() => _currentlyPlayingFile = null);
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text("Playback error: $e")),
      );
      setState(() => _currentlyPlayingFile = null);
      audioStream.statusMessage = "Error: $e";
    }
  }


  Future<void> _stopMusic(AudioStreamService audioStream, BLEService ble) async {
    if (audioStream.isBleStreaming) {
      await audioStream.stopMusicStreamBLEFromPath(ble); // sends MUSIC:STOP instantly
      await audioStream.stopMusicStreamBLE();
    } else {
      await audioStream.stopMusicStream();
      await ble.transmitStopMusic(); // ACK-based stop for non-BLE-stream path
    }
    setState(() => _currentlyPlayingFile = null);
  }

  String _formatPcmDuration(int bytes) {
    final seconds = bytes ~/ 32000;
    final m = seconds ~/ 60;
    final s = seconds % 60;
    return "${m.toString().padLeft(2, '0')}:${s.toString().padLeft(2, '0')}";
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
        backgroundColor: const Color(0xFFE53935),
      ),
    );
  }

  // Sync settings helper
  Future<void> _syncSettingsToRobot(DatabaseService db, BLEService ble) async {
    setState(() {
      _isSettingsSaving = true;
    });

    final firebase = Provider.of<FirebaseService>(context, listen: false);
    final isCloudPaired = firebase.pairedFriendUid != null;

    final defaultGifVal = db.defaultGif;
    final introGifVal = db.introGif;
    final touchSingleVal = isCloudPaired ? 'default' : db.touchSingle;
    final touchDoubleVal = isCloudPaired ? 'default' : db.touchDouble;
    final touchLongVal = isCloudPaired ? 'default' : db.touchLong;

    // Helper map matches JS index calculation
    int getExpressionValue(String val) {
      if (val == 'cycle') return 99;
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
      notificationDurationSec: db.notificationDuration.round(),
      reminderDurationSec: db.reminderDuration.round(),
      birthdayDurationSec: db.birthdayDuration.round(),
      clockStyle: db.clockStyle,
      oledBrightness: db.oledBrightness.round(),
      silentMode: db.silentMode,
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
                                ? "Searching for Mr.&Ms Luna companion robot..."
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
                                          : "Mr. Luna Robot";
                                      final isMiss = name.toLowerCase().contains("ms") || name.toLowerCase().contains("miss");
                                      final newRobot = RobotProfile(
                                        id: id,
                                        name: name,
                                        variant: isMiss ? 'ms_luna' : 'mr_luna',
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

    // Auto-connect companion if paired and primary is connected
    if (ble.isConnected && db.primaryRobot != null && db.primaryRobot!.companionDeviceId != null) {
      final compId = db.primaryRobot!.companionDeviceId!;
      if (!ble.isCompanionConnected && !ble.isCompanionConnecting) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          ble.connectCompanionById(compId);
        });
      }
    }

    // Dynamic theme: Ms. Luna = pink, Mr. Luna = blue
    final isMsLuna = db.primaryRobot?.variant == 'ms_luna';
    _accentColor = isMsLuna ? const Color(0xFFE91E8C) : const Color(0xFF0284C7);
    _accentColorLight = isMsLuna ? const Color(0x1FE91E8C) : const Color(0x1F0284C7);
    // Secondary accent (red for Mr. Luna, rose for Ms. Luna)
    final Color _secondaryColor = isMsLuna ? const Color(0xFFEC4899) : const Color(0xFFE53935);

    // â”€â”€ Resolve the active GIF to show in the simulator â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Priority: BLE label (exact match) â†’ BLE exprId fallback â†’ local state
    String activeGifId = _localActiveGifId;
    String activeLabel = _localActiveLabel;

    if (ble.isConnected) {
      final rawLabel = ble.activeExpressionLabel.trim();
      final upperLabel = rawLabel.toUpperCase();

      if (upperLabel.isNotEmpty && upperLabel != 'IDLE') {
        // 1. Try to find the GIF by matching the label from the hardware status packet
        //    (using robust normalized matching: e.g. "look left" matches "Look Left" or "left")
        GifModel? match;
        try {
          String normalize(String s) => s.replaceAll(RegExp(r'[^a-zA-Z0-9]'), '').toLowerCase();
          final normLabel = normalize(rawLabel);
          
          match = db.gifs.firstWhere(
            (g) => normalize(g.id) == normLabel || normalize(g.name) == normLabel,
          );
        } catch (_) {
          match = null;
        }

        if (match != null) {
          // Perfect match by id or name
          activeGifId = match.id;
          activeLabel = match.name;
        } else {
          // 2. Fallback: use exprId to resolve the 7 standard expressions
          final exprId = ble.activeExpressionId;
          switch (exprId) {
            case 0: activeGifId = 'relaxed';  activeLabel = 'Idle';      break;
            case 1: activeGifId = 'happy';    activeLabel = 'Happy';     break;
            case 2: activeGifId = 'crying';   activeLabel = 'Sad';       break;
            case 3: activeGifId = 'angry';    activeLabel = 'Angry';     break;
            case 4: activeGifId = 'surprised';activeLabel = 'Surprised'; break;
            case 5: activeGifId = 'sleepy';   activeLabel = 'Sleeping';  break;
            case 6: activeGifId = 'wink';     activeLabel = 'Wink';      break;
            case 7: activeGifId = 'clock';    activeLabel = 'Clock';     break;
            default:
              // Keep local state for unknown expressions, but show raw label (e.g. ARCADE, SNAKE, etc.)
              activeLabel = rawLabel;
              break;
          }
        }
      } else if (upperLabel == 'IDLE' || upperLabel.isEmpty) {
        // Hardware is idle â€” show the default idle GIF
        activeGifId = 'relaxed';
        activeLabel = 'Idle';
      }
    }


    return Scaffold(
      backgroundColor: Colors.white,
      body: LunaBackground(
        isMsLuna: isMsLuna,
        child: Stack(
          children: [
          SafeArea(
            child: Column(
              children: [
                // Top sticky navigation bar
                _buildTopNavigation(ble),

                Expanded(
                  child: Padding(
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    child: _buildPanelContent(db, ble, activeGifId, activeLabel),
                  ),
                ),
              ],
            ),
          ),
        ],
        ),
      ),
      bottomNavigationBar: Container(
        color: Colors.white,
        child: _buildBottomNavigationBar(),
      ),
    );
  }

  // Floating capsule Bottom Navigation Bar
  Widget _buildBottomNavigationBar() {
    final List<Map<String, dynamic>> items = [
      {'icon': Icons.home, 'label': 'Home'},
      {'icon': Icons.face, 'label': 'Expressions'},
      {'icon': Icons.audiotrack, 'label': 'Sounds'},
      {'icon': Icons.cloud_sync, 'label': 'Luna Link'},
      {'icon': Icons.calendar_month, 'label': 'Calendar'},
      {'icon': Icons.contact_mail, 'label': 'Card'},
      {'icon': Icons.person, 'label': 'Profile'},
    ];

    return Container(
      margin: const EdgeInsets.only(left: 16, right: 16, bottom: 16),
      height: 68,
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(24),
        border: Border.all(color: _accentColor.withOpacity(0.15), width: 1.2),
        boxShadow: [
          BoxShadow(
            color: _accentColor.withOpacity(0.10),
            blurRadius: 18,
            spreadRadius: 1,
            offset: const Offset(0, 4),
          ),
          BoxShadow(
            color: Colors.black.withOpacity(0.04),
            blurRadius: 8,
            offset: const Offset(0, 2),
          ),
        ],
      ),
      child: ClipRRect(
        borderRadius: BorderRadius.circular(24),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          children: List.generate(items.length, (idx) {
            final isSelected = _activeTabIdx == idx;
            return GestureDetector(
              onTap: () {
                setState(() {
                  _activeTabIdx = idx;
                  if (idx == 6) {
                    _currentSettingsSection = 'categories';
                  }
                });
              },
              child: AnimatedContainer(
                duration: const Duration(milliseconds: 220),
                curve: Curves.easeOutCubic,
                padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
                decoration: BoxDecoration(
                  color: isSelected ? _accentColor : Colors.transparent,
                  borderRadius: BorderRadius.circular(16),
                ),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    AnimatedScale(
                      scale: isSelected ? 1.15 : 1.0,
                      duration: const Duration(milliseconds: 200),
                      child: Icon(
                        items[idx]['icon'] as IconData,
                        color: isSelected ? Colors.white : const Color(0xFF9E9E9E),
                        size: 20,
                      ),
                    ),
                    const SizedBox(height: 3),
                    Text(
                      items[idx]['label'] as String,
                      style: GoogleFonts.outfit(
                        color: isSelected ? Colors.white : const Color(0xFF9E9E9E),
                        fontSize: 9.5,
                        fontWeight: isSelected ? FontWeight.w700 : FontWeight.w500,
                        letterSpacing: 0.2,
                      ),
                    ),
                  ],
                ),
              ),
            );
          }),
        ),
      ),
    );
  }

  // Status indicator and version badges helper methods
  Widget _buildStatusDot(bool connected) {
    return Container(
      width: 6,
      height: 6,
      decoration: BoxDecoration(
        color: connected ? Colors.green : Colors.red,
        shape: BoxShape.circle,
      ),
    );
  }

  Widget _buildVersionBadge(bool hasSpeaker) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 4, vertical: 1),
      decoration: BoxDecoration(
        color: hasSpeaker ? Colors.green.shade50 : Colors.orange.shade50,
        borderRadius: BorderRadius.circular(3),
        border: Border.all(
          color: hasSpeaker ? Colors.green.shade200 : Colors.orange.shade200,
          width: 0.6,
        ),
      ),
      child: Text(
        hasSpeaker ? "v2" : "v1",
        style: GoogleFonts.outfit(
          color: hasSpeaker ? Colors.green.shade700 : Colors.orange.shade700,
          fontSize: 7.5,
          fontWeight: FontWeight.bold,
        ),
      ),
    );
  }

  // Top header navbar
  Widget _buildTopNavigation(BLEService ble) {
    final db = Provider.of<DatabaseService>(context, listen: false);
    _isMsLuna = db.primaryRobot?.variant == 'ms_luna';
    final primaryRobot = db.primaryRobot;
    final hasRelationship = primaryRobot != null && primaryRobot.companionDeviceId != null;
    String primaryName = primaryRobot?.name ?? (_isMsLuna ? "Ms. Luna Robot" : "Mr. Luna Robot");
    if (_isMsLuna && (primaryName == "Mr. Luna Robot" || primaryName == "Mr. Luna")) {
      primaryName = "Ms. Luna Robot";
    } else if (!_isMsLuna && (primaryName == "Ms. Luna Robot" || primaryName == "Ms. Luna")) {
      primaryName = "Mr. Luna Robot";
    }
    
    String? companionName;
    bool isCompanionMsLuna = false;
    if (hasRelationship) {
      final companion = db.robots.firstWhere(
        (r) => r.id == db.primaryRobot!.companionDeviceId, 
        orElse: () => RobotProfile(id: '', name: 'Companion', variant: 'mr_luna', remoteId: '', lastConnected: DateTime.now())
      );
      companionName = companion.name;
      isCompanionMsLuna = companion.variant == 'ms_luna';
    }

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 13),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border(bottom: BorderSide(
          color: _accentColor.withOpacity(0.12),
          width: 1.2,
        )),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.03),
            blurRadius: 6,
            offset: const Offset(0, 2),
          ),
        ],
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
                  if (!hasRelationship) ...[
                    Row(
                      children: [
                        Text(
                          primaryName,
                          style: GoogleFonts.outfit(
                            color: _accentColor,
                            fontSize: 15,
                            fontWeight: FontWeight.w800,
                            letterSpacing: 0.2,
                          ),
                        ),
                        const SizedBox(width: 6),
                        _buildStatusDot(ble.isConnected),
                        if (ble.isConnected) ...[
                          const SizedBox(width: 6),
                          _buildVersionBadge(ble.hasSpeaker),
                        ],
                      ],
                    ),
                    const SizedBox(height: 2),
                    Text(
                      ble.isConnected ? "Connected" : "Disconnected",
                      style: GoogleFonts.outfit(color: textColor54, fontSize: 10, fontWeight: FontWeight.w500),
                    ),
                  ] else ...[
                    Row(
                      children: [
                        Text(
                          primaryName,
                          style: GoogleFonts.outfit(
                            color: _accentColor,
                            fontSize: 13,
                            fontWeight: FontWeight.w800,
                          ),
                        ),
                        const SizedBox(width: 4),
                        _buildStatusDot(ble.isConnected),
                        if (ble.isConnected) ...[
                          const SizedBox(width: 4),
                          _buildVersionBadge(ble.hasSpeaker),
                        ],
                        Padding(
                          padding: const EdgeInsets.symmetric(horizontal: 6),
                          child: Icon(
                            db.primaryRobot?.relationshipType == 'couple' ? Icons.favorite : Icons.link,
                            color: db.primaryRobot?.relationshipType == 'couple' ? Colors.pink : Colors.green,
                            size: 14,
                          ),
                        ),
                        Text(
                          companionName!,
                          style: GoogleFonts.outfit(
                            color: isCompanionMsLuna ? const Color(0xFFE91E8C) : const Color(0xFF0284C7),
                            fontSize: 13,
                            fontWeight: FontWeight.w800,
                          ),
                        ),
                        const SizedBox(width: 4),
                        _buildStatusDot(ble.isCompanionConnected),
                        if (ble.isCompanionConnected) ...[
                          const SizedBox(width: 4),
                          _buildVersionBadge(ble.companionHasSpeaker),
                        ],
                      ],
                    ),
                    const SizedBox(height: 2),
                    Text(
                      "Bond: ${db.primaryRobot?.relationshipType.toUpperCase() ?? 'NONE'}",
                      style: GoogleFonts.outfit(
                        color: textColor54,
                        fontSize: 9,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                  ],
                ],
              ),
            ],
          ),
          
          Row(
            children: [
              GestureDetector(
                onTap: () => _showTerminalLogsDialog(ble),
                child: Container(
                  padding: const EdgeInsets.all(8),
                  decoration: BoxDecoration(
                    color: const Color(0xFFFFF3E0),
                    shape: BoxShape.circle,
                    border: Border.all(
                      color: const Color(0xFFFF9800).withOpacity(0.35),
                    ),
                  ),
                  child: const Icon(
                    Icons.terminal,
                    color: Color(0xFFE65100),
                    size: 18,
                  ),
                ),
              ),
              const SizedBox(width: 8),
              GestureDetector(
                onTap: () async {
                  final db = Provider.of<DatabaseService>(context, listen: false);
                  if (ble.isConnected) {
                    ble.disconnect();
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(
                        content: Text("Disconnected from robots."),
                        duration: Duration(seconds: 2),
                      ),
                    );
                  } else {
                    if (ble.pairedDeviceId != null && ble.pairedDeviceId!.isNotEmpty) {
                      ScaffoldMessenger.of(context).showSnackBar(
                        SnackBar(
                          content: Text("Reconnecting to ${ble.pairedDeviceId}..."),
                          duration: const Duration(seconds: 2),
                        ),
                      );
                      await ble.connectById(ble.pairedDeviceId!);
                    } else {
                      _showBleScanner(db, ble);
                    }
                  }
                },
                child: Container(
                  padding: const EdgeInsets.all(8),
                  decoration: BoxDecoration(
                    color: ble.isConnected
                        ? _accentColor.withOpacity(0.10)
                        : const Color(0xFFF5F5F5),
                    shape: BoxShape.circle,
                    border: Border.all(
                      color: ble.isConnected
                          ? _accentColor.withOpacity(0.35)
                          : const Color(0xFF9E9E9E).withOpacity(0.3),
                    ),
                  ),
                  child: Icon(
                    ble.isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                    color: ble.isConnected ? _accentColor : const Color(0xFF9E9E9E),
                    size: 18,
                  ),
                ),
              ),
              const SizedBox(width: 8),
              GestureDetector(
                onTap: () => setState(() => _activeTabIdx = 6),
                child: CircleAvatar(
                  radius: 17,
                  backgroundImage: NetworkImage(
                    Provider.of<FirebaseService>(context).currentUser?['photoUrl'] ?? 
                    'https://api.dicebear.com/7.x/adventurer/png?seed=Luna'
                  ),
                  backgroundColor: _accentColor.withOpacity(0.1),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  void _showUserProfileDialog(BuildContext context) {
    final firebase = Provider.of<FirebaseService>(context, listen: false);
    final user = firebase.currentUser;
    if (user == null) return;

    final displayNameController = TextEditingController(text: user['displayName'] ?? '');
    final robotNameController = TextEditingController(text: user['robotName'] ?? '');
    String selectedVariant = user['robotVariant'] ?? 'ms_luna';

    showDialog(
      context: context,
      builder: (ctx) {
        return StatefulBuilder(
          builder: (ctx, setModalState) {
            final isPink = selectedVariant == 'ms_luna';
            final themeColor = isPink ? const Color(0xFFEC4899) : const Color(0xFF0074D9);

            return AlertDialog(
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
              contentPadding: const EdgeInsets.fromLTRB(20, 20, 20, 16),
              content: SingleChildScrollView(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Center(
                      child: Stack(
                        children: [
                          CircleAvatar(
                            radius: 36,
                            backgroundImage: NetworkImage(user['photoUrl'] ?? ''),
                            backgroundColor: themeColor.withOpacity(0.1),
                          ),
                          Positioned(
                            bottom: 0,
                            right: 0,
                            child: Container(
                              padding: const EdgeInsets.all(4),
                              decoration: BoxDecoration(
                                color: themeColor,
                                shape: BoxShape.circle,
                                border: Border.all(color: Colors.white, width: 2),
                              ),
                              child: const Icon(Icons.edit, size: 12, color: Colors.white),
                            ),
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(height: 16),
                    Text(
                      user['email'] ?? '',
                      textAlign: TextAlign.center,
                      style: GoogleFonts.outfit(color: textColor60, fontSize: 13),
                    ),
                    const SizedBox(height: 24),
                    TextField(
                      controller: displayNameController,
                      decoration: InputDecoration(
                        labelText: "My Display Name",
                        labelStyle: GoogleFonts.outfit(fontSize: 12),
                        prefixIcon: const Icon(Icons.person_outline, size: 18),
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                    ),
                    const SizedBox(height: 16),
                    TextField(
                      controller: robotNameController,
                      decoration: InputDecoration(
                        labelText: "Robot Name",
                        labelStyle: GoogleFonts.outfit(fontSize: 12),
                        prefixIcon: const Icon(Icons.android_outlined, size: 18),
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(12)),
                      ),
                    ),
                    const SizedBox(height: 16),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          "Robot Model:",
                          style: GoogleFonts.outfit(fontSize: 13, fontWeight: FontWeight.bold),
                        ),
                        Row(
                          children: [
                            ChoiceChip(
                              label: Text("Ms. Luna", style: GoogleFonts.outfit(fontSize: 11)),
                              selected: selectedVariant == 'ms_luna',
                              selectedColor: const Color(0xFFEC4899).withOpacity(0.2),
                              onSelected: (val) {
                                if (val) setModalState(() => selectedVariant = 'ms_luna');
                              },
                            ),
                            const SizedBox(width: 8),
                            ChoiceChip(
                              label: Text("Mr. Luna", style: GoogleFonts.outfit(fontSize: 11)),
                              selected: selectedVariant == 'mr_luna',
                              selectedColor: const Color(0xFF0074D9).withOpacity(0.2),
                              onSelected: (val) {
                                if (val) setModalState(() => selectedVariant = 'mr_luna');
                              },
                            ),
                          ],
                        ),
                      ],
                    ),
                    const SizedBox(height: 24),
                    Row(
                      children: [
                        Expanded(
                          child: OutlinedButton(
                            style: OutlinedButton.styleFrom(
                              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                              padding: const EdgeInsets.symmetric(vertical: 12),
                            ),
                            onPressed: () => Navigator.pop(ctx),
                            child: Text("CANCEL", style: GoogleFonts.outfit(fontWeight: FontWeight.bold, fontSize: 12)),
                          ),
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: ElevatedButton(
                            style: ElevatedButton.styleFrom(
                              backgroundColor: themeColor,
                              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                              padding: const EdgeInsets.symmetric(vertical: 12),
                            ),
                            onPressed: () async {
                              final dName = displayNameController.text.trim();
                              final rName = robotNameController.text.trim();
                              if (dName.isNotEmpty && rName.isNotEmpty) {
                                await firebase.updateUserProfile(dName, rName, selectedVariant);
                                if (context.mounted) {
                                  Navigator.pop(ctx);
                                  setState(() {});
                                }
                              } else {
                                ScaffoldMessenger.of(context).showSnackBar(
                                  const SnackBar(content: Text("Names cannot be empty")),
                                );
                              }
                            },
                            child: Text("SAVE", style: GoogleFonts.outfit(fontWeight: FontWeight.bold, fontSize: 12, color: Colors.white)),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            );
          },
        );
      },
    );
  }

  void _showTerminalLogsDialog(BLEService ble) {
    showDialog(
      context: context,
      builder: (context) {
        final ScrollController logScrollController = ScrollController();
        
        // Auto scroll logs to bottom
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (logScrollController.hasClients) {
            logScrollController.jumpTo(logScrollController.position.maxScrollExtent);
          }
        });

        return StatefulBuilder(
          builder: (context, setState) {
            return AlertDialog(
              backgroundColor: Colors.white,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
              titlePadding: const EdgeInsets.fromLTRB(16, 16, 16, 8),
              contentPadding: const EdgeInsets.fromLTRB(16, 8, 16, 16),
              title: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Row(
                    children: [
                      const Icon(Icons.terminal, color: Colors.black87, size: 20),
                      const SizedBox(width: 8),
                      Text(
                        "DEVICE CONSOLE LOGS",
                        style: GoogleFonts.outfit(
                          color: Colors.black87,
                          fontWeight: FontWeight.bold,
                          fontSize: 14,
                        ),
                      ),
                    ],
                  ),
                  Row(
                    children: [
                      TextButton(
                        onPressed: () {
                          ble.clearLogs();
                          setState(() {});
                        },
                        child: Text(
                          "CLEAR",
                          style: GoogleFonts.outfit(
                            color: Colors.redAccent,
                            fontSize: 11,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ),
                      IconButton(
                        onPressed: () => Navigator.pop(context),
                        icon: const Icon(Icons.close, color: Colors.black54, size: 18),
                        padding: EdgeInsets.zero,
                        constraints: const BoxConstraints(),
                      ),
                    ],
                  ),
                ],
              ),
              content: SizedBox(
                width: double.maxFinite,
                height: 300,
                child: Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    color: const Color(0xFFF8FAFC),
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(color: Colors.black.withOpacity(0.08)),
                  ),
                  child: StreamBuilder(
                    stream: Stream.periodic(const Duration(seconds: 1)),
                    builder: (context, snapshot) {
                      WidgetsBinding.instance.addPostFrameCallback((_) {
                        if (logScrollController.hasClients) {
                          logScrollController.jumpTo(logScrollController.position.maxScrollExtent);
                        }
                      });

                      return ListView.builder(
                        controller: logScrollController,
                        itemCount: ble.consoleLogs.length,
                        itemBuilder: (context, idx) {
                          final logLine = ble.consoleLogs[idx];
                          Color textColor = const Color(0xFF047857);
                          if (logLine.contains('[ERROR]')) textColor = const Color(0xFFDC2626);
                          if (logLine.contains('[BLE]')) textColor = const Color(0xFF1D4ED8);
                          if (logLine.contains('[SETTINGS]')) textColor = const Color(0xFFB45309);
                          if (logLine.contains('[CLOCK]')) textColor = const Color(0xFF7E22CE);
                          if (logLine.contains('[ROBOT]')) textColor = const Color(0xFFBE185D);

                          return Text(
                            logLine,
                            style: GoogleFonts.firaCode(color: textColor, fontSize: 11),
                          );
                        },
                      );
                    }
                  ),
                ),
              ),
            );
          }
        );
      },
    );
  }

  // Active Tab content router
  Widget _buildPanelContent(DatabaseService db, BLEService ble, String activeGifId, String activeLabel) {
    switch (_activeTabIdx) {
      case 0:
        return SingleChildScrollView(
          key: const PageStorageKey('home_scroll'),
          padding: const EdgeInsets.only(top: 10, bottom: 20),
          child: _buildHomeDashboardPanel(db, ble, activeGifId, activeLabel),
        );
      case 1:
        return Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            const SizedBox(height: 10),
            Text(
              "FACE EXPRESSIONS",
              style: GoogleFonts.outfit(
                color: textColor,
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 4),
            Text(
              "Preview pixel animations running on the robot.",
              style: GoogleFonts.outfit(
                color: textColor60,
                fontSize: 13,
              ),
            ),
            const SizedBox(height: 20),
            Center(
              child: OLEDSimulator(
                activeGifId: activeGifId,
                activeLabel: activeLabel,
                marqueeText: _marqueeController.text.isNotEmpty ? _marqueeController.text : null,
                wallpaperBytes: _selectedWallpaperBytes,
              ),
            ),
            const SizedBox(height: 20),
            Expanded(
              child: SingleChildScrollView(
                key: const PageStorageKey('expressions_scroll'),
                padding: const EdgeInsets.only(bottom: 20),
                child: _buildExpressionsPanel(db, ble),
              ),
            ),
          ],
        );
      case 2:
        return SingleChildScrollView(
          key: const PageStorageKey('sounds_scroll'),
          padding: const EdgeInsets.only(top: 10, bottom: 20),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "SOUNDS & EFFECTS",
                style: GoogleFonts.outfit(
                  color: textColor,
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                ),
              ),
              const SizedBox(height: 4),
              Text(
                "Trigger hardware sound effects and compose 8-bit melodies.",
                style: GoogleFonts.outfit(
                  color: textColor60,
                  fontSize: 13,
                ),
              ),
              const SizedBox(height: 20),
              // Show Intercom & Music Player only if speaker hardware is present (v2)
              if (ble.isConnected && ble.hasSpeaker) ...[
                _buildIntercomAndMusicSection(ble),
                const SizedBox(height: 20),
              ],
              _buildSoundBoardPanel(db, ble),
            ],
          ),
        );
      case 3:
        return SingleChildScrollView(
          key: const PageStorageKey('luna_link_scroll'),
          padding: const EdgeInsets.only(top: 10, bottom: 20),
          child: _buildLunaLinkPanel(db, ble),
        );
      case 4:
        return SingleChildScrollView(
          key: const PageStorageKey('calendar_scroll'),
          padding: const EdgeInsets.only(top: 10, bottom: 20),
          child: _buildCalendarPanel(db, ble),
        );
      case 5:
        return const BusinessCardScreen();
      case 6:
        return SingleChildScrollView(
          key: const PageStorageKey('profile_scroll'),
          padding: const EdgeInsets.only(top: 10, bottom: 20),
          child: _buildProfilePanel(db, ble),
        );
      default:
        return const SizedBox();
    }
  }

  Widget _buildExpressionsPanel(DatabaseService db, BLEService ble) {
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
        // GIF Library Header (filters/sorting)
        _buildLibraryControlsHeader(db),
        const SizedBox(height: 12),

        // GIF Library Grid list
        GridView.builder(
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
            maxCrossAxisExtent: 160,
            crossAxisSpacing: 8,
            mainAxisSpacing: 8,
            childAspectRatio: 0.95,
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
                  color: ble.isConnected ? const Color(0xFFE53935) : const Color(0xFF9E9E9E),
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
            onPressed: () async {
              if (ble.isConnected) {
                ble.disconnect();
              } else {
                if (ble.pairedDeviceId != null && ble.pairedDeviceId!.isNotEmpty) {
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(
                      content: Text("Reconnecting to ${ble.pairedDeviceId}..."),
                      duration: const Duration(seconds: 2),
                    ),
                  );
                  await ble.connectById(ble.pairedDeviceId!);
                } else {
                  _showBleScanner(db, ble);
                }
              }
            },
            icon: Icon(ble.isConnected ? Icons.close : Icons.link, size: 14),
            label: Text(ble.isConnected ? "DISCONNECT" : "CONNECT"),
            style: ElevatedButton.styleFrom(
              backgroundColor: ble.isConnected ? const Color(0xFF64748B) : _accentColor,
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
    final batteryPct = ble.batteryPercentage;

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
                borderRadius: BorderRadius.circular(6),
                child: LinearProgressIndicator(
                  value: ble.isConnected ? (batteryPct / 100.0) : 0.0,
                  minHeight: 7,
                  backgroundColor: const Color(0xFFEEEEEE),
                  valueColor: AlwaysStoppedAnimation<Color>(
                    batteryPct < 20
                        ? const Color(0xFFE53935)
                        : (batteryPct < 55
                            ? const Color(0xFFFFB300)
                            : const Color(0xFF43A047)),
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
              color: const Color(0xFFF8FAFC),
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: Colors.black.withOpacity(0.08)),
            ),
            child: ListView.builder(
              controller: logScrollController,
              itemCount: ble.consoleLogs.length,
              itemBuilder: (context, idx) {
                final logLine = ble.consoleLogs[idx];
                Color textColor = const Color(0xFF047857);
                if (logLine.contains('[ERROR]')) textColor = const Color(0xFFDC2626);
                if (logLine.contains('[BLE]')) textColor = const Color(0xFF1D4ED8);
                if (logLine.contains('[SETTINGS]')) textColor = const Color(0xFFB45309);
                if (logLine.contains('[CLOCK]')) textColor = const Color(0xFF7E22CE);
                if (logLine.contains('[ROBOT]')) textColor = const Color(0xFFBE185D);

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
                    const Icon(Icons.cloud_upload, color: Color(0xFFFFCDD2), size: 28),
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
                      backgroundColor: const Color(0xFFE53935),
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
                    style: GoogleFonts.outfit(color: const Color(0xFFFFCDD2), fontSize: 11, fontWeight: FontWeight.bold),
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

  Widget _buildGifCard(DatabaseService db, BLEService ble, GifModel gif) {
    return _GifCardWidget(
      gif: gif,
      db: db,
      ble: ble,
      onTap: () async {
        final mapping = DatabaseService.animMapping[gif.id] ??
            {'expr': 0, 'sound': 0, 'label': gif.name};
        final exprVal = mapping['expr'] as int;
        final soundVal = gif.soundId ?? (mapping['sound'] as int? ?? 0);

        setState(() {
          _localActiveGifId = gif.id;
          _localActiveLabel = gif.name;
        });

        ble.addLog("Executing expression: ${gif.name}", "ANIM");
        await ble.transmitExpression(exprVal, gif.name);

        if (soundVal > 0) {
          Future.delayed(const Duration(milliseconds: 150), () {
            ble.transmitAudio(soundVal);
          });
        }
      },
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
      {'id': 6, 'name': 'Surprise Warp', 'color': const Color(0xFFE53935)},
      {'id': 8, 'name': 'Castle Theme', 'color': const Color(0xFFE11D48)},
      {'id': 9, 'name': 'Underworld Theme', 'color': const Color(0xFF7C3AED)},
      {'id': 10, 'name': 'Theme Toggle SFX', 'color': const Color(0xFF0EA5E9)},
    ];

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
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
            final isEnabled = ble.isConnected;
            return Card(
              color: isEnabled ? const Color(0xFFE0F2FE) : const Color(0xFFF1F5F9),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(10),
                side: BorderSide(color: isEnabled ? _accentColor.withOpacity(0.3) : Colors.black.withOpacity(0.05), width: 1.2),
              ),
              child: InkWell(
                onTap: !isEnabled ? null : () async {
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
                        decoration: BoxDecoration(shape: BoxShape.circle, color: isEnabled ? color : textColor38),
                      ),
                      const SizedBox(width: 10),
                      Expanded(
                        child: Text(
                          sfx['name'] as String,
                          style: GoogleFonts.outfit(
                            color: isEnabled ? textColor : textColor38,
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
  Widget _buildChronosPanel(DatabaseService db, BLEService ble) {
    final now = DateTime.now();
    final is12H = db.is12HourFormat;

    String timeStr;
    if (is12H) {
      final dispHour = now.hour % 12 == 0 ? 12 : now.hour % 12;
      final ampm = now.hour >= 12 ? 'PM' : 'AM';
      final minStr = now.minute.toString().padLeft(2, '0');
      final secStr = now.second.toString().padLeft(2, '0');
      timeStr = "$dispHour:$minStr:$secStr $ampm";
    } else {
      timeStr = "${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}:${now.second.toString().padLeft(2, '0')}";
    }

    return Container(
      height: 340,
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
                  "CLOCK SETTINGS",
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
                  fontSize: 32,
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
                "Syncs local smartphone time to Mr.&Ms Luna's OLED display module clock.",
                textAlign: TextAlign.center,
                style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
              ),
            ),
            const SizedBox(height: 16),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text("24-Hour", style: GoogleFonts.outfit(color: textColor60, fontSize: 13, fontWeight: FontWeight.bold)),
                const SizedBox(width: 8),
                Switch(
                  value: is12H,
                  activeColor: const Color(0xFFE53935),
                  onChanged: (val) async {
                    await db.updateIs12HourFormat(val);
                    await ble.updateTimeFormat(val);
                  },
                ),
                const SizedBox(width: 8),
                Text("12-Hour", style: GoogleFonts.outfit(color: textColor60, fontSize: 13, fontWeight: FontWeight.bold)),
              ],
            ),
            const SizedBox(height: 16),
            ElevatedButton.icon(
              onPressed: () => ble.syncClockToHardware(),
              icon: const Icon(Icons.sync),
              label: const Text("SYNC TIME CLOCK"),
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFFE53935),
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

    final defaultGifOptions = ['cycle', 'default'] +
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
                style: GoogleFonts.outfit(color: const Color(0xFFFFCDD2), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
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

              // Default Screen Mode Dropdown
              _buildDefaultGifDropdown("Default Screen Mode", db.defaultGif, defaultGifOptions, (val) async {
                await db.updateDefaultGif(val);
                _syncSettingsToRobot(db, ble);
              }),
              const SizedBox(height: 16),
              // Intro Sound Speed
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Intro Sound Speed", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Text(ble.hasSpeaker ? "${db.introSoundSpeed.toInt()}%" : "N/A", style: GoogleFonts.firaCode(color: ble.hasSpeaker ? Colors.yellow : textColor38, fontSize: 12, fontWeight: FontWeight.bold)),
                ],
              ),
              Slider(
                value: db.introSoundSpeed,
                min: 20,
                max: 300,
                activeColor: const Color(0xFFE53935),
                onChanged: !ble.hasSpeaker ? null : (val) => db.updateIntroSoundSpeed(val),
                onChangeEnd: !ble.hasSpeaker ? null : (val) => _syncSettingsToRobot(db, ble),
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
                style: GoogleFonts.outfit(color: const Color(0xFFFFCDD2), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
              ),
              const SizedBox(height: 16),

              // Invert option
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Invert OLED Display (Negative)", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Switch(
                    value: db.oledInvert,
                    activeColor: const Color(0xFFE53935),
                    onChanged: (val) async {
                      await db.updateOledInvert(val);
                      await db.updateNegativeEnabled(val);
                      _syncSettingsToRobot(db, ble);
                      if (ble.isConnected && ble.hasSpeaker) {
                        await ble.transmitAudio(10);
                      }
                    },
                  ),
                ],
              ),
              const SizedBox(height: 12),
              // Clock Face Style
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Clock Face Style", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Container(
                    width: 140,
                    padding: const EdgeInsets.symmetric(horizontal: 10),
                    decoration: BoxDecoration(
                      color: Colors.white,
                      borderRadius: BorderRadius.circular(6),
                      border: Border.all(color: Colors.black.withOpacity(0.06)),
                    ),
                    child: DropdownButtonHideUnderline(
                      child: DropdownButton<int>(
                        value: db.clockStyle,
                        dropdownColor: Colors.white,
                        style: GoogleFonts.outfit(color: textColor, fontSize: 12),
                        icon: const Icon(Icons.arrow_drop_down, color: textColor60),
                        isExpanded: true,
                        items: const [
                          DropdownMenuItem(value: 0, child: Text("Classic Border")),
                          DropdownMenuItem(value: 1, child: Text("Minimalist")),
                          DropdownMenuItem(value: 2, child: Text("Analog Split")),
                          DropdownMenuItem(value: 3, child: Text("Custom Wallpaper")),
                        ],
                        onChanged: (val) async {
                          if (val != null) {
                            await db.updateClockStyle(val);
                            _syncSettingsToRobot(db, ble);
                          }
                        },
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              // OLED Brightness level
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Screen Brightness Level", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Container(
                    width: 140,
                    padding: const EdgeInsets.symmetric(horizontal: 10),
                    decoration: BoxDecoration(
                      color: Colors.white,
                      borderRadius: BorderRadius.circular(6),
                      border: Border.all(color: Colors.black.withOpacity(0.06)),
                    ),
                    child: DropdownButtonHideUnderline(
                      child: DropdownButton<int>(
                        value: db.oledBrightness.round(),
                        dropdownColor: Colors.white,
                        style: GoogleFonts.outfit(color: textColor, fontSize: 12),
                        icon: const Icon(Icons.arrow_drop_down, color: textColor60),
                        isExpanded: true,
                        items: const [
                          DropdownMenuItem(value: 1, child: Text("Dim / Low")),
                          DropdownMenuItem(value: 2, child: Text("Medium")),
                          DropdownMenuItem(value: 3, child: Text("Bright / High")),
                        ],
                        onChanged: (val) async {
                          if (val != null) {
                            await db.updateOledBrightness(val.toDouble());
                            _syncSettingsToRobot(db, ble);
                          }
                        },
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              // Buzzer Mode control
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text("Buzzer Mode", style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
                  Container(
                    width: 140,
                    padding: const EdgeInsets.symmetric(horizontal: 10),
                    decoration: BoxDecoration(
                      color: Colors.white,
                      borderRadius: BorderRadius.circular(6),
                      border: Border.all(color: Colors.black.withOpacity(0.06)),
                    ),
                    child: DropdownButtonHideUnderline(
                      child: DropdownButton<bool>(
                        value: db.silentMode,
                        dropdownColor: Colors.white,
                        style: GoogleFonts.outfit(color: textColor, fontSize: 12),
                        icon: const Icon(Icons.arrow_drop_down, color: textColor60),
                        isExpanded: true,
                        items: const [
                          DropdownMenuItem(value: false, child: Text("Sound On")),
                          DropdownMenuItem(value: true, child: Text("Silent Mode")),
                        ],
                        onChanged: (val) async {
                          if (val != null) {
                            await db.updateSilentMode(val);
                            _syncSettingsToRobot(db, ble);
                          }
                        },
                      ),
                    ),
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
                style: GoogleFonts.outfit(color: const Color(0xFFFFCDD2), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
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
        const SizedBox(height: 20),
        // Smartwatch Custom Wallpaper Manager
        GlassCard(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                "SMARTWATCH WALLPAPER",
                style: GoogleFonts.outfit(color: const Color(0xFFFFCDD2), fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1),
              ),
              const SizedBox(height: 16),
              if (_selectedWallpaperBytes != null) ...[
                Text(
                  "Selected: $_selectedWallpaperName",
                  style: GoogleFonts.outfit(color: textColor, fontSize: 13, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 8),
                Text(
                  "Size: ${(_selectedWallpaperBytes!.length / 1024).toStringAsFixed(1)} KB",
                  style: GoogleFonts.outfit(color: textColor60, fontSize: 11),
                ),
                const SizedBox(height: 16),
              ],
              if (_isUploadingWallpaper) ...[
                ClipRRect(
                  borderRadius: BorderRadius.circular(4),
                  child: LinearProgressIndicator(
                    value: _wallpaperUploadProgress,
                    backgroundColor: Colors.black12,
                    valueColor: const AlwaysStoppedAnimation<Color>(Colors.green),
                    minHeight: 8,
                  ),
                ),
                const SizedBox(height: 8),
                Text(
                  "Uploading: ${(_wallpaperUploadProgress * 100).toInt()}%",
                  textAlign: TextAlign.center,
                  style: GoogleFonts.firaCode(color: Colors.yellow, fontSize: 11, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 16),
              ] else ...[
                Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: () => _pickAndProcessWallpaper(db),
                        icon: const Icon(Icons.photo_library),
                        label: const Text("SELECT IMAGE"),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.blueAccent,
                          foregroundColor: Colors.white,
                          padding: const EdgeInsets.symmetric(vertical: 12),
                          textStyle: GoogleFonts.outfit(fontSize: 10, fontWeight: FontWeight.bold),
                        ),
                      ),
                    ),
                    if (_selectedWallpaperBytes != null) ...[
                      const SizedBox(width: 8),
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: !ble.isConnected ? null : () => _uploadWallpaper(ble),
                          icon: const Icon(Icons.bluetooth),
                          label: const Text("BLE UPLOAD"),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.green,
                            foregroundColor: Colors.white,
                            padding: const EdgeInsets.symmetric(vertical: 12),
                            textStyle: GoogleFonts.outfit(fontSize: 10, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ),
                      const SizedBox(width: 8),
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: () => _compileAndFlashViaServer(ble),
                          icon: const Icon(Icons.computer),
                          label: const Text("PC FLASH"),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.indigoAccent,
                            foregroundColor: Colors.white,
                            padding: const EdgeInsets.symmetric(vertical: 12),
                            textStyle: GoogleFonts.outfit(fontSize: 10, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ),
                    ],
                  ],
                ),
                const SizedBox(height: 12),
                Row(
                  children: [
                    Expanded(
                      child: OutlinedButton.icon(
                        onPressed: !ble.isConnected ? null : () async {
                          final cleared = await ble.transmitWallpaperClear();
                          if (cleared) {
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(
                                content: Text("Wallpaper cleared from watch!"),
                                backgroundColor: Colors.green,
                              ),
                            );
                          } else {
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(
                                content: Text("Failed to clear wallpaper."),
                                backgroundColor: Colors.red,
                              ),
                            );
                          }
                        },
                        icon: const Icon(Icons.delete_forever),
                        label: const Text("CLEAR WATCH WALLPAPER"),
                        style: OutlinedButton.styleFrom(
                          foregroundColor: Colors.redAccent,
                          side: const BorderSide(color: Colors.redAccent),
                          padding: const EdgeInsets.symmetric(vertical: 12),
                          textStyle: GoogleFonts.outfit(fontSize: 12, fontWeight: FontWeight.bold),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
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

  Widget _buildDefaultGifDropdown(String label, String value, List<String> options, Function(String) onChanged) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text(label, style: GoogleFonts.outfit(color: textColor60, fontSize: 12)),
        Container(
          width: 185,
          padding: const EdgeInsets.symmetric(horizontal: 10),
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.circular(6),
            border: Border.all(color: Colors.black.withOpacity(0.06)),
          ),
          child: DropdownButtonHideUnderline(
            child: DropdownButton<String>(
              value: options.contains(value) ? value : 'cycle',
              dropdownColor: Colors.white,
              style: GoogleFonts.outfit(color: textColor, fontSize: 12),
              isExpanded: true,
              items: options.map((opt) {
                String displayName = opt;
                if (opt == 'cycle') displayName = "Random Cycle Mode";
                if (opt == 'default') displayName = "Default Idle Face";

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

  Future<ui.Image> resizeImage(Uint8List imageBytes, int targetWidth, int targetHeight) async {
    final codec = await ui.instantiateImageCodec(imageBytes);
    final frameInfo = await codec.getNextFrame();
    final originalImage = frameInfo.image;
    
    final recorder = ui.PictureRecorder();
    final canvas = ui.Canvas(recorder);
    final paint = ui.Paint()..filterQuality = ui.FilterQuality.high;
    
    canvas.drawImageRect(
      originalImage,
      ui.Rect.fromLTWH(0, 0, originalImage.width.toDouble(), originalImage.height.toDouble()),
      ui.Rect.fromLTWH(0, 0, targetWidth.toDouble(), targetHeight.toDouble()),
      paint,
    );
    
    final picture = recorder.endRecording();
    return await picture.toImage(targetWidth, targetHeight);
  }

  Future<Uint8List> convertImageToRGB565(ui.Image image) async {
    final byteData = await image.toByteData(format: ui.ImageByteFormat.rawRgba);
    if (byteData == null) throw Exception("Failed to get raw RGBA bytes");
    
    final width = image.width;
    final height = image.height;
    final rgbaBytes = byteData.buffer.asUint8List();
    
    final rgb565Bytes = Uint8List(width * height * 2);
    final rgb565Data = ByteData.view(rgb565Bytes.buffer);
    
    int srcIdx = 0;
    int dstIdx = 0;
    for (int i = 0; i < width * height; i++) {
      final r = rgbaBytes[srcIdx];
      final g = rgbaBytes[srcIdx + 1];
      final b = rgbaBytes[srcIdx + 2];
      
      final r5 = (r >> 3) & 0x1F;
      final g6 = (g >> 2) & 0x3F;
      final b5 = (b >> 3) & 0x1F;
      
      final color16 = (r5 << 11) | (g6 << 5) | b5;
      rgb565Data.setUint16(dstIdx, color16, Endian.little);
      
      srcIdx += 4;
      dstIdx += 2;
    }
    return rgb565Bytes;
  }

  Future<void> _pickAndProcessWallpaper(DatabaseService db) async {
    try {
      final result = await FilePicker.platform.pickFiles(
        type: FileType.image,
        allowMultiple: false,
      );
      if (result == null || result.files.isEmpty) return;
      
      final file = result.files.first;
      final bytes = file.bytes ?? (file.path != null ? await File(file.path!).readAsBytes() : null);
      if (bytes == null) return;
      
      final robot = db.primaryRobot;
      final int targetW = (robot?.firmwareVersion == 2) ? 240 : 128;
      final int targetH = (robot?.firmwareVersion == 2) ? 240 : 160;
      
      // Launch custom cropping dialog
      final croppedBytes = await showDialog<Uint8List>(
        context: context,
        barrierDismissible: false,
        builder: (context) => WallpaperCropDialog(
          imageBytes: bytes,
          targetW: targetW,
          targetH: targetH,
        ),
      );
      if (croppedBytes == null) return; // User cancelled
      
      setState(() {
        _selectedWallpaperBytes = croppedBytes;
        _selectedWallpaperName = file.name;
      });
      
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Wallpaper cropped & loaded: ${file.name} ($targetW x $targetH)"),
          backgroundColor: Colors.green,
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Error processing image: $e"),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  Future<void> _compileAndFlashViaServer(BLEService ble) async {
    if (_selectedWallpaperBytes == null) return;

    setState(() {
      _isUploadingWallpaper = true;
      _wallpaperUploadProgress = 0.0;
    });

    try {
      final robot = Provider.of<DatabaseService>(context, listen: false).primaryRobot;
      final int targetW = (robot?.firmwareVersion == 2) ? 240 : 128;
      final int targetH = (robot?.firmwareVersion == 2) ? 240 : 160;

      // 1. Convert bytes to C++ header array string
      final buffer = StringBuffer();
      buffer.writeln("// Luna Smartwatch Custom Wallpaper - ${targetW}x${targetH} RGB565");
      buffer.writeln("// Auto-generated by Luna Companion App.");
      buffer.writeln("#ifndef WALLPAPER_IMAGE_H");
      buffer.writeln("#define WALLPAPER_IMAGE_H");
      buffer.writeln("#include <pgmspace.h>");
      buffer.writeln("#define WALLPAPER_WIDTH  $targetW");
      buffer.writeln("#define WALLPAPER_HEIGHT $targetH");
      buffer.writeln("#define WALLPAPER_IS_DEFAULT 0");
      buffer.writeln("const uint16_t wallpaper_data[${targetW * targetH}] PROGMEM = {");

      for (int i = 0; i < _selectedWallpaperBytes!.length; i += 2) {
        final b1 = _selectedWallpaperBytes![i];
        final b2 = _selectedWallpaperBytes![i + 1];
        final val = (b2 << 8) | b1;
        buffer.write("0x${val.toRadixString(16).padLeft(4, '0')},");
        if ((i ~/ 2 + 1) % 12 == 0) {
          buffer.writeln();
        }
      }
      buffer.writeln("\n};");
      buffer.writeln("#endif");

      final codeString = buffer.toString();

      // 2. POST to compilation server
      final serverUrl = Uri.parse('http://${ble.serverIp}:8000/api/upload_wallpaper');
      final response = await http.post(
        serverUrl,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'variant': robot?.firmwareVersion == 2 ? '1.3' : '1.8',
          'code': codeString,
        }),
      ).timeout(const Duration(seconds: 15));

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        if (data['success'] == true) {
          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(
              content: Text("Wallpaper C++ code sent to server on port ${data['port']}! Compilation & flashing started."),
              backgroundColor: Colors.green,
            ),
          );
        } else {
          throw Exception(data['error'] ?? "Unknown server error");
        }
      } else {
        throw Exception("Server returned HTTP ${response.statusCode}");
      }
    } catch (e) {
      _showErrorDialog(
        "Connection Failed",
        "Could not send wallpaper to local server at http://${ble.serverIp}:8000.\n\n"
        "Make sure 'python flash_server.py' is running on your computer.\n\n"
        "Error details: $e"
      );
    } finally {
      setState(() {
        _isUploadingWallpaper = false;
      });
    }
  }

  Future<void> _uploadWallpaper(BLEService ble) async {
    if (_selectedWallpaperBytes == null) return;
    
    setState(() {
      _isUploadingWallpaper = true;
      _wallpaperUploadProgress = 0.0;
    });
    
    try {
      final size = _selectedWallpaperBytes!.length;
      
      final started = await ble.transmitWallpaperStart(size);
      if (!started) {
        throw Exception("Failed to start wallpaper transmission on the watch.");
      }
      
      final hexString = _selectedWallpaperBytes!.map((b) => b.toRadixString(16).padLeft(2, '0')).join();
      
      // 60 hex chars = 30 raw bytes. BLE packet = "WP_CHUNK:" + 60 = 69 bytes.
      // Fits guaranteed under any Android BLE MTU (min ~50 bytes after negotiation).
      const chunkSize = 60;
      final totalChunks = (hexString.length / chunkSize).ceil();
      
      for (int i = 0; i < totalChunks; i++) {
        final start = i * chunkSize;
        final end = (start + chunkSize < hexString.length) ? start + chunkSize : hexString.length;
        final chunk = hexString.substring(start, end);
        
        final chunkSent = await ble.transmitWallpaperChunk(chunk);
        if (!chunkSent) {
          throw Exception("Failed to transmit wallpaper chunk ${i + 1}/$totalChunks.");
        }
        
        setState(() {
          _wallpaperUploadProgress = (i + 1) / totalChunks;
        });
        
        // Give the ESP32 SPIFFS time to flush before next chunk
        await Future.delayed(const Duration(milliseconds: 80));
      }
      
      final ended = await ble.transmitWallpaperEnd();
      if (!ended) {
        throw Exception("Failed to finalize wallpaper transmission on the watch.");
      }
      
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("Wallpaper uploaded successfully!"),
          backgroundColor: Colors.green,
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Upload failed: $e"),
          backgroundColor: Colors.red,
        ),
      );
    } finally {
      setState(() {
        _isUploadingWallpaper = false;
      });
    }
  }



  // ================= NEW TAB 0: HOME DASHBOARD PANEL =================
  Widget _buildHomeDashboardPanel(DatabaseService db, BLEService ble, String activeGifId, String activeLabel) {
    final primary = db.primaryRobot ?? RobotProfile(
      id: 'mr_luna',
      name: 'Mr. Luna',
      variant: 'mr_luna',
      remoteId: '',
      lastConnected: DateTime.now(),
    );
    
    final isMiss = primary.variant == 'ms_luna';
    final accentColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFFE53935);
    final personality = isMiss ? "Softer & Calmer Personality" : "Friendly & Energetic Personality";
    
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
                    colors: isMiss ? [const Color(0xFFEC4899), const Color(0xFFF472B6)] : [const Color(0xFF0284C7), const Color(0xFF38BDF8)],
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
                  isMiss ? "MS. LUNA" : "MR. LUNA",
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
            invertColor: db.oledInvert, 
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
                _activeTabIdx = 5; // Profile/Settings tab
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

        _buildSendMessageSection(ble),
        const SizedBox(height: 20),

        _buildMapStreamingSection(ble),
        const SizedBox(height: 20),

        // Quick Actions panel
        _buildQuickActionsPanel(ble),
      ],
    );
  }

  Widget _buildMapStreamingSection(BLEService ble) {
    const mapBlue = Color(0xFF4285F4); // Google Maps Blue

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSectionHeader("Google Maps Live Streaming", Icons.map_outlined, mapBlue),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // Screen Map Canvas Preview Box (1.69" 240x280 display ratio)
              Container(
                height: 180,
                decoration: BoxDecoration(
                  color: Colors.black,
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(color: mapBlue.withOpacity(0.5), width: 1.5),
                ),
                child: Stack(
                  children: [
                    // Simulated Map background grid / navigation route lines
                    ClipRRect(
                      borderRadius: BorderRadius.circular(10),
                      child: CustomPaint(
                        size: const Size(double.infinity, 180),
                        painter: MapPreviewPainter(isStreaming: _isStreamingMap),
                      ),
                    ),
                    Positioned(
                      top: 10,
                      left: 10,
                      child: Container(
                        padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                        decoration: BoxDecoration(
                          color: Colors.black87,
                          borderRadius: BorderRadius.circular(6),
                          border: Border.all(color: mapBlue.withOpacity(0.5)),
                        ),
                        child: Row(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            const Icon(Icons.navigation, color: mapBlue, size: 12),
                            const SizedBox(width: 4),
                            Text(
                              _isStreamingMap ? "LIVE BLE MAP ACTIVE" : "ST7789 (240x280)",
                              style: GoogleFonts.outfit(
                                color: Colors.white,
                                fontSize: 9,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                          ],
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 14),
              Row(
                children: [
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: () async {
                        setState(() {
                          _isStreamingMap = !_isStreamingMap;
                        });
                        if (_isStreamingMap) {
                          await ble.transmitMapClear();
                          // Stream demo map packets (240x240 RGB565 chunks)
                          for (int line = 0; line < 240; line += 20) {
                            final sampleBase64 = base64Encode(List<int>.filled(480, (line % 255)));
                            await ble.transmitMapLine(line, sampleBase64);
                          }
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(
                              content: Text("Started Live Map Stream to Luna Watch!"),
                              backgroundColor: Color(0xFF4285F4),
                            ),
                          );
                        } else {
                          await ble.transmitMapClear();
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(
                              content: Text("Map stream stopped."),
                            ),
                          );
                        }
                      },
                      icon: Icon(_isStreamingMap ? Icons.stop : Icons.play_arrow, size: 18),
                      label: Text(_isStreamingMap ? "STOP MAP STREAM" : "START MAP STREAM"),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: _isStreamingMap ? Colors.redAccent : mapBlue,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 12),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
                      ),
                    ),
                  ),
                  const SizedBox(width: 10),
                  OutlinedButton.icon(
                    onPressed: () async {
                      await ble.transmitMapClear();
                      setState(() {
                        _isStreamingMap = false;
                      });
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(content: Text("Cleared Watch Screen")),
                      );
                    },
                    icon: const Icon(Icons.clear_all, size: 18),
                    label: const Text("CLEAR"),
                    style: OutlinedButton.styleFrom(
                      foregroundColor: textColor,
                      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
                      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildSendMessageSection(BLEService ble) {
    final isMiss = _isMsLuna;
    final accentColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFFE53935);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSectionHeader("Send Message to Robot", Icons.chat_bubble_outline, accentColor),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Row(
                children: [
                  Expanded(
                    child: TextField(
                      controller: _homeMessageController,
                      style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                      decoration: InputDecoration(
                        hintText: "Type a scrolling message...",
                        hintStyle: GoogleFonts.outfit(color: textColor38),
                        filled: true,
                        fillColor: Colors.white.withOpacity(0.03),
                        contentPadding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                        ),
                        enabledBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: BorderSide(color: Colors.white.withOpacity(0.07)),
                        ),
                        focusedBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: BorderSide(color: accentColor),
                        ),
                      ),
                    ),
                  ),
                  const SizedBox(width: 10),
                  ElevatedButton(
                    onPressed: () async {
                      final text = _homeMessageController.text.trim();
                      if (text.isNotEmpty) {
                        _marqueeController.text = text; // sync with simulator
                        setState(() {}); // Updates OLED preview
                        await ble.transmitMarqueeText(text);
                        _homeMessageController.clear();
                        ScaffoldMessenger.of(context).showSnackBar(
                          SnackBar(
                            content: Text("Sent: '$text'"),
                            backgroundColor: const Color(0xFF10B981),
                          ),
                        );
                      }
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: accentColor,
                      foregroundColor: Colors.white,
                      padding: EdgeInsets.zero,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(10),
                      ),
                      elevation: 2,
                    ),
                    child: Container(
                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
                      decoration: BoxDecoration(
                        borderRadius: BorderRadius.circular(10),
                      ),
                      child: ClipRRect(
                        borderRadius: BorderRadius.circular(10),
                        child: Stack(
                          alignment: Alignment.center,
                          children: [
                            Positioned.fill(
                              child: CustomPaint(
                                painter: ButtonStripePainter(
                                  color: Colors.white.withOpacity(0.15),
                                  stripeWidth: 3,
                                  gapWidth: 6,
                                ),
                              ),
                            ),
                            const Icon(Icons.send, size: 18),
                          ],
                        ),
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              SingleChildScrollView(
                scrollDirection: Axis.horizontal,
                child: Row(
                  children: [
                    "Hello!",
                    "I Love You!",
                    "Good Morning!",
                    "Battery Low!",
                    "Meeting Started!",
                  ].map((preset) {
                    return Padding(
                      padding: const EdgeInsets.only(right: 8.0),
                      child: ActionChip(
                        label: Text(
                          preset,
                          style: GoogleFonts.outfit(color: textColor70, fontSize: 11),
                        ),
                        backgroundColor: Colors.white.withOpacity(0.05),
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(8),
                          side: BorderSide(color: Colors.white.withOpacity(0.07)),
                        ),
                        onPressed: () async {
                          _marqueeController.text = preset;
                          setState(() {});
                          await ble.transmitMarqueeText(preset);
                          ScaffoldMessenger.of(context).showSnackBar(
                            SnackBar(
                              content: Text("Sent Preset: '$preset'"),
                              backgroundColor: const Color(0xFF10B981),
                            ),
                          );
                        },
                      ),
                    );
                  }).toList(),
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildIntercomAndMusicSection(BLEService ble) {
    final audioStream = Provider.of<AudioStreamService>(context);
    final isMiss = _isMsLuna;
    final accentColor = isMiss ? const Color(0xFFEC4899) : const Color(0xFFE53935);
    final hasSpeaker = ble.hasSpeaker;
    
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSectionHeader("Intercom & MP3 Music Player", Icons.settings_voice, accentColor),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              if (audioStream.statusMessage != null) ...[
                Container(
                  padding: const EdgeInsets.all(10),
                  decoration: BoxDecoration(
                    color: Colors.white.withOpacity(0.05),
                    borderRadius: BorderRadius.circular(8),
                    border: Border.all(color: Colors.white.withOpacity(0.08)),
                  ),
                  child: Row(
                    children: [
                      Icon(
                        audioStream.isCalling 
                            ? Icons.phone_in_talk 
                            : audioStream.isStreamingMusic 
                                ? Icons.music_note 
                                : Icons.info_outline,
                        color: accentColor,
                        size: 16,
                      ),
                      const SizedBox(width: 8),
                      Expanded(
                        child: Text(
                          audioStream.statusMessage!,
                          style: GoogleFonts.outfit(color: textColor, fontSize: 13, fontWeight: FontWeight.w500),
                        ),
                      ),
                    ],
                  ),
                ),
                const SizedBox(height: 16),
              ],
              
              Row(
                children: [
                  // VoIP Call Button
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: !hasSpeaker ? null : () async {
                        if (audioStream.isCalling) {
                          await audioStream.stopCall();
                          await ble.transmitStopCall();
                        } else {
                          final serverIp = ble.serverIp;
                          final cleanMac = ble.pairedDeviceId!.replaceAll(':', '').toUpperCase();
                          
                          await ble.transmitStartCall();
                          final success = await audioStream.startCall(serverIp, cleanMac, ble: ble);
                          if (success) {
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(content: Text(serverIp == 'localhost' || serverIp.isEmpty ? "BLE Call Connected!" : "VoIP Call Connected!")),
                            );
                          }
                        }
                      },
                      icon: Icon(audioStream.isCalling ? Icons.call_end : Icons.call, size: 18),
                      label: Text(audioStream.isCalling ? "End Call" : "Voice Call"),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: !hasSpeaker ? Colors.grey : (audioStream.isCalling ? Colors.red.shade700 : const Color(0xFF10B981)),
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                      ),
                    ),
                  ),
                  const SizedBox(width: 12),
                  // Local Library Button
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: !hasSpeaker ? null : () {
                        _showLocalAudioFilesDialog(ble, audioStream);
                      },
                      icon: const Icon(Icons.library_music, size: 18),
                      label: const Text("Local Library"),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: !hasSpeaker ? Colors.grey : const Color(0xFF3B82F6),
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 12),
              SizedBox(
                width: double.infinity,
                child: ElevatedButton.icon(
                  onPressed: !hasSpeaker ? null : () async {
                    if (audioStream.isLoopbackActive) {
                      await audioStream.stopLoopback(ble);
                    } else {
                      await audioStream.startLoopback(ble);
                    }
                  },
                  icon: Icon(audioStream.isLoopbackActive ? Icons.stop : Icons.loop, size: 18),
                  label: Text(audioStream.isLoopbackActive ? "Stop Loopback Test" : "Hardware Loopback Test"),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: !hasSpeaker ? Colors.grey : (audioStream.isLoopbackActive ? Colors.red.shade700 : const Color(0xFF8B5CF6)),
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
              ),
              if (audioStream.isStreamingMusic) ...[
                const SizedBox(height: 16),
                Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    color: Colors.white.withOpacity(0.03),
                    borderRadius: BorderRadius.circular(10),
                    border: Border.all(color: Colors.white.withOpacity(0.05)),
                  ),
                  child: Column(
                    children: [
                      Text(
                        _currentlyPlayingFile?['name'] ?? "Streaming audio...",
                        style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 13),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      ),
                      const SizedBox(height: 8),
                      Row(
                        children: [
                          Text(
                            _formatPcmDuration(audioStream.musicStreamOffset),
                            style: GoogleFonts.firaCode(color: textColor60, fontSize: 11),
                          ),
                          Expanded(
                            child: Slider(
                              value: audioStream.musicStreamOffset.toDouble(),
                              min: 0,
                              max: audioStream.musicStreamTotalSize.toDouble() > 0 
                                  ? audioStream.musicStreamTotalSize.toDouble() 
                                  : 1.0,
                              activeColor: accentColor,
                              inactiveColor: textColor24,
                              onChanged: (val) {
                                audioStream.seekMusic(val.toInt());
                              },
                            ),
                          ),
                          Text(
                            _formatPcmDuration(audioStream.musicStreamTotalSize),
                            style: GoogleFonts.firaCode(color: textColor60, fontSize: 11),
                          ),
                        ],
                      ),
                      const SizedBox(height: 8),
                      // ── Playback controls ─────────────────────────────────────────
                      Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          IconButton(
                            icon: Icon(
                              audioStream.isMusicPaused ? Icons.play_arrow : Icons.pause,
                              color: textColor,
                              size: 28,
                            ),
                            onPressed: () {
                              if (audioStream.isMusicPaused) {
                                audioStream.resumeMusic();
                              } else {
                                audioStream.pauseMusic();
                              }
                            },
                          ),
                          const SizedBox(width: 20),
                          IconButton(
                            icon: const Icon(
                              Icons.stop,
                              color: Colors.red,
                              size: 28,
                            ),
                            onPressed: () async {
                              await _stopMusic(audioStream, ble);
                            },
                          ),
                        ],
                      ),
                      const SizedBox(height: 6),
                      const Divider(color: Colors.black12, height: 1),
                      const SizedBox(height: 10),
                      // ── Volume slider ──────────────────────────────────────────
                      Row(
                        children: [
                          Icon(Icons.volume_down, color: textColor60, size: 18),
                          Expanded(
                            child: SliderTheme(
                              data: SliderTheme.of(context).copyWith(
                                trackHeight: 4.0,
                                thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 8),
                                overlayShape: const RoundSliderOverlayShape(overlayRadius: 16),
                                activeTrackColor: hasSpeaker ? accentColor : Colors.grey,
                                inactiveTrackColor: textColor24,
                                thumbColor: hasSpeaker ? accentColor : Colors.grey,
                                overlayColor: hasSpeaker ? accentColor.withOpacity(0.18) : Colors.transparent,
                              ),
                              child: Slider(
                                value: audioStream.volume.toDouble(),
                                min: 0,
                                max: 100,
                                divisions: 20,
                                label: "Vol ${audioStream.volume}%",
                                onChanged: !hasSpeaker ? null : (val) {
                                  audioStream.setVolume(val.toInt());
                                },
                                // Send BLE command only when user releases finger
                                onChangeEnd: !hasSpeaker ? null : (val) {
                                  ble.transmitVolume(val.toInt());
                                },
                              ),
                            ),
                          ),
                          Icon(Icons.volume_up, color: hasSpeaker ? accentColor : Colors.grey, size: 18),
                          const SizedBox(width: 4),
                          Text(
                            "${audioStream.volume}%",
                            style: GoogleFonts.firaCode(color: textColor60, fontSize: 11),
                          ),
                        ],
                      ),
                      // ── Bass slider ────────────────────────────────────────────
                      Row(
                        children: [
                          Icon(Icons.graphic_eq, color: textColor60, size: 18),
                          Expanded(
                            child: SliderTheme(
                              data: SliderTheme.of(context).copyWith(
                                trackHeight: 4.0,
                                thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 8),
                                overlayShape: const RoundSliderOverlayShape(overlayRadius: 16),
                                activeTrackColor: hasSpeaker ? const Color(0xFF7C3AED) : Colors.grey,
                                inactiveTrackColor: textColor24,
                                thumbColor: hasSpeaker ? const Color(0xFF7C3AED) : Colors.grey,
                                overlayColor: hasSpeaker ? const Color(0x287C3AED) : Colors.transparent,
                              ),
                              child: Slider(
                                value: audioStream.bass.toDouble(),
                                min: 0,
                                max: 10,
                                divisions: 10,
                                label: "Bass ${audioStream.bass}",
                                onChanged: !hasSpeaker ? null : (val) {
                                  audioStream.setBass(val.toInt());
                                },
                                // Send BLE command only when user releases finger
                                onChangeEnd: !hasSpeaker ? null : (val) {
                                  ble.transmitBass(val.toInt());
                                },
                              ),
                            ),
                          ),
                          const Icon(Icons.speaker, color: Color(0xFF7C3AED), size: 18),
                          const SizedBox(width: 4),
                          Text(
                            "Bass ${audioStream.bass}",
                            style: GoogleFonts.firaCode(color: textColor60, fontSize: 11),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),
              ],
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildRobotStatusGrid(RobotProfile robot, BLEService ble) {
    final batteryPct = ble.batteryPercentage;

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
                    color: ble.isConnected ? const Color(0xFFE53935) : Colors.white38,
                    size: 18,
                  ),
                  Text(
                    ble.isConnected ? "Active" : "Offline",
                    style: GoogleFonts.outfit(
                      color: ble.isConnected ? const Color(0xFFE53935) : Colors.white38,
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
    final heartEmoji = isCouple ? "\u{2764}\u{FE0F}" : "\u{1F91D}";
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
        'icon': Icons.watch_later,
        'label': 'Show Clock',
        'color': const Color(0xFFE53935),
        'onTap': () => ble.transmitExpression(8, ""), // 8 is EXPR_CLOCK
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
        'icon': Icons.message,
        'label': 'Send Msg',
        'color': const Color(0xFF3B82F6),
        'onTap': () => _showSendMessageDialog(ble),
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
                        "Mr. Luna: Hello! How is your day going? Let's write some code together!",
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
      {'label': 'Birthday Reminder \u{1F382}', 'text': 'HAPPY BIRTHDAY!'},
      {'label': 'Meeting Reminder \u{1F4C5}', 'text': 'MEETING IN 5 MINS'},
      {'label': 'Task Reminder \u{2705}', 'text': 'DRINK WATER / STAND UP'},
      {'label': 'Weather Alert \u{26C8}\u{FE0F}', 'text': 'HEAVY RAIN EXPECTED'},
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
                trailing: const Icon(Icons.send, color: Color(0xFFE53935), size: 16),
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
              final isMiss = robot.variant == 'ms_luna';
              final accent = isMiss ? const Color(0xFFEC4899) : const Color(0xFFE53935);

              // Dynamically check version if currently connected, else fallback to database
              bool isV2 = robot.firmwareVersion == 2;
              if (ble.isConnected && ble.connectedDevice?.remoteId.str == robot.remoteId) {
                isV2 = ble.hasSpeaker;
              } else if (ble.isCompanionConnected && ble.companionDevice?.remoteId.str == robot.remoteId) {
                isV2 = ble.companionHasSpeaker;
              }
              final versionStr = isV2 ? "Version 2 (Luna v2)" : "Version 1 (Luna v1)";

              return Card(
                color: Colors.white,
                elevation: 0.5,
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                  side: BorderSide(color: isPrimary ? accent : Colors.black.withOpacity(0.08)),
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
                        style: GoogleFonts.outfit(color: Colors.black87, fontWeight: FontWeight.bold, fontSize: 14),
                      ),
                      if (isPrimary) ...[
                        const SizedBox(width: 8),
                        Container(
                          padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                          decoration: BoxDecoration(
                            color: const Color(0xFFD1FAE5),
                            borderRadius: BorderRadius.circular(10),
                          ),
                          child: Text(
                            "PRIMARY",
                            style: GoogleFonts.outfit(color: const Color(0xFF065F46), fontSize: 8, fontWeight: FontWeight.bold),
                          ),
                        ),
                      ],
                    ],
                  ),
                  subtitle: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const SizedBox(height: 4),
                      Text(
                        isMiss ? "Ms. Luna variant" : "Mr. Luna variant",
                        style: GoogleFonts.outfit(color: Colors.black54, fontSize: 11),
                      ),
                      const SizedBox(height: 2),
                      Text(
                        "Bluetooth ID: ${robot.remoteId}",
                        style: GoogleFonts.outfit(color: Colors.black54, fontSize: 11),
                      ),
                      const SizedBox(height: 2),
                      Text(
                        versionStr,
                        style: GoogleFonts.outfit(
                          color: isV2 ? const Color(0xFF047857) : const Color(0xFFB45309),
                          fontSize: 11,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                  trailing: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      IconButton(
                        icon: const Icon(Icons.edit, color: Colors.black54, size: 16),
                        onPressed: () => _showEditRobotDialog(db, robot),
                      ),
                      if (!isPrimary)
                        IconButton(
                          icon: const Icon(Icons.star_border, color: Colors.black54, size: 16),
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
            backgroundColor: const Color(0xFFE53935),
            foregroundColor: Colors.white,
            padding: const EdgeInsets.symmetric(vertical: 14),
          ),
        ),

        const SizedBox(height: 24),


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
                "Manage relationship status (Friends \u{1F91D} vs Couple \u{2764}\u{FE0F}) between Mr. Luna and Ms. Luna companions.",
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
                          label: const Text("Mr. Luna"),
                          selected: selectedVariant == 'mr_luna',
                          onSelected: (val) {
                            if (val) setModalState(() => selectedVariant = 'mr_luna');
                          },
                          selectedColor: _accentColor.withOpacity(0.2),
                          checkmarkColor: _accentColor,
                        ),
                      ),
                      const SizedBox(width: 8),
                      Expanded(
                        child: ChoiceChip(
                          label: const Text("Ms. Luna"),
                          selected: selectedVariant == 'ms_luna',
                          onSelected: (val) {
                            if (val) setModalState(() => selectedVariant = 'ms_luna');
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
                      title: Text("\u{1F91D} Friends Mode", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14)),
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
                      title: Text("\u{2764}\u{FE0F} Couple Mode", style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 14)),
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

  // ================= NEW TAB 3: LUNA LINK (CLOUD SYNC) PANEL =================
  Widget _buildLunaLinkPanel(DatabaseService db, BLEService ble) {
    final firebase = Provider.of<FirebaseService>(context);
    final themeColor = _accentColor;
    
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        // Title Block
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  "LUNA LINK",
                  style: GoogleFonts.outfit(
                    color: textColor,
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Text(
                  "Cloud robot pairing & authentication",
                  style: GoogleFonts.outfit(
                    color: textColor60,
                    fontSize: 12,
                  ),
                ),
              ],
            ),
            Row(
              children: [
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                  decoration: BoxDecoration(
                    color: Colors.green.withOpacity(0.1),
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Row(
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
                        "Live Cloud Sync",
                        style: GoogleFonts.outfit(
                          color: Colors.green,
                          fontSize: 11,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
          ],
        ),
        const SizedBox(height: 16),



          // SNAPCHAT STYLE FRIENDS CARD
          GlassCard(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text(
                      "FRIENDS & MATCHMAKING",
                      style: GoogleFonts.outfit(
                        color: textColor,
                        fontSize: 14,
                        fontWeight: FontWeight.bold,
                        letterSpacing: 0.5,
                      ),
                    ),
                    Icon(Icons.people_outline, color: themeColor, size: 20),
                  ],
                ),
                const SizedBox(height: 12),

                // Search Bar
                TextField(
                  controller: _lunaLinkSearchController,
                  onChanged: (val) {
                    setState(() {});
                  },
                  decoration: InputDecoration(
                    hintText: "Search email or username...",
                    hintStyle: GoogleFonts.outfit(color: textColor38, fontSize: 13),
                    prefixIcon: const Icon(Icons.search, size: 18),
                    suffixIcon: _lunaLinkSearchController.text.isNotEmpty
                        ? GestureDetector(
                            onTap: () {
                              _lunaLinkSearchController.clear();
                              setState(() {});
                            },
                            child: const Icon(Icons.clear, size: 18),
                          )
                        : null,
                    isDense: true,
                    contentPadding: const EdgeInsets.all(10),
                    border: OutlineInputBorder(
                      borderRadius: BorderRadius.circular(12),
                      borderSide: BorderSide(color: themeColor.withOpacity(0.2)),
                    ),
                    focusedBorder: OutlineInputBorder(
                      borderRadius: BorderRadius.circular(12),
                      borderSide: BorderSide(color: themeColor),
                    ),
                  ),
                ),

                // Search results list
                if (_lunaLinkSearchController.text.isNotEmpty) ...[
                  const SizedBox(height: 12),
                  Text(
                    "Search Results",
                    style: GoogleFonts.outfit(color: textColor38, fontSize: 11, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 6),
                  ...firebase.searchProfiles(_lunaLinkSearchController.text).map((user) {
                    final isFriend = firebase.friends.any((f) => f['uid'] == user['uid']);
                    return ListTile(
                      contentPadding: EdgeInsets.zero,
                      leading: CircleAvatar(
                        backgroundImage: NetworkImage(user['photoUrl'] ?? ''),
                        radius: 16,
                      ),
                      title: Text(user['displayName'] ?? '', style: GoogleFonts.outfit(fontSize: 13, fontWeight: FontWeight.bold)),
                      subtitle: Text(user['email'] ?? '', style: GoogleFonts.outfit(fontSize: 11, color: textColor60)),
                      trailing: isFriend
                          ? Text("Friend", style: GoogleFonts.outfit(fontSize: 11, color: Colors.green, fontWeight: FontWeight.bold))
                          : ElevatedButton(
                              style: ElevatedButton.styleFrom(
                                backgroundColor: themeColor,
                                padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
                                minimumSize: Size.zero,
                                tapTargetSize: MaterialTapTargetSize.shrinkWrap,
                                shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                              ),
                              onPressed: () async {
                                final sent = await firebase.sendFriendRequest(user);
                                if (sent) {
                                  ScaffoldMessenger.of(context).showSnackBar(
                                    SnackBar(content: Text("Sent friend request to ${user['displayName']}!")),
                                  );
                                }
                              },
                              child: Text("Add", style: GoogleFonts.outfit(fontSize: 11, fontWeight: FontWeight.bold)),
                            ),
                    );
                  }).toList(),
                  if (firebase.searchProfiles(_lunaLinkSearchController.text).isEmpty)
                    Padding(
                      padding: const EdgeInsets.symmetric(vertical: 8),
                      child: Text("No users found matching query.", style: GoogleFonts.outfit(color: textColor60, fontSize: 12, fontStyle: FontStyle.italic)),
                    ),
                  const Divider(height: 24),
                ],

                // Pending Requests
                if (firebase.friendRequests.isNotEmpty) ...[
                  const SizedBox(height: 12),
                  Row(
                    children: [
                      Container(
                        width: 8,
                        height: 8,
                        decoration: const BoxDecoration(color: Colors.redAccent, shape: BoxShape.circle),
                      ),
                      const SizedBox(width: 6),
                      Text(
                        "Pending Friend Requests",
                        style: GoogleFonts.outfit(color: Colors.redAccent, fontSize: 11, fontWeight: FontWeight.bold),
                      ),
                    ],
                  ),
                  const SizedBox(height: 6),
                  ...firebase.friendRequests.map((req) {
                    return ListTile(
                      contentPadding: EdgeInsets.zero,
                      leading: CircleAvatar(
                        backgroundImage: NetworkImage(req['photoUrl'] ?? ''),
                        radius: 16,
                      ),
                      title: Text(req['displayName'] ?? '', style: GoogleFonts.outfit(fontSize: 13, fontWeight: FontWeight.bold)),
                      trailing: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          IconButton(
                            icon: const Icon(Icons.check_circle, color: Colors.green, size: 24),
                            onPressed: () => firebase.acceptFriendRequest(req['id']),
                            padding: EdgeInsets.zero,
                            constraints: const BoxConstraints(),
                          ),
                          const SizedBox(width: 8),
                          IconButton(
                            icon: const Icon(Icons.cancel, color: Colors.redAccent, size: 24),
                            onPressed: () => firebase.rejectFriendRequest(req['id']),
                            padding: EdgeInsets.zero,
                            constraints: const BoxConstraints(),
                          ),
                        ],
                      ),
                    );
                  }).toList(),
                  const Divider(height: 24),
                ],

                // Friends List
                const SizedBox(height: 8),
                Text(
                  "My Friends",
                  style: GoogleFonts.outfit(color: textColor38, fontSize: 11, fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 6),
                if (firebase.friends.isEmpty)
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 12),
                    child: Text("Add friends to pair robots over the cloud database.", style: GoogleFonts.outfit(color: textColor60, fontSize: 12, fontStyle: FontStyle.italic)),
                  )
                else
                  ...firebase.friends.map((friend) {
                    final isOnline = friend['isOnline'] == true;
                    final isPaired = firebase.pairedFriendUid == friend['uid'];
                    return ListTile(
                      contentPadding: EdgeInsets.zero,
                      leading: Stack(
                        children: [
                          CircleAvatar(
                            backgroundImage: NetworkImage(friend['photoUrl'] ?? ''),
                            radius: 18,
                          ),
                          Positioned(
                            bottom: 0,
                            right: 0,
                            child: Container(
                              width: 8,
                              height: 8,
                              decoration: BoxDecoration(
                                color: isOnline ? Colors.green : Colors.grey,
                                shape: BoxShape.circle,
                                border: Border.all(color: Colors.white, width: 1.5),
                              ),
                            ),
                          ),
                        ],
                      ),
                      title: Row(
                        children: [
                          Text(friend['displayName'] ?? '', style: GoogleFonts.outfit(fontSize: 14, fontWeight: FontWeight.bold)),
                          if (friend['streak'] != null && friend['streak']['streakCount'] != null && friend['streak']['streakCount'] > 0) ...[
                            const SizedBox(width: 6),
                            Container(
                              padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                              decoration: BoxDecoration(
                                color: Colors.amber.shade100,
                                borderRadius: BorderRadius.circular(8),
                              ),
                              child: Row(
                                children: [
                                  const Text("🔥", style: TextStyle(fontSize: 11)),
                                  const SizedBox(width: 2),
                                  Text(
                                    "${friend['streak']['streakCount']}",
                                    style: GoogleFonts.outfit(fontSize: 10, fontWeight: FontWeight.w800, color: Colors.orange.shade800),
                                  ),
                                ],
                              ),
                            ),
                          ],
                        ],
                      ),
                      subtitle: Text(
                        isPaired
                            ? "Paired Robot Link Active"
                            : (friend['robotName'] != null
                                ? "${friend['robotName']} (${friend['robotVariant'] == 'ms_luna' ? 'MS' : 'MR'})"
                                : "No Robot Registered"),
                        style: GoogleFonts.outfit(fontSize: 11, color: isPaired ? themeColor : textColor60, fontWeight: isPaired ? FontWeight.bold : FontWeight.normal),
                      ),
                      trailing: Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          if (isOnline && friend['robotId'] != null) ...[
                            IconButton(
                              icon: Icon(
                                isPaired ? Icons.cloud_done : Icons.cloud_upload_outlined,
                                color: isPaired ? Colors.green : themeColor,
                                size: 20,
                              ),
                              onPressed: () async {
                                if (isPaired) {
                                  await firebase.unpairRobot();
                                } else {
                                  await firebase.pairRobotWithFriend(friend);
                                }
                                if (ble.isConnected) {
                                  await _syncSettingsToRobot(db, ble);
                                }
                              },
                              tooltip: isPaired ? "Disconnect Cloud Link" : "Cloud Pair Robots",
                            ),
                          ],
                          IconButton(
                            icon: const Icon(Icons.delete_outline, color: Colors.redAccent, size: 20),
                            onPressed: () => firebase.unfriend(friend['uid']),
                            tooltip: "Unfriend",
                          ),
                        ],
                      ),
                    );
                  }).toList(),
              ],
            ),
          ),
          const SizedBox(height: 16),

          // CLOUD LOGS
          GlassCard(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text(
                      "SANDBOX CLOUD LOGS",
                      style: GoogleFonts.outfit(
                        color: const Color(0xFF22D3EE),
                        fontSize: 12,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    TextButton(
                      style: TextButton.styleFrom(padding: EdgeInsets.zero, minimumSize: Size.zero, tapTargetSize: MaterialTapTargetSize.shrinkWrap),
                      onPressed: () => firebase.clearCloudLogs(),
                      child: Text("CLEAR", style: GoogleFonts.outfit(color: Colors.redAccent, fontSize: 10, fontWeight: FontWeight.bold)),
                    ),
                  ],
                ),
                const SizedBox(height: 8),
                Container(
                  height: 120,
                  padding: const EdgeInsets.all(10),
                  decoration: BoxDecoration(
                    color: const Color(0xFFF8FAFC),
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(color: Colors.black.withOpacity(0.08)),
                  ),
                  child: ListView.builder(
                    itemCount: firebase.cloudLogs.length,
                    itemBuilder: (context, idx) {
                      return Text(
                        firebase.cloudLogs[idx],
                        style: GoogleFonts.firaCode(color: const Color(0xFF0F172A), fontSize: 10.5),
                      );
                    },
                  ),
                ),
              ],
            ),
          ),
        ],
    );
  }

  void _showGoogleSignInBottomSheet(BuildContext context, FirebaseService firebase) {
    final TextEditingController emailController = TextEditingController();
    final TextEditingController nameController = TextEditingController();
    final TextEditingController robotController = TextEditingController();
    String selectedVariant = 'mr_luna';
    String selectedAvatar = 'Sasi';

    final List<Map<String, String>> mockAccounts = [
      {'name': 'Sasi Dev', 'email': 'sasi.dev@gmail.com', 'avatar': 'Sasi', 'robot': 'LunaMax', 'variant': 'mr_luna'},
      {'name': 'Alice Owner', 'email': 'alice.luna@gmail.com', 'avatar': 'Alice', 'robot': 'Lumina', 'variant': 'ms_luna'},
      {'name': 'Bob Tech', 'email': 'bob.luna@gmail.com', 'avatar': 'Bob', 'robot': 'RoboBob', 'variant': 'mr_luna'},
      {'name': 'Luna Fanatic', 'email': 'luna.fanatic@gmail.com', 'avatar': 'Fanatic', 'robot': 'Rosy', 'variant': 'ms_luna'},
    ];

    showModalBottomSheet(
      context: context,
      isScrollControlled: true,
      backgroundColor: Colors.transparent,
      builder: (ctx) {
        return StatefulBuilder(
          builder: (ctx, setModalState) {
            return Container(
              decoration: const BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.only(
                  topLeft: Radius.circular(28),
                  topRight: Radius.circular(28),
                ),
              ),
              padding: EdgeInsets.fromLTRB(20, 20, 20, MediaQuery.of(ctx).viewInsets.bottom + 20),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Center(
                    child: Container(
                      width: 40,
                      height: 5,
                      decoration: BoxDecoration(
                        color: Colors.grey.shade300,
                        borderRadius: BorderRadius.circular(10),
                      ),
                    ),
                  ),
                  const SizedBox(height: 16),
                  Text(
                    "Sign In with Google",
                    style: GoogleFonts.outfit(
                      color: textColor,
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                    textAlign: TextAlign.center,
                  ),
                  const SizedBox(height: 8),
                  Text(
                    "Select a simulated Google Account to login to the cloud sandbox.",
                    style: GoogleFonts.outfit(
                      color: textColor60,
                      fontSize: 12,
                    ),
                    textAlign: TextAlign.center,
                  ),
                  const SizedBox(height: 16),

                  // MOCK ACCOUNT LIST
                  ...mockAccounts.map((acc) {
                    return Card(
                      color: Colors.grey.shade50,
                      elevation: 0,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12),
                        side: BorderSide(color: Colors.grey.shade200),
                      ),
                      margin: const EdgeInsets.only(bottom: 8),
                      child: ListTile(
                        leading: CircleAvatar(
                          backgroundImage: NetworkImage('https://api.dicebear.com/7.x/adventurer/png?seed=${acc['avatar']}'),
                          radius: 16,
                        ),
                        title: Text(acc['name']!, style: GoogleFonts.outfit(fontSize: 13, fontWeight: FontWeight.bold)),
                        subtitle: Text(acc['email']!, style: GoogleFonts.outfit(fontSize: 11, color: textColor60)),
                        trailing: Icon(Icons.arrow_forward_ios, size: 12, color: _accentColor),
                        onTap: () {
                          firebase.signInWithGoogle(
                            acc['email']!,
                            acc['name']!,
                            acc['avatar']!,
                            acc['robot']!,
                            acc['variant']!,
                          );
                          Navigator.pop(ctx);
                        },
                      ),
                    );
                  }).toList(),
                  const Divider(height: 24),
                  
                  // CUSTOM ACCOUNT ACCORDION
                  Text(
                    "OR AUTHENTICATE CUSTOM ACCOUNT",
                    style: GoogleFonts.outfit(
                      color: textColor38,
                      fontSize: 10,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 8),
                  TextField(
                    controller: nameController,
                    decoration: InputDecoration(
                      labelText: "Display Name",
                      labelStyle: GoogleFonts.outfit(fontSize: 12),
                      border: OutlineInputBorder(borderRadius: BorderRadius.circular(8)),
                      isDense: true,
                    ),
                  ),
                  const SizedBox(height: 8),
                  TextField(
                    controller: emailController,
                    decoration: InputDecoration(
                      labelText: "Google Email Address",
                      labelStyle: GoogleFonts.outfit(fontSize: 12),
                      border: OutlineInputBorder(borderRadius: BorderRadius.circular(8)),
                      isDense: true,
                    ),
                  ),
                  const SizedBox(height: 8),
                  TextField(
                    controller: robotController,
                    decoration: InputDecoration(
                      labelText: "My Robot Name",
                      labelStyle: GoogleFonts.outfit(fontSize: 12),
                      border: OutlineInputBorder(borderRadius: BorderRadius.circular(8)),
                      isDense: true,
                    ),
                  ),
                  const SizedBox(height: 8),
                  Row(
                    children: [
                      Text("Robot Model:", style: GoogleFonts.outfit(fontSize: 12, fontWeight: FontWeight.bold)),
                      const SizedBox(width: 12),
                      ChoiceChip(
                        label: Text("Mr. Luna", style: GoogleFonts.outfit(fontSize: 11)),
                        selected: selectedVariant == 'mr_luna',
                        onSelected: (val) {
                          setModalState(() {
                            selectedVariant = 'mr_luna';
                          });
                        },
                      ),
                      const SizedBox(width: 8),
                      ChoiceChip(
                        label: Text("Ms. Luna", style: GoogleFonts.outfit(fontSize: 11)),
                        selected: selectedVariant == 'ms_luna',
                        onSelected: (val) {
                          setModalState(() {
                            selectedVariant = 'ms_luna';
                          });
                        },
                      ),
                    ],
                  ),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: _accentColor,
                      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      padding: const EdgeInsets.symmetric(vertical: 12),
                    ),
                    onPressed: () {
                      final email = emailController.text.trim();
                      final name = nameController.text.trim();
                      final robot = robotController.text.trim();
                      if (email.isNotEmpty && name.isNotEmpty && robot.isNotEmpty) {
                        firebase.signInWithGoogle(email, name, name, robot, selectedVariant);
                        Navigator.pop(ctx);
                      } else {
                        ScaffoldMessenger.of(context).showSnackBar(
                          const SnackBar(content: Text("Please fill all custom fields.")),
                        );
                      }
                    },
                    child: Text("AUTHENTICATE CUSTOM", style: GoogleFonts.outfit(fontWeight: FontWeight.bold)),
                  ),
                ],
              ),
            );
          },
        );
      },
    );
  }

  String _getMonthName(int month) {
    const months = [
      "January", "February", "March", "April", "May", "June",
      "July", "August", "September", "October", "November", "December"
    ];
    return months[month - 1];
  }

  void _showEventSchedulerPopup(BuildContext context, DateTime date, DatabaseService db, BLEService ble) {
    _calendarTitleController.clear();
    String localSelectedType = 'meeting';
    TimeOfDay selectedTime = TimeOfDay.now();

    showDialog(
      context: context,
      builder: (ctx) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            return AlertDialog(
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
              backgroundColor: Colors.white,
              title: Text(
                "SCHEDULE EVENT\n${date.day} ${_getMonthName(date.month)} ${date.year}",
                style: GoogleFonts.outfit(
                  color: Colors.black87,
                  fontWeight: FontWeight.bold,
                  fontSize: 16,
                ),
                textAlign: TextAlign.center,
              ),
              content: SingleChildScrollView(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Text(
                      "EVENT TYPE",
                      style: GoogleFonts.outfit(color: const Color(0xFF64748B), fontSize: 10, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 8),
                    Wrap(
                      spacing: 8,
                      runSpacing: 8,
                      children: [
                        _buildTypeChip('meeting', 'Meeting', Icons.groups, const Color(0xFFEF4444), localSelectedType, (t) => setModalState(() => localSelectedType = t)),
                        _buildTypeChip('birthday', 'Birthday', Icons.cake, const Color(0xFFEC4899), localSelectedType, (t) => setModalState(() => localSelectedType = t)),
                        _buildTypeChip('alarm', 'Alarm', Icons.alarm, const Color(0xFF14B8A6), localSelectedType, (t) => setModalState(() => localSelectedType = t)),
                        _buildTypeChip('reminder', 'Reminder', Icons.notifications, const Color(0xFFF59E0B), localSelectedType, (t) => setModalState(() => localSelectedType = t)),
                      ],
                    ),
                    const SizedBox(height: 16),
                    Text(
                      "DESCRIPTION",
                      style: GoogleFonts.outfit(color: const Color(0xFF64748B), fontSize: 10, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 8),
                    TextField(
                      controller: _calendarTitleController,
                      style: GoogleFonts.outfit(color: Colors.black87, fontSize: 14),
                      decoration: InputDecoration(
                        hintText: "e.g., Standup, Call, Wakeup",
                        hintStyle: GoogleFonts.outfit(color: Colors.black38, fontSize: 13),
                        filled: true,
                        fillColor: Colors.black.withOpacity(0.04),
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: BorderSide(color: Colors.black.withOpacity(0.08)),
                        ),
                        enabledBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: BorderSide(color: Colors.black.withOpacity(0.08)),
                        ),
                        focusedBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(10),
                          borderSide: const BorderSide(color: Color(0xFFA855F7)),
                        ),
                      ),
                    ),
                    const SizedBox(height: 16),
                    Text(
                      "SELECT TIME",
                      style: GoogleFonts.outfit(color: const Color(0xFF64748B), fontSize: 10, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 8),
                    InkWell(
                      onTap: () async {
                        final picked = await showTimePicker(
                          context: context,
                          initialTime: selectedTime,
                          builder: (context, child) {
                            return Theme(
                              data: ThemeData.light().copyWith(
                                colorScheme: const ColorScheme.light(
                                  primary: Color(0xFFA855F7),
                                  onPrimary: Colors.white,
                                  surface: Colors.white,
                                  onSurface: Colors.black87,
                                ),
                                dialogBackgroundColor: Colors.white,
                              ),
                              child: child!,
                            );
                          },
                        );
                        if (picked != null) {
                          setModalState(() {
                            selectedTime = picked;
                          });
                        }
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 12, horizontal: 16),
                        decoration: BoxDecoration(
                          color: Colors.black.withOpacity(0.04),
                          border: Border.all(color: Colors.black.withOpacity(0.08)),
                          borderRadius: BorderRadius.circular(10),
                        ),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              selectedTime.format(context),
                              style: GoogleFonts.outfit(color: Colors.black87, fontSize: 14, fontWeight: FontWeight.bold),
                            ),
                            const Icon(Icons.access_time, color: Colors.purpleAccent, size: 18),
                          ],
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(ctx),
                  child: Text("CANCEL", style: GoogleFonts.outfit(color: Colors.black54)),
                ),
                ElevatedButton(
                  style: ElevatedButton.styleFrom(
                    backgroundColor: _accentColor,
                    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                  ),
                  onPressed: () async {
                    if (_calendarTitleController.text.trim().isEmpty) {
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(
                          content: Text("Please enter a description"),
                          backgroundColor: Colors.redAccent,
                        ),
                      );
                      return;
                    }

                    final eventDateTime = DateTime(
                      date.year,
                      date.month,
                      date.day,
                      selectedTime.hour,
                      selectedTime.minute,
                    );

                    final newEvent = CalendarEvent(
                      id: DateTime.now().millisecondsSinceEpoch.toString(),
                      title: _calendarTitleController.text.trim(),
                      dateTime: eventDateTime,
                      type: localSelectedType,
                    );

                    await db.addEvent(newEvent);

                    if (localSelectedType == 'alarm') {
                      final newAlarm = AlarmModel(
                        id: newEvent.id,
                        hour: selectedTime.hour,
                        minute: selectedTime.minute,
                        label: newEvent.title,
                        isEnabled: true,
                      );
                      await db.addAlarm(newAlarm);
                    }

                    final hh = selectedTime.hour.toString().padLeft(2, '0');
                    final mm = selectedTime.minute.toString().padLeft(2, '0');
                    final timeStr = "$hh:$mm";

                    if (ble.isConnected) {
                      await ble.transmitCalendarEvent(
                        newEvent.type,
                        timeStr,
                        newEvent.title,
                      );
                    }

                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(
                        content: Text("Scheduled '${newEvent.title}' for ${selectedTime.format(context)}!"),
                        backgroundColor: const Color(0xFF10B981),
                      ),
                    );

                    Navigator.pop(ctx);
                    setState(() {});
                  },
                  child: Text("SAVE & SYNC", style: GoogleFonts.outfit(fontWeight: FontWeight.bold, color: Colors.white)),
                ),
              ],
            );
          },
        );
      },
    );
  }

  Widget _buildTypeChip(String typeKey, String label, IconData icon, Color color, String currentType, Function(String) onSelected) {
    final isSelected = currentType == typeKey;
    return ChoiceChip(
      avatar: Icon(icon, color: isSelected ? Colors.white : color, size: 16),
      label: Text(label),
      selected: isSelected,
      onSelected: (selected) {
        if (selected) onSelected(typeKey);
      },
      selectedColor: color,
      backgroundColor: Colors.black.withOpacity(0.03),
      labelStyle: GoogleFonts.outfit(
        color: isSelected ? Colors.white : Colors.black87,
        fontSize: 12,
        fontWeight: FontWeight.bold,
      ),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(10),
        side: BorderSide(color: isSelected ? color : Colors.black.withOpacity(0.08)),
      ),
    );
  }

  // ================= NEW TAB 4: CALENDAR MANAGEMENT PANEL =================
  Widget _buildCalendarPanel(DatabaseService db, BLEService ble) {
    final year = _calendarViewDate.year;
    final month = _calendarViewDate.month;
    final firstDay = DateTime(year, month, 1);
    final startWeekday = firstDay.weekday; 
    final totalDays = DateTime(year, month + 1, 0).day;

    final prevMonthEnd = DateTime(year, month, 0);
    final int prevDaysCount = startWeekday - 1;
    final List<DateTime> cells = [];
    for (int i = prevDaysCount - 1; i >= 0; i--) {
      cells.add(DateTime(prevMonthEnd.year, prevMonthEnd.month, prevMonthEnd.day - i));
    }
    for (int i = 1; i <= totalDays; i++) {
      cells.add(DateTime(year, month, i));
    }
    final int remaining = 42 - cells.length;
    for (int i = 1; i <= remaining; i++) {
      final nextMonth = DateTime(year, month + 1, 1);
      cells.add(DateTime(nextMonth.year, nextMonth.month, i));
    }

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

        // Visual Monthly Calendar Grid
        GlassCard(
          padding: const EdgeInsets.all(12),
          child: Column(
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  IconButton(
                    icon: Icon(Icons.chevron_left, color: textColor),
                    onPressed: () {
                      setState(() {
                        _calendarViewDate = DateTime(
                          _calendarViewDate.year,
                          _calendarViewDate.month - 1,
                          1,
                        );
                      });
                    },
                  ),
                  Text(
                    "${_getMonthName(_calendarViewDate.month)} ${_calendarViewDate.year}".toUpperCase(),
                    style: GoogleFonts.outfit(
                      color: textColor,
                      fontSize: 15,
                      fontWeight: FontWeight.bold,
                      letterSpacing: 1,
                    ),
                  ),
                  IconButton(
                    icon: Icon(Icons.chevron_right, color: textColor),
                    onPressed: () {
                      setState(() {
                        _calendarViewDate = DateTime(
                          _calendarViewDate.year,
                          _calendarViewDate.month + 1,
                          1,
                        );
                      });
                    },
                  ),
                ],
              ),
              const Divider(color: Colors.white10),
              const SizedBox(height: 8),
              
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: ["Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"].map((day) {
                  return Expanded(
                    child: Center(
                      child: Text(
                        day,
                        style: GoogleFonts.outfit(
                          color: _accentColor.withOpacity(0.8),
                          fontSize: 12,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                  );
                }).toList(),
              ),
              const SizedBox(height: 8),

              GridView.builder(
                shrinkWrap: true,
                physics: const NeverScrollableScrollPhysics(),
                itemCount: 42,
                gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
                  crossAxisCount: 7,
                  childAspectRatio: 1.0,
                  crossAxisSpacing: 4,
                  mainAxisSpacing: 4,
                ),
                itemBuilder: (context, idx) {
                  final cellDate = cells[idx];
                  final isCurrentMonth = cellDate.month == month;
                  final isToday = cellDate.year == DateTime.now().year &&
                      cellDate.month == DateTime.now().month &&
                      cellDate.day == DateTime.now().day;

                  final dayEvents = db.events.where((e) {
                    return e.dateTime.year == cellDate.year &&
                        e.dateTime.month == cellDate.month &&
                        e.dateTime.day == cellDate.day;
                  }).toList();

                  return GestureDetector(
                    onTap: () => _showEventSchedulerPopup(context, cellDate, db, ble),
                    child: Container(
                      decoration: BoxDecoration(
                        color: isToday
                            ? _accentColor.withOpacity(0.15)
                            : (isCurrentMonth ? Colors.white.withOpacity(0.03) : Colors.transparent),
                        border: Border.all(
                          color: isToday
                              ? _accentColor
                              : (isCurrentMonth ? Colors.white.withOpacity(0.05) : Colors.transparent),
                          width: isToday ? 1.5 : 1.0,
                        ),
                        borderRadius: BorderRadius.circular(8),
                      ),
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Text(
                            "${cellDate.day}",
                            style: GoogleFonts.outfit(
                              color: isCurrentMonth
                                  ? (isToday ? _accentColor : textColor)
                                  : textColor38.withOpacity(0.3),
                              fontSize: 13,
                              fontWeight: isToday ? FontWeight.bold : FontWeight.normal,
                            ),
                          ),
                          if (dayEvents.isNotEmpty) ...[
                            const SizedBox(height: 4),
                            Row(
                              mainAxisAlignment: MainAxisAlignment.center,
                              children: dayEvents.take(4).map((ev) {
                                Color dotColor = const Color(0xFFE2E8F0);
                                if (ev.type == 'meeting') dotColor = const Color(0xFFEF4444);
                                else if (ev.type == 'birthday') dotColor = const Color(0xFFEC4899);
                                else if (ev.type == 'alarm') dotColor = const Color(0xFF14B8A6);
                                else if (ev.type == 'reminder') dotColor = const Color(0xFFF59E0B);
                                return Container(
                                  width: 4,
                                  height: 4,
                                  margin: const EdgeInsets.symmetric(horizontal: 1),
                                  decoration: BoxDecoration(
                                    color: dotColor,
                                    shape: BoxShape.circle,
                                  ),
                                );
                              }).toList(),
                            )
                          ]
                        ],
                      ),
                    ),
                  );
                },
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
              "No upcoming meetings, birthdays, alarms or reminders.",
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
              final isBirthday = event.type == 'birthday';
              final isAlarm = event.type == 'alarm';
              final isReminder = event.type == 'reminder';

              final hh = event.dateTime.hour.toString().padLeft(2, '0');
              final mm = event.dateTime.minute.toString().padLeft(2, '0');
              final timeStr = "$hh:$mm";
              final dateStr = "${event.dateTime.day}/${event.dateTime.month}/${event.dateTime.year}";

              IconData eventIcon = Icons.groups;
              List<Color> gradientColors = [const Color(0xFFE53935), const Color(0xFFFFB300)];
              String typeLabel = "Event";

              if (isMeeting) {
                eventIcon = Icons.groups;
                gradientColors = [const Color(0xFFE53935), const Color(0xFFFFB300)];
                typeLabel = "Meeting @ $timeStr ($dateStr)";
              } else if (isBirthday) {
                eventIcon = Icons.cake;
                gradientColors = [const Color(0xFFEC4899), const Color(0xFFF43F5E)];
                typeLabel = "Birthday ($dateStr)";
              } else if (isAlarm) {
                eventIcon = Icons.alarm;
                gradientColors = [const Color(0xFF14B8A6), const Color(0xFF0D9488)];
                typeLabel = "Alarm @ $timeStr ($dateStr)";
              } else if (isReminder) {
                eventIcon = Icons.notifications;
                gradientColors = [const Color(0xFFF59E0B), const Color(0xDDF59E0B)];
                typeLabel = "Reminder @ $timeStr ($dateStr)";
              }

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
                    Container(
                      width: 36,
                      height: 36,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        gradient: LinearGradient(colors: gradientColors),
                      ),
                      child: Icon(
                        eventIcon,
                        color: textColor,
                        size: 18,
                      ),
                    ),
                    const SizedBox(width: 12),

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
                            typeLabel,
                            style: GoogleFonts.outfit(
                              color: textColor54,
                              fontSize: 11,
                            ),
                          ),
                        ],
                      ),
                    ),

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
                            if (isAlarm) {
                              await db.deleteAlarm(event.id);
                            }
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
        const SizedBox(height: 28),
        _buildAlarmsSection(db, ble),
      ],
    );
  }

  // ================= NEW TAB 4: ADVANCED SETTINGS PANEL =================
  Widget _buildSectionHeader(String title, IconData icon, Color color) {
    return Row(
      children: [
        Icon(icon, color: color, size: 18),
        const SizedBox(width: 8),
        Text(
          title.toUpperCase(),
          style: GoogleFonts.outfit(
            color: textColor,
            fontSize: 13,
            fontWeight: FontWeight.bold,
            letterSpacing: 1.0,
          ),
        ),
      ],
    );
  }

  Widget _buildAlarmsSection(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSectionHeader("Alarms & Reminders", Icons.alarm, Colors.teal.shade600),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              if (db.alarms.isEmpty)
                Padding(
                  padding: const EdgeInsets.symmetric(vertical: 24),
                  child: Center(
                    child: Text(
                      "No active alarms. Tap below to create one.",
                      style: GoogleFonts.outfit(color: textColor60, fontSize: 13),
                    ),
                  ),
                )
              else
                ListView.separated(
                  shrinkWrap: true,
                  physics: const NeverScrollableScrollPhysics(),
                  itemCount: db.alarms.length,
                  separatorBuilder: (context, index) => Divider(color: textColor12, height: 16),
                  itemBuilder: (context, index) {
                    final alarm = db.alarms[index];
                    return Row(
                      children: [
                        Icon(Icons.alarm, color: Colors.teal.shade400, size: 24),
                        const SizedBox(width: 12),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                alarm.formatTime(db.is12HourFormat),
                                style: GoogleFonts.outfit(
                                  color: textColor,
                                  fontSize: 18,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              if (alarm.label.isNotEmpty)
                                Text(
                                  alarm.label,
                                  style: GoogleFonts.outfit(
                                    color: textColor60,
                                    fontSize: 12,
                                  ),
                                ),
                            ],
                          ),
                        ),
                        Switch(
                          value: alarm.isEnabled,
                          activeColor: Colors.teal.shade400,
                          onChanged: (val) {
                            db.toggleAlarm(alarm.id);
                          },
                        ),
                        IconButton(
                          icon: const Icon(Icons.delete_outline, color: Colors.redAccent, size: 22),
                          onPressed: () {
                            db.deleteAlarm(alarm.id);
                          },
                        ),
                      ],
                    );
                  },
                ),
              const SizedBox(height: 16),
              ElevatedButton.icon(
                onPressed: () => _addAlarmFlow(db, ble),
                icon: const Icon(Icons.add_alarm),
                label: const Text("ADD NEW ALARM"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.teal.shade600,
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(vertical: 12),
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }

  Future<void> _addAlarmFlow(DatabaseService db, BLEService ble) async {
    final TimeOfDay? picked = await showTimePicker(
      context: context,
      initialTime: TimeOfDay.now(),
    );
    if (picked != null) {
      final alarm = AlarmModel(
        id: DateTime.now().millisecondsSinceEpoch.toString(),
        hour: picked.hour,
        minute: picked.minute,
        label: "Alarm",
        isEnabled: true,
      );
      await db.addAlarm(alarm);

      final hh = picked.hour.toString().padLeft(2, '0');
      final mm = picked.minute.toString().padLeft(2, '0');
      if (ble.isConnected) {
        await ble.transmitCalendarEvent("alarm", "$hh:$mm", "Alarm");
      }
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Alarm set for $hh:$mm!"),
          backgroundColor: const Color(0xFF10B981),
        ),
      );
    }
  }

  Widget _buildNotificationSyncPanel(DatabaseService db, BLEService ble) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildSectionHeader("Notification Sync Settings", Icons.notifications_active, Colors.indigo.shade600),
        const SizedBox(height: 12),
        GlassCard(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          "Forward Phone Notifications",
                          style: GoogleFonts.outfit(
                            color: textColor,
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          "Forward incoming notifications from all apps to the robot OLED screen.",
                          style: GoogleFonts.outfit(
                            color: textColor60,
                            fontSize: 12,
                          ),
                        ),
                      ],
                    ),
                  ),
                  Switch(
                    value: db.notificationSyncEnabled,
                    activeColor: Colors.indigo.shade400,
                    onChanged: (val) async {
                      if (val && !_isNotificationPermissionGranted) {
                        _requestNotificationPermission();
                      } else {
                        await db.updateNotificationSyncEnabled(val);
                      }
                    },
                  ),
                ],
              ),
              const SizedBox(height: 12),
              Row(
                children: [
                  Icon(
                    _isNotificationPermissionGranted ? Icons.check_circle : Icons.warning,
                    color: _isNotificationPermissionGranted ? Colors.green : Colors.amber,
                    size: 16,
                  ),
                  const SizedBox(width: 8),
                  Text(
                    _isNotificationPermissionGranted
                        ? "Notification Listener access is GRANTED"
                        : "Notification Listener access is REQUIRED",
                    style: GoogleFonts.outfit(
                      color: _isNotificationPermissionGranted ? Colors.green : Colors.amber.shade700,
                      fontSize: 12,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const Spacer(),
                  if (!_isNotificationPermissionGranted)
                    TextButton(
                      onPressed: _requestNotificationPermission,
                      child: Text(
                        "GRANT",
                        style: GoogleFonts.outfit(
                          color: _accentColor,
                          fontWeight: FontWeight.bold,
                          fontSize: 12,
                        ),
                      ),
                    ),
                ],
              ),
              const SizedBox(height: 8),
              Row(
                children: [
                  Icon(
                    _isPostNotificationsPermissionGranted ? Icons.check_circle : Icons.warning,
                    color: _isPostNotificationsPermissionGranted ? Colors.green : Colors.amber,
                    size: 16,
                  ),
                  const SizedBox(width: 8),
                  Text(
                    _isPostNotificationsPermissionGranted
                        ? "System Notification Bar is GRANTED"
                        : "System Notification Bar is REQUIRED",
                    style: GoogleFonts.outfit(
                      color: _isPostNotificationsPermissionGranted ? Colors.green : Colors.amber.shade700,
                      fontSize: 12,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const Spacer(),
                  if (!_isPostNotificationsPermissionGranted)
                    TextButton(
                      onPressed: () async {
                        final service = Provider.of<PhoneNotificationService>(context, listen: false);
                        await service.requestPostNotificationsPermission();
                        await service.startBackgroundService();
                      },
                      child: Text(
                        "GRANT",
                        style: GoogleFonts.outfit(
                          color: _accentColor,
                          fontWeight: FontWeight.bold,
                          fontSize: 12,
                        ),
                      ),
                    ),
                ],
              ),
              const Divider(color: Colors.white12, height: 24),
              _buildDurationSlider(
                title: "Notification Duration",
                subtitle: "How long standard notifications remain on screen",
                value: db.notificationDuration,
                min: 2,
                max: 30,
                onChanged: (val) => db.updateNotificationDuration(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const SizedBox(height: 16),
              _buildDurationSlider(
                title: "Reminder Duration",
                subtitle: "How long calendar reminders remain on screen",
                value: db.reminderDuration,
                min: 2,
                max: 30,
                onChanged: (val) => db.updateReminderDuration(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const SizedBox(height: 16),
              _buildDurationSlider(
                title: "Birthday Duration",
                subtitle: "How long birthday notifications remain on screen",
                value: db.birthdayDuration,
                min: 2,
                max: 30,
                onChanged: (val) => db.updateBirthdayDuration(val),
                onChangeEnd: (val) => _syncSettingsToRobot(db, ble),
              ),
              const Divider(color: Colors.white12, height: 24),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          "Filter Applications",
                          style: GoogleFonts.outfit(
                            color: textColor,
                            fontSize: 14,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(height: 4),
                        Text(
                          "Choose which apps can forward notifications to your robot.",
                          style: GoogleFonts.outfit(
                            color: textColor60,
                            fontSize: 11,
                          ),
                        ),
                      ],
                    ),
                  ),
                  ElevatedButton.icon(
                    onPressed: () => _showAppSelectionDialog(db),
                    icon: const Icon(Icons.apps, size: 16),
                    label: Text(
                      "${db.allowedNotificationApps.length} Apps",
                      style: GoogleFonts.outfit(fontSize: 12, fontWeight: FontWeight.bold),
                    ),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: _accentColor,
                      foregroundColor: Colors.white,
                      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
                      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
                      elevation: 2,
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildAppFilterChip(DatabaseService db, String appKey, String label, IconData icon) {
    final isSelected = db.allowedNotificationApps.contains(appKey);
    return GestureDetector(
      onTap: () async {
        final List<String> updated = List.from(db.allowedNotificationApps);
        if (!isSelected) {
          if (!updated.contains(appKey)) {
            updated.add(appKey);
          }
        } else {
          updated.remove(appKey);
        }
        await db.updateAllowedNotificationApps(updated);
      },
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 150),
        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 8),
        decoration: BoxDecoration(
          color: isSelected ? _accentColor : Colors.black.withOpacity(0.04),
          borderRadius: BorderRadius.circular(30),
          border: Border.all(
            color: isSelected ? _accentColor : Colors.black.withOpacity(0.08),
            width: 1,
          ),
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            _buildAppIconWithCheck(icon, isSelected),
            const SizedBox(width: 8),
            Text(
              label,
              style: GoogleFonts.outfit(
                color: isSelected ? Colors.white : textColor,
                fontSize: 12.5,
                fontWeight: FontWeight.bold,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildAppIconWithCheck(IconData icon, bool isSelected) {
    return Stack(
      clipBehavior: Clip.none,
      children: [
        Container(
          width: 22,
          height: 22,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            color: isSelected ? Colors.white24 : Colors.black.withOpacity(0.05),
            border: Border.all(
              color: isSelected ? Colors.white : Colors.black26,
              width: 1.2,
            ),
          ),
          child: Icon(
            icon,
            size: 11,
            color: isSelected ? Colors.white : textColor60,
          ),
        ),
        if (isSelected)
          Positioned(
            right: -2,
            bottom: -2,
            child: Container(
              width: 9,
              height: 9,
              decoration: const BoxDecoration(
                shape: BoxShape.circle,
                color: Colors.white,
              ),
              child: Center(
                child: Icon(
                  Icons.check,
                  size: 7,
                  color: _accentColor,
                ),
              ),
            ),
          ),
      ],
    );
  }

  Future<void> _loadInstalledApps() async {
    if (_isLoadingApps) return;
    setState(() {
      _isLoadingApps = true;
    });
    try {
      const channel = MethodChannel('com.mrmsluna/notifications');
      final List<dynamic>? apps = await channel.invokeMethod<List<dynamic>>('getInstalledApps');
      if (apps != null) {
        final List<Map<String, String>> loaded = apps.map((item) {
          final map = item as Map<dynamic, dynamic>;
          return {
            'name': (map['name'] ?? '').toString(),
            'packageName': (map['packageName'] ?? '').toString(),
          };
        }).toList();
        setState(() {
          _installedApps = loaded;
        });
      }
    } catch (e) {
      debugPrint("Failed to load installed apps: $e");
    } finally {
      setState(() {
        _isLoadingApps = false;
      });
    }
  }

  IconData _getAppIcon(String packageName) {
    final pkg = packageName.toLowerCase();
    if (pkg.contains('whatsapp')) return Icons.message;
    if (pkg.contains('instagram')) return Icons.camera_alt;
    if (pkg.contains('snapchat')) return Icons.chat_bubble_outline;
    if (pkg.contains('telegram')) return Icons.send;
    if (pkg.contains('messenger')) return Icons.chat;
    if (pkg.contains('maps')) return Icons.map;
    if (pkg.contains('gmail') || pkg.contains('email') || pkg.contains('mail')) return Icons.email;
    if (pkg.contains('youtube')) return Icons.play_circle;
    if (pkg.contains('sms') || pkg.contains('mms') || pkg.contains('message')) return Icons.sms;
    if (pkg.contains('phone') || pkg.contains('dialer') || pkg.contains('call')) return Icons.phone;
    if (pkg.contains('calendar')) return Icons.calendar_month;
    if (pkg.contains('clock') || pkg.contains('alarm')) return Icons.alarm;
    if (pkg.contains('camera')) return Icons.camera;
    if (pkg.contains('gallery') || pkg.contains('photos')) return Icons.photo;
    if (pkg.contains('music') || pkg.contains('spotify')) return Icons.music_note;
    if (pkg.contains('chrome') || pkg.contains('browser')) return Icons.web;
    if (pkg.contains('settings')) return Icons.settings;
    return Icons.apps;
  }

  void _showAppSelectionDialog(DatabaseService db) {
    showGeneralDialog(
      context: context,
      barrierDismissible: true,
      barrierLabel: "App Selection",
      barrierColor: Colors.black.withOpacity(0.5),
      transitionDuration: const Duration(milliseconds: 250),
      pageBuilder: (context, anim1, anim2) {
        return StatefulBuilder(
          builder: (context, setModalState) {
            final searchQuery = _appSearchController.text.trim().toLowerCase();
            final filteredApps = _installedApps.where((app) {
              final name = app['name']?.toLowerCase() ?? '';
              final pkg = app['packageName']?.toLowerCase() ?? '';
              return name.contains(searchQuery) || pkg.contains(searchQuery);
            }).toList();

            // Sort: selected apps first, then alphabetical by name
            filteredApps.sort((a, b) {
              final aSelected = db.allowedNotificationApps.contains(a['packageName']);
              final bSelected = db.allowedNotificationApps.contains(b['packageName']);
              if (aSelected && !bSelected) return -1;
              if (!aSelected && bSelected) return 1;
              final aName = a['name']?.toLowerCase() ?? '';
              final bName = b['name']?.toLowerCase() ?? '';
              return aName.compareTo(bName);
            });

            return Align(
              alignment: Alignment.center,
              child: Container(
                width: MediaQuery.of(context).size.width * 0.9,
                height: MediaQuery.of(context).size.height * 0.8,
                decoration: BoxDecoration(
                  color: const Color(0xFFF8FAFC),
                  borderRadius: BorderRadius.circular(28),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black26,
                      blurRadius: 20,
                      offset: const Offset(0, 10),
                    ),
                  ],
                ),
                child: Scaffold(
                  backgroundColor: Colors.transparent,
                  body: Padding(
                    padding: const EdgeInsets.all(20.0),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.stretch,
                      children: [
                        // Header
                        Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              "Filter Applications",
                              style: GoogleFonts.outfit(
                                color: textColor,
                                fontSize: 18,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            IconButton(
                              icon: const Icon(Icons.close, size: 20),
                              color: textColor54,
                              onPressed: () => Navigator.pop(context),
                            ),
                          ],
                        ),
                        const SizedBox(height: 4),
                        Text(
                          "Configure which applications are permitted to send notifications to Mr.&Ms Luna.",
                          style: GoogleFonts.outfit(
                            color: textColor60,
                            fontSize: 12,
                          ),
                        ),
                        const SizedBox(height: 16),
                        
                        // Search bar & Refresh
                        Row(
                          children: [
                            Expanded(
                              child: TextField(
                                controller: _appSearchController,
                                style: GoogleFonts.outfit(color: textColor, fontSize: 13),
                                decoration: InputDecoration(
                                  hintText: "Search installed apps...",
                                  hintStyle: GoogleFonts.outfit(color: textColor38, fontSize: 13),
                                  prefixIcon: const Icon(Icons.search, color: textColor38, size: 18),
                                  filled: true,
                                  fillColor: Colors.black.withOpacity(0.04),
                                  contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                                  border: OutlineInputBorder(
                                    borderRadius: BorderRadius.circular(24),
                                    borderSide: BorderSide.none,
                                  ),
                                ),
                                onChanged: (val) {
                                  setModalState(() {});
                                },
                              ),
                            ),
                            const SizedBox(width: 8),
                            IconButton(
                              icon: const Icon(Icons.refresh, size: 18),
                              color: _accentColor,
                              onPressed: () async {
                                await _loadInstalledApps();
                                setModalState(() {});
                              },
                            ),
                          ],
                        ),
                        const SizedBox(height: 16),
                        
                        // Select All / Clear All
                        Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              "Showing ${filteredApps.length} apps",
                              style: GoogleFonts.outfit(color: textColor54, fontSize: 12, fontWeight: FontWeight.w600),
                            ),
                            Row(
                              children: [
                                TextButton(
                                  onPressed: () async {
                                    final List<String> allPkgs = _installedApps.map((a) => a['packageName']!).toList();
                                    await db.updateAllowedNotificationApps(allPkgs);
                                    setModalState(() {});
                                    setState(() {});
                                  },
                                  child: Text("Select All", style: GoogleFonts.outfit(color: _accentColor, fontSize: 12, fontWeight: FontWeight.bold)),
                                ),
                                const SizedBox(width: 8),
                                TextButton(
                                  onPressed: () async {
                                    await db.updateAllowedNotificationApps([]);
                                    setModalState(() {});
                                    setState(() {});
                                  },
                                  child: Text("Clear All", style: GoogleFonts.outfit(color: Colors.redAccent, fontSize: 12, fontWeight: FontWeight.bold)),
                                ),
                              ],
                            ),
                          ],
                        ),
                        const Divider(color: Colors.black12, height: 16),
                        
                        // Scrollable List
                        Expanded(
                          child: _isLoadingApps
                              ? Center(
                                  child: CircularProgressIndicator(color: _accentColor),
                                )
                              : filteredApps.isEmpty
                                  ? Center(
                                      child: Text(
                                        _installedApps.isEmpty ? "No apps loaded. Tap refresh." : "No apps matching search.",
                                        style: GoogleFonts.outfit(color: textColor38, fontSize: 13),
                                      ),
                                    )
                                  : ListView.builder(
                                      itemCount: filteredApps.length,
                                      itemBuilder: (context, index) {
                                        final app = filteredApps[index];
                                        final appKey = app['packageName']!;
                                        final label = app['name']!;
                                        final icon = _getAppIcon(appKey);
                                        final isSelected = db.allowedNotificationApps.contains(appKey);
                                        return GestureDetector(
                                          onTap: () async {
                                            final List<String> updated = List.from(db.allowedNotificationApps);
                                            if (!isSelected) {
                                              if (!updated.contains(appKey)) {
                                                updated.add(appKey);
                                              }
                                            } else {
                                              updated.remove(appKey);
                                            }
                                            await db.updateAllowedNotificationApps(updated);
                                            setModalState(() {});
                                            setState(() {});
                                          },
                                          child: Container(
                                            margin: const EdgeInsets.symmetric(vertical: 4),
                                            padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
                                            decoration: BoxDecoration(
                                              color: isSelected ? _accentColor.withOpacity(0.06) : Colors.transparent,
                                              borderRadius: BorderRadius.circular(16),
                                              border: Border.all(
                                                color: isSelected ? _accentColor.withOpacity(0.15) : Colors.black.withOpacity(0.03),
                                                width: 1,
                                              ),
                                            ),
                                            child: Row(
                                              children: [
                                                // App Icon Circle
                                                Container(
                                                  width: 38,
                                                  height: 38,
                                                  decoration: BoxDecoration(
                                                    color: isSelected ? _accentColor : Colors.black.withOpacity(0.04),
                                                    shape: BoxShape.circle,
                                                  ),
                                                  child: Icon(
                                                    icon,
                                                    size: 18,
                                                    color: isSelected ? Colors.white : textColor60,
                                                  ),
                                                ),
                                                const SizedBox(width: 14),
                                                // App Name & Package
                                                Expanded(
                                                  child: Column(
                                                    crossAxisAlignment: CrossAxisAlignment.start,
                                                    children: [
                                                      Text(
                                                        label,
                                                        style: GoogleFonts.outfit(
                                                          color: textColor,
                                                          fontSize: 14,
                                                          fontWeight: FontWeight.bold,
                                                        ),
                                                      ),
                                                      const SizedBox(height: 2),
                                                      Text(
                                                        appKey,
                                                        style: GoogleFonts.outfit(
                                                          color: textColor38,
                                                          fontSize: 10.5,
                                                        ),
                                                        maxLines: 1,
                                                        overflow: TextOverflow.ellipsis,
                                                      ),
                                                    ],
                                                  ),
                                                ),
                                                // Selection State Indicator
                                                Checkbox(
                                                  value: isSelected,
                                                  activeColor: _accentColor,
                                                  shape: RoundedRectangleBorder(
                                                    borderRadius: BorderRadius.circular(4),
                                                  ),
                                                  onChanged: (val) async {
                                                    final List<String> updated = List.from(db.allowedNotificationApps);
                                                    if (val == true) {
                                                      if (!updated.contains(appKey)) {
                                                        updated.add(appKey);
                                                      }
                                                    } else {
                                                      updated.remove(appKey);
                                                    }
                                                    await db.updateAllowedNotificationApps(updated);
                                                    setModalState(() {});
                                                    setState(() {});
                                                  },
                                                ),
                                              ],
                                            ),
                                          ),
                                        );
                                      },
                                    ),
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            );
          },
        );
      },
      transitionBuilder: (context, anim1, anim2, child) {
        return ScaleTransition(
          scale: CurvedAnimation(parent: anim1, curve: Curves.easeOutBack),
          child: FadeTransition(
            opacity: anim1,
            child: child,
          ),
        );
      },
    );
  }

  Widget _buildDurationSlider({
    required String title,
    required String subtitle,
    required double value,
    required double min,
    required double max,
    required ValueChanged<double> onChanged,
    required ValueChanged<double> onChangeEnd,
  }) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    title,
                    style: GoogleFonts.outfit(
                      color: textColor,
                      fontSize: 14,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 2),
                  Text(
                    subtitle,
                    style: GoogleFonts.outfit(
                      color: textColor60,
                      fontSize: 11,
                    ),
                  ),
                ],
              ),
            ),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
              decoration: BoxDecoration(
                color: Colors.white.withOpacity(0.05),
                borderRadius: BorderRadius.circular(6),
              ),
              child: Text(
                "${value.round()}s",
                style: GoogleFonts.outfit(
                  color: _accentColor,
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: 4),
        SliderTheme(
          data: SliderTheme.of(context).copyWith(
            activeTrackColor: _accentColor,
            inactiveTrackColor: Colors.white10,
            thumbColor: _accentColor,
            overlayColor: _accentColor.withOpacity(0.2),
            valueIndicatorColor: _accentColor,
            trackHeight: 3,
            thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6),
            overlayShape: const RoundSliderOverlayShape(overlayRadius: 14),
          ),
          child: Slider(
            value: value,
            min: min,
            max: max,
            divisions: (max - min).toInt(),
            onChanged: onChanged,
            onChangeEnd: onChangeEnd,
          ),
        ),
      ],
    );
  }

  Widget _buildProfilePanel(DatabaseService db, BLEService ble) {
    final firebase = Provider.of<FirebaseService>(context);
    final user = firebase.currentUser;
    if (user == null) {
      return Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const SizedBox(height: 40),
          const CircularProgressIndicator(),
          const SizedBox(height: 16),
          Text(
            "Loading User Profile...",
            style: GoogleFonts.outfit(color: textColor60, fontSize: 14),
          ),
        ],
      );
    }

    if (!_isProfileInitialized) {
      _profileDisplayNameController.text = user['displayName'] ?? '';
      final rName = (user['robotName'] != null && user['robotName'].toString().isNotEmpty)
          ? user['robotName']
          : (db.primaryRobot?.name ?? 'Mr. Luna Robot');
      final rVariant = user['robotVariant'] ?? (db.primaryRobot?.variant ?? 'mr_luna');
      _profileRobotNameController.text = rName;
      _profileSelectedVariant = rVariant;
      _isProfileInitialized = true;

      if (db.primaryRobot != null && (db.primaryRobot!.variant != rVariant || db.primaryRobot!.name != rName)) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          db.updateRobotProfile(db.primaryRobot!.id, rName, rVariant);
        });
      }
    }

    final isPink = _profileSelectedVariant == 'ms_luna';
    final themeColor = isPink ? const Color(0xFFEC4899) : const Color(0xFF0074D9);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          "MY PROFILE",
          style: GoogleFonts.outfit(
            color: textColor,
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 4),
        Text(
          "Manage your user profile and personalize your companion robot model.",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 13,
          ),
        ),
        const SizedBox(height: 20),

        // User Profile GlassCard
        GlassCard(
          padding: const EdgeInsets.all(20),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Center(
                child: Stack(
                  children: [
                    CircleAvatar(
                      radius: 44,
                      backgroundImage: NetworkImage(user['photoUrl'] ?? 'https://api.dicebear.com/7.x/adventurer/png?seed=Luna'),
                      backgroundColor: themeColor.withOpacity(0.1),
                    ),
                    Positioned(
                      bottom: 0,
                      right: 0,
                      child: Container(
                        padding: const EdgeInsets.all(6),
                        decoration: BoxDecoration(
                          color: themeColor,
                          shape: BoxShape.circle,
                          border: Border.all(color: Colors.white, width: 2),
                        ),
                        child: const Icon(Icons.person, size: 14, color: Colors.white),
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 12),
              Text(
                user['email'] ?? '',
                textAlign: TextAlign.center,
                style: GoogleFonts.outfit(
                  color: textColor60,
                  fontSize: 13,
                  fontWeight: FontWeight.w500,
                ),
              ),
              const SizedBox(height: 24),
              
              // Inputs
              TextField(
                controller: _profileDisplayNameController,
                style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                decoration: InputDecoration(
                  labelText: "My Display Name",
                  labelStyle: GoogleFonts.outfit(fontSize: 12, color: textColor60),
                  prefixIcon: const Icon(Icons.person_outline, size: 18, color: textColor60),
                  border: OutlineInputBorder(borderRadius: BorderRadius.circular(12)),
                  enabledBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(12),
                    borderSide: BorderSide(color: textColor.withOpacity(0.15)),
                  ),
                  focusedBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(12),
                    borderSide: BorderSide(color: themeColor, width: 1.5),
                  ),
                ),
              ),
              const SizedBox(height: 16),
              TextField(
                controller: _profileRobotNameController,
                style: GoogleFonts.outfit(color: textColor, fontSize: 14),
                decoration: InputDecoration(
                  labelText: "Robot Name",
                  labelStyle: GoogleFonts.outfit(fontSize: 12, color: textColor60),
                  prefixIcon: const Icon(Icons.android_outlined, size: 18, color: textColor60),
                  border: OutlineInputBorder(borderRadius: BorderRadius.circular(12)),
                  enabledBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(12),
                    borderSide: BorderSide(color: textColor.withOpacity(0.15)),
                  ),
                  focusedBorder: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(12),
                    borderSide: BorderSide(color: themeColor, width: 1.5),
                  ),
                ),
              ),
              const SizedBox(height: 16),
              
              // Variant Choice
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text(
                    "Robot Model:",
                    style: GoogleFonts.outfit(fontSize: 13, fontWeight: FontWeight.bold, color: textColor),
                  ),
                  Row(
                    children: [
                      ChoiceChip(
                        label: Text("Ms. Luna", style: GoogleFonts.outfit(fontSize: 11, color: _profileSelectedVariant == 'ms_luna' ? Colors.pink.shade700 : textColor)),
                        selected: _profileSelectedVariant == 'ms_luna',
                        selectedColor: const Color(0xFFEC4899).withOpacity(0.2),
                        onSelected: (val) {
                          if (val) {
                            setState(() {
                              _profileSelectedVariant = 'ms_luna';
                              if (_profileRobotNameController.text == "Mr. Luna Robot" || _profileRobotNameController.text.isEmpty) {
                                _profileRobotNameController.text = "Ms. Luna Robot";
                              }
                            });
                          }
                        },
                      ),
                      const SizedBox(width: 8),
                      ChoiceChip(
                        label: Text("Mr. Luna", style: GoogleFonts.outfit(fontSize: 11, color: _profileSelectedVariant == 'mr_luna' ? Colors.blue.shade700 : textColor)),
                        selected: _profileSelectedVariant == 'mr_luna',
                        selectedColor: const Color(0xFF0074D9).withOpacity(0.2),
                        onSelected: (val) {
                          if (val) {
                            setState(() {
                              _profileSelectedVariant = 'mr_luna';
                              if (_profileRobotNameController.text == "Ms. Luna Robot" || _profileRobotNameController.text.isEmpty) {
                                _profileRobotNameController.text = "Mr. Luna Robot";
                              }
                            });
                          }
                        },
                      ),
                    ],
                  ),
                ],
              ),
              const SizedBox(height: 20),
              
              // Action Buttons row
              Row(
                children: [
                  Expanded(
                    child: OutlinedButton(
                      style: OutlinedButton.styleFrom(
                        side: BorderSide(color: Colors.red.shade400, width: 1.2),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                        padding: const EdgeInsets.symmetric(vertical: 12),
                      ),
                      onPressed: () async {
                        await firebase.signOut();
                        if (context.mounted) {
                          Navigator.pushReplacement(
                            context,
                            MaterialPageRoute(builder: (context) => const LoginScreen()),
                          );
                        }
                      },
                      child: Text(
                        "SIGN OUT",
                        style: GoogleFonts.outfit(
                          color: Colors.red.shade600,
                          fontWeight: FontWeight.bold,
                          fontSize: 12,
                        ),
                      ),
                    ),
                  ),
                  const SizedBox(width: 12),
                  Expanded(
                    child: ElevatedButton(
                      style: ElevatedButton.styleFrom(
                        backgroundColor: themeColor,
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                        padding: const EdgeInsets.symmetric(vertical: 12),
                        elevation: 0,
                      ),
                      onPressed: () async {
                        final dName = _profileDisplayNameController.text.trim();
                        final rName = _profileRobotNameController.text.trim();
                        if (dName.isNotEmpty && rName.isNotEmpty) {
                          await firebase.updateUserProfile(dName, rName, _profileSelectedVariant!);
                          final isMsLuna = _profileSelectedVariant == 'ms_luna';
                          await db.updateNegativeEnabled(isMsLuna);
                          await db.updateOledInvert(isMsLuna);

                          if (db.primaryRobot != null) {
                            await db.updateRobotProfile(db.primaryRobot!.id, rName, _profileSelectedVariant!);
                          } else {
                            final newRobot = RobotProfile(
                              id: ble.connectedDevice?.remoteId.str ?? 'primary_robot',
                              name: rName,
                              variant: _profileSelectedVariant!,
                              remoteId: ble.connectedDevice?.remoteId.str ?? '00:00:00:00:00:00',
                              isPrimary: true,
                              lastConnected: DateTime.now(),
                            );
                            await db.addRobot(newRobot);
                          }

                          if (ble.isConnected) {
                            await ble.transmitModelVariant(_profileSelectedVariant!);
                          }
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text("Profile updated and synced to hardware!")),
                          );
                          setState(() {});
                        } else {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text("Names cannot be empty")),
                          );
                        }
                      },
                      child: Text(
                        "SAVE CHANGES",
                        style: GoogleFonts.outfit(
                          fontWeight: FontWeight.bold,
                          fontSize: 12,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
        
        const SizedBox(height: 32),
        
        // ------------------ ALL SETTINGS UNDER PROFILE ------------------
        Text(
          "ROBOT & SYSTEM SETTINGS",
          style: GoogleFonts.outfit(
            color: textColor,
            fontSize: 16,
            fontWeight: FontWeight.bold,
          ),
        ),
        const SizedBox(height: 4),
        Text(
          "Configure gesture controls, sync schedules, and device settings below.",
          style: GoogleFonts.outfit(
            color: textColor60,
            fontSize: 12,
          ),
        ),
        const SizedBox(height: 16),

        // 1. Companion Profiles
        _buildSectionHeader("Companion Profiles", Icons.people, Colors.blue.shade600),
        const SizedBox(height: 12),
        _buildCompanionsPanel(db, ble),
        const SizedBox(height: 28),

        // 2. Bonding Tap Sequences
        _buildSectionHeader("Bonding Tap Sequences", Icons.sync_alt, Colors.pink.shade600),
        const SizedBox(height: 12),
        _buildRelationshipActionsCard(db, ble),
        const SizedBox(height: 28),

        // 3. Device Configuration
        _buildSectionHeader("Device Configuration", Icons.settings, Colors.purple.shade600),
        const SizedBox(height: 12),
        _buildChronosPanel(db, ble),
        const SizedBox(height: 28),
        _buildNotificationSyncPanel(db, ble),
        const SizedBox(height: 28),
        _buildHardwarePanel(db, ble),
        const SizedBox(height: 24),
      ],
    );
  }

  Widget _buildTapSequenceRow({
    required String actionLabel,
    required int selectedExpr,
    required int selectedSound,
    required bool targetHasSpeaker,
    required ValueChanged<int?> onExprChanged,
    required ValueChanged<int?> onSoundChanged,
  }) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8.0),
      child: Row(
        children: [
          Expanded(
            flex: 3,
            child: Text(
              actionLabel,
              style: GoogleFonts.outfit(color: textColor, fontWeight: FontWeight.bold, fontSize: 13),
            ),
          ),
          const SizedBox(width: 8),
          Expanded(
            flex: 4,
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 8),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(6),
                border: Border.all(color: Colors.black.withOpacity(0.06)),
              ),
              child: DropdownButtonHideUnderline(
                child: DropdownButton<int>(
                  value: (selectedExpr == 0 ||
                          selectedExpr == 1 ||
                          selectedExpr == 2 ||
                          selectedExpr == 3 ||
                          selectedExpr == 4 ||
                          selectedExpr == 5 ||
                          selectedExpr == 6 ||
                          (selectedExpr >= 100 && selectedExpr < 100 + DatabaseService.animMapping.length))
                      ? selectedExpr
                      : 1, // Fallback to Happy
                  dropdownColor: Colors.white,
                  style: GoogleFonts.outfit(color: textColor, fontSize: 12),
                  icon: const Icon(Icons.arrow_drop_down, color: textColor60, size: 16),
                  isExpanded: true,
                  items: [
                    const DropdownMenuItem(value: 0, child: Text("Idle/Blank")),
                    const DropdownMenuItem(value: 1, child: Text("Happy")),
                    const DropdownMenuItem(value: 2, child: Text("Sad")),
                    const DropdownMenuItem(value: 3, child: Text("Angry")),
                    const DropdownMenuItem(value: 4, child: Text("Surprised")),
                    const DropdownMenuItem(value: 5, child: Text("Sleeping")),
                    const DropdownMenuItem(value: 6, child: Text("Wink")),
                    ...DatabaseService.animMapping.keys.map((key) {
                      final idx = DatabaseService.animMapping.keys.toList().indexOf(key);
                      final label = DatabaseService.animMapping[key]!['label'] as String;
                      return DropdownMenuItem(
                        value: 100 + idx,
                        child: Text(label),
                      );
                    }).toList(),
                  ],
                  onChanged: onExprChanged,
                ),
              ),
            ),
          ),
          const SizedBox(width: 8),
          Expanded(
            flex: 4,
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 8),
              decoration: BoxDecoration(
                color: targetHasSpeaker ? Colors.white : Colors.grey.shade100,
                borderRadius: BorderRadius.circular(6),
                border: Border.all(color: Colors.black.withOpacity(0.06)),
              ),
              child: DropdownButtonHideUnderline(
                child: DropdownButton<int>(
                  value: targetHasSpeaker ? selectedSound : 0,
                  dropdownColor: Colors.white,
                  style: GoogleFonts.outfit(
                    color: targetHasSpeaker ? textColor : textColor38,
                    fontSize: 12,
                  ),
                  icon: Icon(
                    Icons.arrow_drop_down,
                    color: targetHasSpeaker ? textColor60 : textColor38,
                    size: 16,
                  ),
                  isExpanded: true,
                  items: const [
                    DropdownMenuItem(value: 0, child: Text("No Sound")),
                    DropdownMenuItem(value: 1, child: Text("Jump")),
                    DropdownMenuItem(value: 2, child: Text("Coin")),
                    DropdownMenuItem(value: 3, child: Text("Power Up")),
                    DropdownMenuItem(value: 4, child: Text("Power Down")),
                    DropdownMenuItem(value: 5, child: Text("Game Over")),
                    DropdownMenuItem(value: 6, child: Text("Chirp")),
                    DropdownMenuItem(value: 7, child: Text("Startup")),
                    DropdownMenuItem(value: 8, child: Text("Castle")),
                    DropdownMenuItem(value: 9, child: Text("Underworld")),
                    DropdownMenuItem(value: 10, child: Text("Theme Change")),
                  ],
                  onChanged: targetHasSpeaker ? onSoundChanged : null,
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildRelationshipActionsCard(DatabaseService db, BLEService ble) {
    final bool hasRelationship = db.primaryRobot != null && db.primaryRobot!.companionDeviceId != null;
    final primaryName = db.primaryRobot?.name ?? "Primary Robot";
    
    String companionName = "Companion";
    if (hasRelationship) {
      try {
        companionName = db.robots.firstWhere((r) => r.id == db.primaryRobot!.companionDeviceId).name;
      } catch (_) {}
    }

    final primaryHasSpeaker = ble.hasSpeaker;
    final companionHasSpeaker = ble.companionHasSpeaker;

    return GlassCard(
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if (!hasRelationship) ...[
            const Center(
              child: Icon(Icons.favorite_border, color: textColor38, size: 48),
            ),
            const SizedBox(height: 12),
            Text(
              "No relationship configured.\nPair two robots in Friends or Couple mode first to enable interactive tap triggers!",
              textAlign: TextAlign.center,
              style: GoogleFonts.outfit(color: textColor38, fontSize: 13),
            ),
          ] else ...[
            Text(
              "RELATIONSHIP COMMUNICATION STATUS",
              style: GoogleFonts.outfit(color: _accentColor, fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 0.8),
            ),
            const SizedBox(height: 6),
            Text(
              "Enable or disable interactive tap communication for each Luna robot device. If turned off, that robot will not send or react to any relationship triggers.",
              style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
            ),
            const SizedBox(height: 12),
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
              decoration: BoxDecoration(
                color: Colors.white.withOpacity(0.05),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: Colors.white.withOpacity(0.1)),
              ),
              child: Column(
                children: [
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Row(
                        children: [
                          Icon(Icons.wifi_tethering, color: ble.isPrimaryCommEnabled ? Colors.green : Colors.grey, size: 20),
                          const SizedBox(width: 8),
                          Text(
                            "$primaryName Communication",
                            style: GoogleFonts.outfit(color: textColor60, fontSize: 13, fontWeight: FontWeight.bold),
                          ),
                        ],
                      ),
                      Switch(
                        value: ble.isPrimaryCommEnabled,
                        activeColor: _accentColor,
                        onChanged: (val) {
                          ble.togglePrimaryComm(val);
                        },
                      ),
                    ],
                  ),
                  const Divider(color: Colors.white10, height: 1),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Row(
                        children: [
                          Icon(Icons.wifi_tethering, color: ble.isCompanionCommEnabled ? Colors.green : Colors.grey, size: 20),
                          const SizedBox(width: 8),
                          Text(
                            "$companionName Communication",
                            style: GoogleFonts.outfit(color: textColor60, fontSize: 13, fontWeight: FontWeight.bold),
                          ),
                        ],
                      ),
                      Switch(
                        value: ble.isCompanionCommEnabled,
                        activeColor: _accentColor,
                        onChanged: (val) {
                          ble.toggleCompanionComm(val);
                        },
                      ),
                    ],
                  ),
                ],
              ),
            ),
            const SizedBox(height: 24),
            const Divider(color: Colors.black12, height: 1),
            const SizedBox(height: 20),
            Text(
              "PRIMARY TAP SEQUENCE (WHEN $primaryName IS TAPPED)",
              style: GoogleFonts.outfit(color: _accentColor, fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 0.8),
            ),
            const SizedBox(height: 6),
            Text(
              "Configure how companion robot '$companionName' responds when you interact with '$primaryName'.",
              style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
            ),
            const SizedBox(height: 12),
            _buildTapSequenceRow(
              actionLabel: "Single Tap",
              selectedExpr: ble.relPrimaryTapExpr,
              selectedSound: ble.relPrimaryTapSound,
              targetHasSpeaker: companionHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: val,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: val,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
            ),
            _buildTapSequenceRow(
              actionLabel: "Double Tap",
              selectedExpr: ble.relPrimaryDoubleExpr,
              selectedSound: ble.relPrimaryDoubleSound,
              targetHasSpeaker: companionHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: val,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: val,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
            ),
            _buildTapSequenceRow(
              actionLabel: "Long Press",
              selectedExpr: ble.relPrimaryLongExpr,
              selectedSound: ble.relPrimaryLongSound,
              targetHasSpeaker: companionHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: val,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: val,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
            ),
            if (!companionHasSpeaker) ...[
              const SizedBox(height: 6),
              Row(
                children: [
                  const Icon(Icons.volume_mute, color: Colors.orange, size: 14),
                  const SizedBox(width: 6),
                  Expanded(
                    child: Text(
                      "Companion '$companionName' has no speaker (Luna v1). Sound triggers are disabled.",
                      style: GoogleFonts.outfit(color: Colors.orange.shade700, fontSize: 10, fontWeight: FontWeight.bold),
                    ),
                  ),
                ],
              ),
            ],
            const SizedBox(height: 24),
            const Divider(color: Colors.black12, height: 1),
            const SizedBox(height: 20),
            Text(
              "COMPANION TAP SEQUENCE (WHEN $companionName IS TAPPED)",
              style: GoogleFonts.outfit(color: const Color(0xFFEC4899), fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 0.8),
            ),
            const SizedBox(height: 6),
            Text(
              "Configure how primary robot '$primaryName' responds when you interact with '$companionName'.",
              style: GoogleFonts.outfit(color: textColor38, fontSize: 11),
            ),
            const SizedBox(height: 12),
            _buildTapSequenceRow(
              actionLabel: "Single Tap",
              selectedExpr: ble.relCompanionTapExpr,
              selectedSound: ble.relCompanionTapSound,
              targetHasSpeaker: primaryHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: val,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: val,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
            ),
            _buildTapSequenceRow(
              actionLabel: "Double Tap",
              selectedExpr: ble.relCompanionDoubleExpr,
              selectedSound: ble.relCompanionDoubleSound,
              targetHasSpeaker: primaryHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: val,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: val,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
            ),
            _buildTapSequenceRow(
              actionLabel: "Long Press",
              selectedExpr: ble.relCompanionLongExpr,
              selectedSound: ble.relCompanionLongSound,
              targetHasSpeaker: primaryHasSpeaker,
              onExprChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relPrimaryLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: val,
                    companionLongSound: ble.relCompanionLongSound,
                  );
                }
              },
              onSoundChanged: (val) {
                if (val != null) {
                  ble.saveRelationshipSettings(
                    primaryTapExpr: ble.relPrimaryTapExpr,
                    primaryTapSound: ble.relPrimaryTapSound,
                    primaryDoubleExpr: ble.relPrimaryDoubleExpr,
                    primaryDoubleSound: ble.relPrimaryDoubleSound,
                    primaryLongExpr: ble.relPrimaryLongExpr,
                    primaryLongSound: ble.relCompanionLongSound,
                    companionTapExpr: ble.relCompanionTapExpr,
                    companionTapSound: ble.relCompanionTapSound,
                    companionDoubleExpr: ble.relCompanionDoubleExpr,
                    companionDoubleSound: ble.relCompanionDoubleSound,
                    companionLongExpr: ble.relCompanionLongExpr,
                    companionLongSound: val,
                  );
                }
              },
            ),
            if (!primaryHasSpeaker) ...[
              const SizedBox(height: 6),
              Row(
                children: [
                  const Icon(Icons.volume_mute, color: Colors.orange, size: 14),
                  const SizedBox(width: 6),
                  Expanded(
                    child: Text(
                      "Primary '$primaryName' has no speaker (Luna v1). Sound triggers are disabled.",
                      style: GoogleFonts.outfit(color: Colors.orange.shade700, fontSize: 10, fontWeight: FontWeight.bold),
                    ),
                  ),
                ],
              ),
            ],
          ],
        ],
      ),
    );
  }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Animated GIF Card â€” each instance owns a GifController for looping playback
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class _GifCardWidget extends StatefulWidget {
  final GifModel gif;
  final DatabaseService db;
  final BLEService ble;
  final VoidCallback onTap;

  const _GifCardWidget({
    required this.gif,
    required this.db,
    required this.ble,
    required this.onTap,
  });

  @override
  State<_GifCardWidget> createState() => _GifCardWidgetState();
}

class _GifCardWidgetState extends State<_GifCardWidget>
    with SingleTickerProviderStateMixin {
  late GifController _controller;

  static const Color textColor38 = Color(0x61000000);

  @override
  void initState() {
    super.initState();
    _controller = GifController(vsync: this);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _showSoundSelectionDialog(BuildContext context, DatabaseService db, GifModel gif) {
    final sfxList = [
      {'id': 0, 'name': 'None (Muted)', 'color': Colors.grey},
      {'id': 1, 'name': 'Coin Collect', 'color': const Color(0xFFFFDC00)},
      {'id': 2, 'name': 'Super Mushroom', 'color': const Color(0xFF2ECC40)},
      {'id': 3, 'name': '1-Up Melody', 'color': const Color(0xFF0074D9)},
      {'id': 4, 'name': 'Stomp SFX', 'color': const Color(0xFFFFA500)},
      {'id': 5, 'name': 'Player Shrink', 'color': const Color(0xFFFF4136)},
      {'id': 6, 'name': 'Surprise Warp', 'color': const Color(0xFFE53935)},
      {'id': 8, 'name': 'Castle Theme', 'color': const Color(0xFFE11D48)},
      {'id': 9, 'name': 'Underworld Theme', 'color': const Color(0xFF7C3AED)},
      {'id': 10, 'name': 'Theme Toggle SFX', 'color': const Color(0xFF0EA5E9)},
    ];

    final mapping = DatabaseService.animMapping[gif.id] ?? {'sound': 0};
    final currentSoundId = gif.soundId ?? (mapping['sound'] as int? ?? 0);

    showDialog(
      context: context,
      builder: (BuildContext context) {
        return Theme(
          data: ThemeData.dark(),
          child: AlertDialog(
            backgroundColor: const Color(0xFF1E1D30),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(16),
              side: BorderSide(color: Colors.white.withOpacity(0.08)),
            ),
            title: Text(
              "Select SFX Soundtrack",
              style: GoogleFonts.outfit(
                color: Colors.white,
                fontWeight: FontWeight.bold,
                fontSize: 16,
              ),
            ),
            content: SizedBox(
              width: double.maxFinite,
              child: ListView.builder(
                shrinkWrap: true,
                itemCount: sfxList.length,
                itemBuilder: (context, index) {
                  final sfx = sfxList[index];
                  final sfxId = sfx['id'] as int;
                  final sfxName = sfx['name'] as String;
                  final sfxColor = sfx['color'] as Color;
                  final isSelected = currentSoundId == sfxId;

                  return Container(
                    margin: const EdgeInsets.symmetric(vertical: 4),
                    decoration: BoxDecoration(
                      color: isSelected ? sfxColor.withOpacity(0.12) : const Color(0x0AFFFFFF),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(
                        color: isSelected ? sfxColor : Colors.transparent,
                        width: 1.5,
                      ),
                    ),
                    child: ListTile(
                      dense: true,
                      leading: Icon(
                        sfxId == 0 ? Icons.volume_mute : Icons.music_note,
                        color: sfxColor,
                        size: 16,
                      ),
                      title: Text(
                        sfxName,
                        style: GoogleFonts.outfit(
                          color: Colors.white,
                          fontSize: 13,
                          fontWeight: isSelected ? FontWeight.bold : FontWeight.normal,
                        ),
                      ),
                      trailing: isSelected
                          ? Icon(Icons.check_circle, color: sfxColor, size: 16)
                          : null,
                      onTap: () {
                        db.updateGifSound(gif.id, sfxId);
                        Navigator.of(context).pop();
                      },
                    ),
                  );
                },
              ),
            ),
            actions: [
              TextButton(
                onPressed: () => Navigator.of(context).pop(),
                child: Text(
                  "Close",
                  style: GoogleFonts.outfit(color: Colors.white70),
                ),
              ),
            ],
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final gif = widget.gif;
    final db = widget.db;
    final isFav = gif.favorite;
    final isSelected = gif.selected;
    final isHidden = gif.hidden;
    final isMiss = db.primaryRobot?.variant == 'ms_luna';
    final previewColor =
        isMiss ? const Color(0xFFEC4899) : const Color(0xFF00F0FF);

    Widget imageWidget;
    if (gif.customData != null && gif.customData!.isNotEmpty) {
      try {
        final rawBytes = base64Decode(gif.customData!.split(',').last);
        imageWidget = Gif(
          image: MemoryImage(rawBytes),
          controller: _controller,
          autostart: Autostart.loop,
          fit: BoxFit.cover,
        );
      } catch (_) {
        imageWidget = const Icon(Icons.broken_image, color: Colors.red);
      }
    } else {
      imageWidget = Gif(
        image: AssetImage('assets/animations/${gif.id}.gif'),
        controller: _controller,
        autostart: Autostart.loop,
        fit: BoxFit.cover,
      );
    }

    return GestureDetector(
      behavior: HitTestBehavior.opaque,
      onTap: widget.onTap,
      child: Container(
        decoration: BoxDecoration(
          color: const Color(0x66161526),
          border: Border.all(color: Colors.white.withOpacity(0.06)),
          borderRadius: BorderRadius.circular(12),
        ),
        child: ClipRRect(
          borderRadius: BorderRadius.circular(12),
          child: Stack(
            children: [
              // Main content Column (GIF top + text/actions bottom)
              Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Expanded(
                    child: Container(
                      color: Colors.black,
                      child: ClipRect(
                        child: ColorFiltered(
                          colorFilter:
                              ColorFilter.mode(previewColor, BlendMode.modulate),
                          child: Opacity(
                            opacity: isHidden ? 0.3 : 1.0,
                            child: SizedBox.expand(
                              child: imageWidget,
                            ),
                          ),
                        ),
                      ),
                    ),
                  ),
                  Container(
                    height: 36,
                    alignment: Alignment.centerLeft,
                    padding: const EdgeInsets.symmetric(horizontal: 8),
                    child: Row(
                      children: [
                        Expanded(
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
                        const SizedBox(width: 4),
                        // Action buttons (visibility, delete)
                        GestureDetector(
                          onTap: () => db.toggleHidden(gif.id),
                          child: Padding(
                            padding: const EdgeInsets.all(2),
                            child: Icon(
                              isHidden ? Icons.visibility_off : Icons.visibility,
                              color: Colors.white60,
                              size: 12,
                            ),
                          ),
                        ),
                        const SizedBox(width: 4),
                        GestureDetector(
                          onTap: () => _showSoundSelectionDialog(context, db, gif),
                          child: Padding(
                            padding: const EdgeInsets.all(2),
                            child: Icon(
                              gif.soundId != null && gif.soundId! > 0
                                  ? Icons.volume_up
                                  : Icons.volume_mute,
                              color: gif.soundId != null && gif.soundId! > 0
                                  ? const Color(0xFF00F0FF)
                                  : Colors.white38,
                              size: 12,
                            ),
                          ),
                        ),
                        if (!DatabaseService.animMapping.containsKey(gif.id)) ...[
                          const SizedBox(width: 4),
                          GestureDetector(
                            onTap: () => db.deleteCustomGif(gif.id),
                            child: const Padding(
                              padding: EdgeInsets.all(2),
                              child: Icon(Icons.delete,
                                  color: Colors.redAccent, size: 12),
                            ),
                          ),
                        ],
                      ],
                    ),
                  ),
                ],
              ),

              // Select Checkbox Top Left (drawn over the GIF)
              Positioned(
                top: 2,
                left: 2,
                child: SizedBox(
                  width: 24,
                  height: 24,
                  child: Checkbox(
                    value: isSelected,
                    activeColor: const Color(0xFFA855F7),
                    onChanged: (_) => db.toggleSelected(gif.id),
                  ),
                ),
              ),

              // Favorite Star Top Right (drawn over the GIF)
              Positioned(
                top: 4,
                right: 4,
                child: GestureDetector(
                  onTap: () => db.toggleFavorite(gif.id),
                  child: Icon(
                    isFav ? Icons.star : Icons.star_border,
                    color: isFav ? const Color(0xFFFFDC00) : Colors.white38,
                    size: 16,
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class ButtonStripePainter extends CustomPainter {
  final Color color;
  final double stripeWidth;
  final double gapWidth;

  ButtonStripePainter({
    required this.color,
    this.stripeWidth = 3,
    this.gapWidth = 6,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = color
      ..strokeWidth = stripeWidth
      ..style = PaintingStyle.stroke;

    final double step = stripeWidth + gapWidth;
    for (double i = -size.height; i < size.width; i += step) {
      canvas.drawLine(
        Offset(i, 0),
        Offset(i + size.height, size.height),
        paint,
      );
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

class WallpaperCropDialog extends StatefulWidget {
  final Uint8List imageBytes;
  final int targetW;
  final int targetH;

  const WallpaperCropDialog({
    Key? key,
    required this.imageBytes,
    required this.targetW,
    required this.targetH,
  }) : super(key: key);

  @override
  _WallpaperCropDialogState createState() => _WallpaperCropDialogState();
}

class _WallpaperCropDialogState extends State<WallpaperCropDialog> {
  ui.Image? _decodedImage;
  final TransformationController _transformationController = TransformationController();
  bool _initialized = false;
  double _viewportW = 0.0;
  double _viewportH = 0.0;

  @override
  void initState() {
    super.initState();
    _decodeImage();
  }

  Future<void> _decodeImage() async {
    final codec = await ui.instantiateImageCodec(widget.imageBytes);
    final frameInfo = await codec.getNextFrame();
    setState(() {
      _decodedImage = frameInfo.image;
    });
  }

  void _initializeMatrix(double containerWidth, double containerHeight) {
    if (_decodedImage == null || _initialized) return;

    // Viewport size matches target aspect ratio, fitting inside the screen container
    final double targetAspect = widget.targetW / widget.targetH;
    final double containerAspect = containerWidth / containerHeight;

    if (containerAspect > targetAspect) {
      _viewportH = containerHeight * 0.8;
      _viewportW = _viewportH * targetAspect;
    } else {
      _viewportW = containerWidth * 0.8;
      _viewportH = _viewportW / targetAspect;
    }

    // Set initial scale to fill the crop window
    final double imgW = _decodedImage!.width.toDouble();
    final double imgH = _decodedImage!.height.toDouble();
    final double scale = (_viewportW / imgW > _viewportH / imgH) ? (_viewportW / imgW) : (_viewportH / imgH);

    final double tx = (_viewportW - imgW * scale) / 2;
    final double ty = (_viewportH - imgH * scale) / 2;

    _transformationController.value = Matrix4.identity()
      ..translate(tx, ty)
      ..scale(scale);

    _initialized = true;
  }

  Future<Uint8List> _convertImageToRGB565(ui.Image image) async {
    final byteData = await image.toByteData(format: ui.ImageByteFormat.rawRgba);
    if (byteData == null) throw Exception("Failed to get raw RGBA bytes");
    
    final width = image.width;
    final height = image.height;
    final rgbaBytes = byteData.buffer.asUint8List();
    
    final rgb565Bytes = Uint8List(width * height * 2);
    final rgb565Data = ByteData.view(rgb565Bytes.buffer);
    
    int srcIdx = 0;
    int dstIdx = 0;
    for (int i = 0; i < width * height; i++) {
      final r = rgbaBytes[srcIdx];
      final g = rgbaBytes[srcIdx + 1];
      final b = rgbaBytes[srcIdx + 2];
      
      final r5 = (r >> 3) & 0x1F;
      final g6 = (g >> 2) & 0x3F;
      final b5 = (b >> 3) & 0x1F;
      
      final rgb565 = (r5 << 11) | (g6 << 5) | b5;
      rgb565Data.setUint16(dstIdx, rgb565, Endian.little);
      
      srcIdx += 4;
      dstIdx += 2;
    }
    return rgb565Bytes;
  }

  @override
  Widget build(BuildContext context) {
    if (_decodedImage == null) {
      return const Center(child: CircularProgressIndicator());
    }

    return Dialog(
      backgroundColor: Colors.transparent,
      insetPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 24),
      child: Container(
        decoration: BoxDecoration(
          color: const Color(0xFF0F172A), // Slate 900
          borderRadius: BorderRadius.circular(16),
          border: Border.all(color: Colors.white24),
        ),
        padding: const EdgeInsets.all(16),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text(
              "Adjust Wallpaper",
              style: GoogleFonts.outfit(
                color: Colors.white,
                fontSize: 18,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              "Drag to pan. Pinch with two fingers to zoom.",
              style: GoogleFonts.outfit(
                color: Colors.white70,
                fontSize: 12,
              ),
            ),
            const SizedBox(height: 16),
            // Crop Window Container
            SizedBox(
              height: 300,
              child: LayoutBuilder(
                builder: (context, constraints) {
                  final double width = constraints.maxWidth;
                  final double height = constraints.maxHeight;
                  
                  _initializeMatrix(width, height);

                  return Center(
                    child: Container(
                      width: _viewportW,
                      height: _viewportH,
                      decoration: BoxDecoration(
                        border: Border.all(color: const Color(0xFF00F0FF), width: 2), // Neon cyan border
                        borderRadius: BorderRadius.circular(4),
                      ),
                      child: ClipRect(
                        child: InteractiveViewer(
                          transformationController: _transformationController,
                          minScale: 0.05,
                          maxScale: 10.0,
                          boundaryMargin: const EdgeInsets.all(1000),
                          child: SizedBox(
                            width: _decodedImage!.width.toDouble(),
                            height: _decodedImage!.height.toDouble(),
                            child: RawImage(
                              image: _decodedImage,
                              fit: BoxFit.fill,
                            ),
                          ),
                        ),
                      ),
                    ),
                  );
                },
              ),
            ),
            const SizedBox(height: 24),
            Row(
              mainAxisAlignment: MainAxisAlignment.end,
              children: [
                TextButton(
                  onPressed: () => Navigator.of(context).pop(),
                  child: Text("CANCEL", style: GoogleFonts.outfit(color: Colors.white70)),
                ),
                const SizedBox(width: 8),
                ElevatedButton(
                  onPressed: () async {
                    // Extract cropped image bytes!
                    final matrix = _transformationController.value;
                    final double scale = matrix.storage[0];
                    final double tx = matrix.storage[12];
                    final double ty = matrix.storage[13];

                    final double cx1 = -tx / scale;
                    final double cy1 = -ty / scale;
                    final double cx2 = (_viewportW - tx) / scale;
                    final double cy2 = (_viewportH - ty) / scale;

                    final recorder = ui.PictureRecorder();
                    final canvas = ui.Canvas(recorder);
                    final paint = ui.Paint()..filterQuality = ui.FilterQuality.high;

                    canvas.drawImageRect(
                      _decodedImage!,
                      ui.Rect.fromLTRB(cx1, cy1, cx2, cy2),
                      ui.Rect.fromLTWH(0, 0, widget.targetW.toDouble(), widget.targetH.toDouble()),
                      paint,
                    );

                    final picture = recorder.endRecording();
                    final croppedUiImage = await picture.toImage(widget.targetW, widget.targetH);
                    final bytes = await _convertImageToRGB565(croppedUiImage);

                    Navigator.of(context).pop(bytes);
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFF00F0FF),
                    foregroundColor: Colors.black,
                  ),
                  child: Text("CROP & SAVE", style: GoogleFonts.outfit(fontWeight: FontWeight.bold)),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class MapPreviewPainter extends CustomPainter {
  final bool isStreaming;
  MapPreviewPainter({required this.isStreaming});

  @override
  void paint(Canvas canvas, Size size) {
    final bgPaint = Paint()..color = const Color(0xFF1E293B);
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height), bgPaint);

    final gridPaint = Paint()
      ..color = Colors.white.withOpacity(0.08)
      ..strokeWidth = 1.0;

    for (double x = 0; x < size.width; x += 24) {
      canvas.drawLine(Offset(x, 0), Offset(x, size.height), gridPaint);
    }
    for (double y = 0; y < size.height; y += 24) {
      canvas.drawLine(Offset(0, y), Offset(size.width, y), gridPaint);
    }

    // Draw route path
    final routePaint = Paint()
      ..color = const Color(0xFF4285F4)
      ..strokeWidth = 6.0
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round;

    final path = Path()
      ..moveTo(size.width * 0.2, size.height * 0.8)
      ..lineTo(size.width * 0.4, size.height * 0.4)
      ..lineTo(size.width * 0.7, size.height * 0.3);

    canvas.drawPath(path, routePaint);

    // Location Marker
    final pinPaint = Paint()..color = const Color(0xFFEA4335);
    canvas.drawCircle(Offset(size.width * 0.7, size.height * 0.3), 8, pinPaint);

    // Navigation Arrow
    final arrowPaint = Paint()..color = const Color(0xFF34A853);
    canvas.drawCircle(Offset(size.width * 0.2, size.height * 0.8), 10, arrowPaint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}

