import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:http/http.dart' as http;
import 'database_service.dart';
import '../models/calendar_event.dart';

class BLEService with ChangeNotifier {
  static const String serviceUuid = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
  static const String expressionCharUuid = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
  static const String audioCharUuid = 'd90e0c03-51ee-4c31-893c-cf572db85700';
  static const String textCharUuid = 'c8a00d04-62ff-4b32-843d-0f1c6db8a101';
  static const String statusCharUuid = 'fb2f0e05-73ee-4f32-833d-1f2c6db8a102';
  static const String audioStreamCharUuid = 'a823e50b-71ee-48c5-9276-2e8c6db8a103';
  static const String imageCharUuid = 'e1234501-1fb5-459e-8fcc-c5c9c331914b';

  BluetoothDevice? _connectedDevice;
  bool _isConnected = false;
  bool _isScanning = false;
  List<ScanResult> _scanResults = [];
  
  // Reconnection and pairing states
  String? _pairedDeviceId;
  bool _isConnecting = false;
  Timer? _reconnectTimer;
  int _lastReconnectLogTime = 0;
  bool _isReconnecting = false;
  int _lastScanTime = 0;

  // Subscriptions
  StreamSubscription? _scanSub;
  StreamSubscription? _connectionStateSub;
  StreamSubscription? _statusNotificationSub;
  StreamSubscription? _audioStreamNotificationSub;

  // Audio Stream controller
  final StreamController<List<int>> _audioStreamController = StreamController<List<int>>.broadcast();
  Stream<List<int>> get audioStreamData => _audioStreamController.stream;

  // Characteristics
  BluetoothCharacteristic? _exprChar;
  BluetoothCharacteristic? _audioChar;
  BluetoothCharacteristic? _textChar;
  BluetoothCharacteristic? _statusChar;
  BluetoothCharacteristic? _audioStreamChar;
  BluetoothCharacteristic? _imageChar;

  // Diagnostics variables
  int _uptimeSeconds = 0;
  int _touchCount = 0;
  double _batteryVoltage = 0.0;
  int _activeExpressionId = 0;
  String _activeExpressionLabel = "IDLE";
  String _activeScreenMode = "FACE";

  // Hardware Stored Events & Reminders
  List<CalendarEvent> _hardwareEvents = [];
  bool _isHardwareEventsSynced = false;

  List<CalendarEvent> get hardwareEvents => _hardwareEvents;
  bool get isHardwareEventsSynced => _isHardwareEventsSynced;

  Function(List<CalendarEvent>)? onHardwareEventsSynced;
  Function(String id, String title)? onHardwareAlarmRinging;
  VoidCallback? onHardwareAlarmDismissed;

  // Live GPS Navigation tracking
  String _navDirection = "STRAIGHT";
  String _navDistance = "--";
  String _navDescription = "";
  String _navRoad = "";
  String _navTotalTime = "";
  String _navTotalDist = "";
  String _navEta = "";
  bool _isNavActive = false;

  // Companion Device Properties
  BluetoothDevice? _companionDevice;
  bool _isCompanionConnected = false;
  bool _isCompanionConnecting = false;

  BluetoothCharacteristic? _companionExprChar;
  BluetoothCharacteristic? _companionAudioChar;
  BluetoothCharacteristic? _companionTextChar;
  BluetoothCharacteristic? _companionStatusChar;
  BluetoothCharacteristic? _companionAudioStreamChar;

  StreamSubscription? _companionConnectionStateSub;
  StreamSubscription? _companionStatusNotificationSub;

  int _companionUptimeSeconds = 0;
  int _companionTouchCount = 0;
  double _companionBatteryVoltage = 0.0;
  int _companionActiveExpressionId = 0;
  String _companionActiveExpressionLabel = "IDLE";

  // Relationship Configurable Actions
  int relPrimaryTapExpr = 1; // Default Happy
  int relPrimaryTapSound = 2; // Default Coin
  int relPrimaryDoubleExpr = 6; // Default Wink
  int relPrimaryDoubleSound = 6; // Default Jump
  int relPrimaryLongExpr = 5; // Default Sleeping
  int relPrimaryLongSound = 4; // Default Powerdown

  int relCompanionTapExpr = 1;
  int relCompanionTapSound = 2;
  int relCompanionDoubleExpr = 6;
  int relCompanionDoubleSound = 6;
  int relCompanionLongExpr = 5;
  int relCompanionLongSound = 4;

  bool isPrimaryCommEnabled = true;
  bool isCompanionCommEnabled = true;
  void Function(String eventType, int expr, int sound)? onPrimaryTouchTriggered;
  void Function(int clockStyle, int brightness, bool negative, bool silent)? onSettingsSyncedFromWatch;

  // Console log entries
  final List<String> _consoleLogs = [];

  // Robot events stream for dismissals and other custom messages
  final StreamController<String> _robotEventsController = StreamController<String>.broadcast();
  Stream<String> get robotEvents => _robotEventsController.stream;

  // Server IP & Wi-Fi control helper variables
  String _serverIp = 'localhost';

  int _nextMsgId = 1;
  final Map<String, _PendingAck> _pendingAcks = {};

  // Getters
  BluetoothDevice? get connectedDevice => _connectedDevice;
  bool get isConnected => _isConnected;
  bool get isScanning => _isScanning;
  List<ScanResult> get scanResults => _scanResults;
  String? get pairedDeviceId => _pairedDeviceId;
  int get uptimeSeconds => _uptimeSeconds;
  int get touchCount => _touchCount;
  double get batteryVoltage => _batteryVoltage;
  int get batteryPercentage {
    if (!_isConnected || _batteryVoltage <= 0.0) return 0;
    double pct = 0.0;
    final v = _batteryVoltage;
    if (v >= 4.15) {
      pct = 100.0;
    } else if (v >= 4.05) {
      pct = 90.0 + (v - 4.05) * 100.0;
    } else if (v >= 3.95) {
      pct = 80.0 + (v - 3.95) * 100.0;
    } else if (v >= 3.87) {
      pct = 70.0 + (v - 3.87) * 125.0;
    } else if (v >= 3.82) {
      pct = 60.0 + (v - 3.82) * 200.0;
    } else if (v >= 3.79) {
      pct = 50.0 + (v - 3.79) * 333.0;
    } else if (v >= 3.75) {
      pct = 40.0 + (v - 3.75) * 250.0;
    } else if (v >= 3.72) {
      pct = 30.0 + (v - 3.72) * 333.0;
    } else if (v >= 3.68) {
      pct = 20.0 + (v - 3.68) * 250.0;
    } else if (v >= 3.60) {
      pct = 10.0 + (v - 3.60) * 125.0;
    } else if (v >= 3.30) {
      pct = (v - 3.30) * 33.3;
    } else {
      pct = 0.0;
    }
    return pct.clamp(0.0, 100.0).toInt();
  }

  int get activeExpressionId => _activeExpressionId;
  String get activeExpressionLabel => _activeExpressionLabel;
  String get activeScreenMode => _activeScreenMode;
  String get navDirection => _navDirection;
  String get navDistance => _navDistance;
  String get navDescription => _navDescription;
  String get navRoad => _navRoad;
  String get navTotalTime => _navTotalTime;
  String get navTotalDist => _navTotalDist;
  String get navEta => _navEta;
  bool get isNavActive => _isNavActive;
  List<String> get consoleLogs => _consoleLogs;
  String get serverIp => _serverIp;
  bool get hasSpeaker => false;
  String get hardwareVersionString => !_isConnected ? "Disconnected" : "Luna v1 (No Speaker)";

  // Companion Getters
  BluetoothDevice? get companionDevice => _companionDevice;
  bool get isCompanionConnected => _isCompanionConnected;
  bool get isCompanionConnecting => _isCompanionConnecting;
  int get companionUptimeSeconds => _companionUptimeSeconds;
  int get companionTouchCount => _companionTouchCount;
  double get companionBatteryVoltage => _companionBatteryVoltage;
  int get companionBatteryPercentage {
    if (!_isCompanionConnected || _companionBatteryVoltage <= 0.0) return 0;
    double pct = 0.0;
    final v = _companionBatteryVoltage;
    if (v >= 4.15) {
      pct = 100.0;
    } else if (v >= 4.05) {
      pct = 90.0 + (v - 4.05) * 100.0;
    } else if (v >= 3.95) {
      pct = 80.0 + (v - 3.95) * 100.0;
    } else if (v >= 3.87) {
      pct = 70.0 + (v - 3.87) * 125.0;
    } else if (v >= 3.82) {
      pct = 60.0 + (v - 3.82) * 200.0;
    } else if (v >= 3.79) {
      pct = 50.0 + (v - 3.79) * 333.0;
    } else if (v >= 3.75) {
      pct = 40.0 + (v - 3.75) * 250.0;
    } else if (v >= 3.72) {
      pct = 30.0 + (v - 3.72) * 333.0;
    } else if (v >= 3.68) {
      pct = 20.0 + (v - 3.68) * 250.0;
    } else if (v >= 3.60) {
      pct = 10.0 + (v - 3.60) * 125.0;
    } else if (v >= 3.30) {
      pct = (v - 3.30) * 33.3;
    } else {
      pct = 0.0;
    }
    return pct.clamp(0.0, 100.0).toInt();
  }
  int get companionActiveExpressionId => _companionActiveExpressionId;
  String get companionActiveExpressionLabel => _companionActiveExpressionLabel;
  bool get companionHasSpeaker => false;
  String get companionHardwareVersionString => !_isCompanionConnected ? "Disconnected" : "Luna v1 (No Speaker)";

  void setServerIp(String ip) {
    if (_serverIp != ip) {
      _serverIp = ip;
      notifyListeners();
    }
  }

  Future<bool> _transmitWifiCommand(String command) async {
    if (_pairedDeviceId == null) return false;
    final cleanMac = _pairedDeviceId!.replaceAll(':', '').toUpperCase();
    try {
      final url = Uri.parse('http://$_serverIp:8000/api/trigger');
      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'mac': cleanMac,
          'command': command,
        }),
      ).timeout(const Duration(seconds: 3));
      
      if (response.statusCode == 200) {
        addLog("Transmitted via Wi-Fi: '$command'", "WIFI_CTRL");
        return true;
      } else {
        addLog("Wi-Fi transmit returned status ${response.statusCode}", "ERROR");
        return false;
      }
    } catch (e) {
      addLog("Wi-Fi transmission failed: $e", "ERROR");
      return false;
    }
  }

  Future<bool> transmitAudioChunk(List<int> chunk) async {
    if (_audioStreamChar == null) return false;
    try {
      const int maxPacket = 240;
      int offset = 0;
      while (offset < chunk.length) {
        final end = (offset + maxPacket < chunk.length) ? offset + maxPacket : chunk.length;
        final sub = chunk.sublist(offset, end);
        await _audioStreamChar!.write(
          sub is Uint8List ? sub : Uint8List.fromList(sub),
          withoutResponse: true,
        );
        offset = end;
      }
      return true;
    } catch (_) {
      return false;
    }
  }

  BLEService() {
    _initBLE();
    _loadPairedDevice().then((_) {
      triggerReconnection();
    });
    _startReconnectTimer();
    loadRelationshipSettings();
    
    // Periodically sync connection status with background service
    Timer.periodic(const Duration(seconds: 5), (timer) {
      const MethodChannel('com.mrmsluna/notifications').invokeMethod('updateConnectionStatus', {'connected': _isConnected});
    });
  }

  void _initBLE() {
    FlutterBluePlus.isScanning.listen((scanning) {
      _isScanning = scanning;
      notifyListeners();
    });
  }

  Future<void> _loadPairedDevice() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      _pairedDeviceId = prefs.getString("paired_device_id");
      if (_pairedDeviceId != null) {
        addLog("Found paired robot ID: $_pairedDeviceId. Starting background reconnection...", "BLE");
      }
    } catch (e) {
      print("Failed to load paired device ID: $e");
    }
  }

  Future<void> triggerReconnection() async {
    if (_pairedDeviceId == null || _isConnected || _isConnecting || _isScanning || _isReconnecting) {
      return;
    }
    
    final nowMs = DateTime.now().millisecondsSinceEpoch;
    if (nowMs - _lastScanTime < 15000) {
      return;
    }

    _isReconnecting = true;
    try {
      if (await FlutterBluePlus.adapterState.first != BluetoothAdapterState.on) {
        _isReconnecting = false;
        return;
      }
      addLog("Starting fast reconnection flow...", "BLE");
      // 1. Check system devices
      List<BluetoothDevice> systemDevices = await FlutterBluePlus.systemDevices([Guid(serviceUuid)]);
      for (var d in systemDevices) {
        if (d.remoteId.str == _pairedDeviceId) {
          addLog("Found in system cache during fast reconnect. Connecting...", "BLE");
          await connect(d);
          _isReconnecting = false;
          return;
        }
      }

      // 2. Scan immediately
      _isScanning = true;
      _lastScanTime = nowMs;
      notifyListeners();

      StreamSubscription? tempSub;
      tempSub = FlutterBluePlus.scanResults.listen((results) async {
        for (var r in results) {
          if (r.device.remoteId.str == _pairedDeviceId) {
            addLog("Paired robot located during fast scan! Connecting...", "BLE");
            tempSub?.cancel();
            await FlutterBluePlus.stopScan();
            _isScanning = false;
            await connect(r.device);
            break;
          }
        }
      });

      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 4),
        androidUsesFineLocation: true,
      );
      await Future.delayed(const Duration(seconds: 4));
      tempSub?.cancel();
    } catch (e) {
      print("Fast reconnect failed: $e");
    } finally {
      _isScanning = false;
      _isReconnecting = false;
      notifyListeners();
    }
  }

  void _startReconnectTimer() {
    _reconnectTimer?.cancel();
    _reconnectTimer = Timer.periodic(const Duration(seconds: 5), (timer) async {
      // Proactively verify connection state to prevent getting stuck in a fake connected state
      if (_isConnected && _connectedDevice != null) {
        try {
          final state = await _connectedDevice!.connectionState.first.timeout(const Duration(milliseconds: 500));
          if (state != BluetoothConnectionState.connected) {
            addLog("Device connection verified as LOST. Cleaning up state.", "BLE");
            _handleDisconnect();
          }
        } catch (e) {
          addLog("Connection verification failed: $e. Cleaning up state.", "WARNING");
          _handleDisconnect();
        }
      }

      if (_pairedDeviceId == null || _isConnected || _isConnecting || _isScanning || _isReconnecting) {
        return;
      }
      
      final nowMs = DateTime.now().millisecondsSinceEpoch;
      if (nowMs - _lastScanTime < 15000) {
        return;
      }
      
      _isReconnecting = true;
      try {
        if (await FlutterBluePlus.adapterState.first == BluetoothAdapterState.on) {
          final logNow = DateTime.now().millisecondsSinceEpoch;
          if (logNow - _lastReconnectLogTime > 15000) {
            _lastReconnectLogTime = logNow;
            addLog("Checking connection status for paired robot...", "BLE");
          }

          // 1. Try to find in system/cached devices first
          List<BluetoothDevice> systemDevices = await FlutterBluePlus.systemDevices([Guid(serviceUuid)]);
          BluetoothDevice? targetDevice;
          for (var d in systemDevices) {
            if (d.remoteId.str == _pairedDeviceId) {
              targetDevice = d;
              break;
            }
          }

          if (targetDevice != null) {
            _lastReconnectLogTime = logNow;
            addLog("Paired robot found in system cache. Connecting...", "BLE");
            await connect(targetDevice);
            _isReconnecting = false;
            return;
          }

          // 2. Perform a brief scan to discover if advertising
          _lastReconnectLogTime = logNow;
          addLog("Scanning to locate paired robot...", "BLE");
          _isScanning = true;
          _lastScanTime = nowMs;
          notifyListeners();

          StreamSubscription? tempSub;
          tempSub = FlutterBluePlus.scanResults.listen((results) async {
            for (var r in results) {
              if (r.device.remoteId.str == _pairedDeviceId) {
                addLog("Paired robot located! Connecting...", "BLE");
                tempSub?.cancel();
                await FlutterBluePlus.stopScan();
                _isScanning = false;
                await connect(r.device);
                break;
              }
            }
          });

          try {
            await FlutterBluePlus.startScan(
              timeout: const Duration(seconds: 4),
              androidUsesFineLocation: true,
            );
          } catch (e) {
            print("Auto-reconnect scan failed: $e");
          }

          await Future.delayed(const Duration(seconds: 4));
          tempSub?.cancel();
        }
      } catch (e) {
        print("Error during reconnection tick: $e");
      } finally {
        _isScanning = false;
        _isReconnecting = false;
        notifyListeners();
      }
    });
  }

  void addLog(String message, String type) {
    final timestamp = DateTime.now().toLocal().toString().split(' ')[1].substring(0, 8);
    print("[$type] $message");
    _consoleLogs.add('[$timestamp] [$type] $message');
    if (_consoleLogs.length > 150) {
      _consoleLogs.removeAt(0);
    }
    notifyListeners();
  }

  void clearLogs() {
    _consoleLogs.clear();
    notifyListeners();
  }

  Future<bool> _requestPermissions() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();

    bool scanGranted = statuses[Permission.bluetoothScan]?.isGranted ?? false;
    bool connectGranted = statuses[Permission.bluetoothConnect]?.isGranted ?? false;
    bool locationGranted = statuses[Permission.location]?.isGranted ?? false;

    if (!scanGranted || !connectGranted || !locationGranted) {
      addLog("Permissions not granted: SCAN=$scanGranted, CONNECT=$connectGranted, LOCATION=$locationGranted", "ERROR");
      return false;
    }
    return true;
  }

  Future<void> startScan() async {
    if (_isScanning) return;
    _scanResults.clear();
    addLog("Scanning for Mr.&Ms Luna companion robot...", "BLE");
    notifyListeners();

    bool hasPermissions = await _requestPermissions();
    if (!hasPermissions) {
      _isScanning = false;
      notifyListeners();
      return;
    }

    try {
      _scanSub = FlutterBluePlus.scanResults.listen((results) {
        _scanResults = results.where((r) => r.device.platformName.contains('Luna') || r.advertisementData.serviceUuids.contains(Guid(serviceUuid))).toList();
        notifyListeners();
      });

      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 10),
      );
    } catch (e) {
      addLog("Scan failed: $e", "ERROR");
    }
  }

  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
    _scanSub?.cancel();
    _isScanning = false;
    notifyListeners();
  }

  Future<void> connect(BluetoothDevice device) async {
    if (_isConnecting) return;
    _isConnecting = true;
    notifyListeners();

    await stopScan();
    
    PermissionStatus status = await Permission.bluetoothConnect.request();
    if (!status.isGranted) {
      addLog("Bluetooth Connect permission denied.", "ERROR");
      _isConnecting = false;
      _handleDisconnect();
      return;
    }

    try {
      // Check if already connected
      final startState = await device.connectionState.first;
      if (startState == BluetoothConnectionState.connected) {
        _handleConnected(device);
        return;
      }

      _connectionStateSub?.cancel();
      _connectionStateSub = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.connected) {
          _handleConnected(device);
        } else if (state == BluetoothConnectionState.disconnected) {
          _handleDisconnect();
        }
      });

      // Connect directly with shorter timeout (5 seconds)
      await device.connect(autoConnect: false, timeout: const Duration(seconds: 5));

      // Synchronously verify if it's already connected (or completed immediately)
      final currentState = await device.connectionState.first.timeout(
        const Duration(milliseconds: 500),
        onTimeout: () => BluetoothConnectionState.disconnected,
      );
      if (currentState == BluetoothConnectionState.connected) {
        _handleConnected(device);
      }
    } catch (e) {
      addLog("Connection attempt failed: $e", "WARNING");
      _isConnecting = false;
      _handleDisconnect();
    }
  }

  Future<void> connectById(String deviceId) async {
    if (_isConnecting) return;
    try {
      addLog("Connecting directly to paired ID: $deviceId...", "BLE");
      final device = BluetoothDevice.fromId(deviceId);
      await connect(device);
    } catch (e) {
      addLog("Direct connection failed: $e", "ERROR");
      _isConnecting = false;
      _handleDisconnect();
    }
  }

  void _handleConnected(BluetoothDevice device) {
    if (_isConnected) return;
    _isConnected = true;
    _isConnecting = false;
    _connectedDevice = device;
    _pairedDeviceId = device.remoteId.str;
    SharedPreferences.getInstance().then((prefs) {
      prefs.setString("paired_device_id", _pairedDeviceId!);
    });
    addLog("Connected to Mr.&Ms Luna successfully!", "BLE");
    _setupServices(device);
    
    // Request high connection priority for faster communication and reconnection stability
    try {
      device.requestConnectionPriority(connectionPriorityRequest: ConnectionPriority.high);
    } catch (e) {
      addLog("Failed to request high connection priority: $e", "WARNING");
    }
    
    // Request high MTU size for high audio throughput
    try {
      device.requestMtu(512);
    } catch (e) {
      addLog("Failed to request MTU 512: $e", "WARNING");
    }
    
    // Update background service immediately
    const MethodChannel('com.mrmsluna/notifications').invokeMethod('updateConnectionStatus', {'connected': true});
    
    notifyListeners();
  }

  Future<void> _setupServices(BluetoothDevice device) async {
    try {
      addLog("Waiting for BLE connection to settle...", "BLE");
      await Future.delayed(const Duration(milliseconds: 1000));

      List<BluetoothService> services = [];
      int retries = 3;
      while (retries > 0) {
        try {
          services = await device.discoverServices();
          break;
        } catch (e) {
          retries--;
          addLog("Discover services failed: $e. Retries left: $retries", "WARNING");
          if (retries == 0) rethrow;
          await Future.delayed(const Duration(milliseconds: 1000));
        }
      }

      BluetoothService? targetService = services.firstWhere(
        (s) => s.uuid == Guid(serviceUuid),
      );

      for (var c in targetService.characteristics) {
        if (c.uuid == Guid(expressionCharUuid)) _exprChar = c;
        if (c.uuid == Guid(audioCharUuid)) _audioChar = c;
        if (c.uuid == Guid(textCharUuid)) _textChar = c;
        if (c.uuid == Guid(statusCharUuid)) _statusChar = c;
        if (c.uuid == Guid(audioStreamCharUuid)) _audioStreamChar = c;
        if (c.uuid == Guid(imageCharUuid)) _imageChar = c;
      }

      if (_statusChar != null) {
        await _statusChar!.setNotifyValue(true);
        _statusNotificationSub = _statusChar!.lastValueStream.listen((value) {
          _parseStatusData(value);
        });
        addLog("Status notifications enabled.", "BLE");
        try {
          final initVal = await _statusChar!.read();
          _parseStatusData(initVal);
        } catch (e) {
          addLog("Failed to read initial status: $e", "WARNING");
        }
      }

      if (_audioStreamChar != null) {
        await _audioStreamChar!.setNotifyValue(true);
        _audioStreamNotificationSub = _audioStreamChar!.lastValueStream.listen((value) {
          if (value.isNotEmpty) {
            _audioStreamController.add(value);
          }
        });
        addLog("Audio stream notifications enabled.", "BLE");
      }

      // Sync clock to hardware after successful connection
      Future.delayed(const Duration(milliseconds: 700), () => syncClockToHardware());
      Future.delayed(const Duration(milliseconds: 1400), () => queryHardwareEvents());

      // App is Single Source of Truth: push app's stored robot variant and negative display settings to hardware upon connection
      Future.delayed(const Duration(milliseconds: 900), () async {
        try {
          final prefs = await SharedPreferences.getInstance();
          final jsonStr = prefs.getString('robots_database');
          String targetVariant = 'mr_luna';
          if (jsonStr != null) {
            final List<dynamic> decoded = jsonDecode(jsonStr);
            final primary = decoded.firstWhere((r) => r['isPrimary'] == true, orElse: () => decoded.isNotEmpty ? decoded.first : null);
            if (primary != null && primary['variant'] != null) {
              targetVariant = primary['variant'] as String;
            }
          }
          final negEnabled = prefs.getBool('negativeEnabled') ?? false;
          final silentModeVal = prefs.getBool('silentMode') ?? false;

          final speedMsVal = (prefs.getDouble('gifSpeed') ?? 169.0).toInt();
          final introSpeedMsVal = (prefs.getDouble('gifIntroSpeed') ?? 169.0).toInt();
          final introSoundSpeedVal = (prefs.getDouble('introSoundSpeed') ?? 169.0).toInt();
          final oledBrightnessVal = (prefs.getDouble('oledBrightness') ?? 3.0).round();
          final clockStyleVal = prefs.getInt('clockStyle') ?? 0;

          final defaultGifStr = prefs.getString('defaultGif') ?? 'default';
          final introGifStr = prefs.getString('introGif') ?? 'default';
          final touchSingleStr = prefs.getString('touchSingle') ?? 'default';
          final touchDoubleStr = prefs.getString('touchDouble') ?? 'default';
          final touchLongStr = prefs.getString('touchLong') ?? 'default';

          const animKeys = ['happy', 'angry', 'relaxed', 'sad', 'scared', 'surprised', 'neutral'];

          int getExpressionValue(String val) {
            if (val == 'cycle') return 99;
            if (val == 'default') return 0;
            final idx = animKeys.indexOf(val);
            return idx != -1 ? 100 + idx : 0;
          }

          int getTouchActionValue(String val) {
            if (val == 'default') return 0;
            if (val == 'clock') return 1;
            if (val == 'skip_anim') return 2;
            if (val == 'bt_toggle') return 3;
            final idx = animKeys.indexOf(val);
            return idx != -1 ? 20 + idx : 0;
          }

          final notificationDurationSecVal = (prefs.getDouble('notificationDuration') ?? 5.0).round();
          final reminderDurationSecVal = (prefs.getDouble('reminderDuration') ?? 10.0).round();
          final birthdayDurationSecVal = (prefs.getDouble('birthdayDuration') ?? 15.0).round();

          addLog("Pushing app primary variant ($targetVariant) & negative display ($negEnabled) & silentMode ($silentModeVal) to hardware...", "BLE");
          await transmitModelVariant(targetVariant);
          
          await transmitSaveSettings(
            bleEnabled: true,
            speedMs: speedMsVal,
            defaultGif: getExpressionValue(defaultGifStr),
            introGif: getExpressionValue(introGifStr),
            touchSingle: getTouchActionValue(touchSingleStr),
            touchDouble: getTouchActionValue(touchDoubleStr),
            touchLong: getTouchActionValue(touchLongStr),
            negativeEnabled: negEnabled,
            introSpeedMs: introSpeedMsVal,
            introSoundSpeed: introSoundSpeedVal,
            notificationDurationSec: notificationDurationSecVal,
            reminderDurationSec: reminderDurationSecVal,
            birthdayDurationSec: birthdayDurationSecVal,
            clockStyle: clockStyleVal,
            oledBrightness: oledBrightnessVal,
            silentMode: silentModeVal,
          );
        } catch (e) {
          addLog("Failed to sync app settings to hardware on connect: $e", "WARNING");
        }
      });

      // Sync relationship communication state and mapping to primary
      Future.delayed(const Duration(milliseconds: 1100), () async {
        await _writePrimaryTextDirect("REL_COMM:${isPrimaryCommEnabled ? 1 : 0}");
        await _writePrimaryTextDirect("SET_REL_MAP:1|$relPrimaryTapExpr|$relPrimaryTapSound");
        await _writePrimaryTextDirect("SET_REL_MAP:2|$relPrimaryDoubleExpr|$relPrimaryDoubleSound");
        await _writePrimaryTextDirect("SET_REL_MAP:4|$relPrimaryLongExpr|$relPrimaryLongSound");
      });

    } catch (e) {
      addLog("Service discovery failed: $e", "ERROR");
    }
  }

  void _parseStatusData(List<int> value) {
    if (value.isEmpty) return;
    try {
      final dataStr = utf8.decode(value);
      
      // Parse direct log notifications from the robot
      if (dataStr.startsWith("LOG:")) {
        final logMsg = dataStr.substring(4);
        addLog(logMsg, "ROBOT");
        
        if (logMsg.startsWith("ACK:")) {
          final ackId = logMsg.substring(4);
          _handleAckReceived(ackId);
        }
        
        // Handle primary touch events (make companion respond)
        if (logMsg.startsWith("TOUCH_REL:")) {
          final payload = logMsg.substring(10);
          final parts = payload.split('|');
          if (parts.length >= 3) {
            final eventType = parts[0];
            final expr = int.tryParse(parts[1]) ?? 1;
            final sound = int.tryParse(parts[2]) ?? 2;
            _triggerRelationshipActionDirect(fromPrimary: true, eventType: eventType, expr: expr, sound: sound);
          }
        } else if (logMsg.startsWith("TOUCH:")) {
          final event = logMsg.substring(6); // TAP, DOUBLE, TRIPLE, LONG
          _triggerRelationshipAction(fromPrimary: true, eventType: event);
        } else if (logMsg.startsWith("EXPR_SYNC:")) {
          final payload = logMsg.substring(10);
          final parts = payload.split('|');
          if (parts.isNotEmpty) {
            _activeExpressionId = int.tryParse(parts[0]) ?? _activeExpressionId;
            if (parts.length > 1) {
              _activeExpressionLabel = parts[1].trim();
            }
            notifyListeners();
          }
        } else if (logMsg.startsWith("SCREEN_SYNC:")) {
          final screenName = logMsg.substring(12).trim();
          _activeScreenMode = screenName;
          notifyListeners();
        } else if (logMsg.startsWith("SET_SYNC:")) {
          final payload = logMsg.substring(9);
          final parts = payload.split(',');
          if (parts.length >= 4) {
            final clockStyle = int.tryParse(parts[0]) ?? 0;
            final oledBrightness = int.tryParse(parts[1]) ?? 3;
            final negativeDisplay = (parts[2] == "1");
            final silent = (parts[3] == "1");
            if (onSettingsSyncedFromWatch != null) {
              onSettingsSyncedFromWatch!(clockStyle, oledBrightness, negativeDisplay, silent);
            }
          }
        } else if (logMsg.startsWith("EVT_START:")) {
          _hardwareEvents.clear();
          _isHardwareEventsSynced = false;
          notifyListeners();
        } else if (logMsg.startsWith("EVT:")) {
          final payload = logMsg.substring(4);
          final parts = payload.split('|');
          if (parts.length >= 5) {
            final id = parts[0].trim();
            final type = parts[1].trim();
            final dateStr = parts[2].trim();
            final timeStr = parts[3].trim();
            final title = parts[4].trim();

            DateTime parsedDt = DateTime.now();
            final timeParts = timeStr.split(':');
            int h = timeParts.isNotEmpty ? (int.tryParse(timeParts[0]) ?? 12) : 12;
            int m = timeParts.length > 1 ? (int.tryParse(timeParts[1]) ?? 0) : 0;

            if (dateStr.isNotEmpty && dateStr != "*") {
              final dParts = dateStr.split(' ');
              if (dParts.length >= 2) {
                int day = int.tryParse(dParts[0]) ?? parsedDt.day;
                const months = ['jan', 'feb', 'mar', 'apr', 'may', 'jun', 'jul', 'aug', 'sep', 'oct', 'nov', 'dec'];
                int monIdx = months.indexOf(dParts[1].toLowerCase());
                int mon = (monIdx >= 0) ? (monIdx + 1) : parsedDt.month;
                parsedDt = DateTime(parsedDt.year, mon, day, h, m);
              } else {
                parsedDt = DateTime(parsedDt.year, parsedDt.month, parsedDt.day, h, m);
              }
            } else {
              parsedDt = DateTime(parsedDt.year, parsedDt.month, parsedDt.day, h, m);
            }

            final eventItem = CalendarEvent(
              id: id,
              title: title,
              dateTime: parsedDt,
              type: type,
            );
            _hardwareEvents.removeWhere((e) => e.id == id);
            _hardwareEvents.add(eventItem);
            notifyListeners();
          }
        } else if (logMsg.startsWith("EVT_END")) {
          _isHardwareEventsSynced = true;
          notifyListeners();
          if (onHardwareEventsSynced != null) {
            onHardwareEventsSynced!(_hardwareEvents);
          }
        } else if (logMsg.startsWith("EVT_DEL_OK:")) {
          final id = logMsg.substring(11).trim();
          _hardwareEvents.removeWhere((e) => e.id == id);
          notifyListeners();
        } else if (logMsg.startsWith("ALARM_RING:")) {
          final payload = logMsg.substring(11);
          final parts = payload.split('|');
          final ringId = parts.isNotEmpty ? parts[0] : "";
          final ringTitle = parts.length > 1 ? parts[1] : "Alarm";
          if (onHardwareAlarmRinging != null) {
            onHardwareAlarmRinging!(ringId, ringTitle);
          }
        } else if (logMsg.startsWith("ALARM_DISMISSED")) {
          if (onHardwareAlarmDismissed != null) {
            onHardwareAlarmDismissed!();
          }
        }
        
        _robotEventsController.add(logMsg);
        return;
      }
      
      // Expected format: uptime_s,touchCount,batteryV,exprId
      final parts = dataStr.split(',');
      if (parts.length >= 3) {
        _uptimeSeconds = int.tryParse(parts[0]) ?? _uptimeSeconds;
        _touchCount = int.tryParse(parts[1]) ?? _touchCount;
        _batteryVoltage = double.tryParse(parts[2]) ?? _batteryVoltage;
        
        if (parts.length >= 4) {
          _activeExpressionId = int.tryParse(parts[3]) ?? _activeExpressionId;
        }
        if (parts.length >= 5) {
          _activeExpressionLabel = parts[4].trim();
        }
        if (parts.length >= 6) {
          final ip = parts[5].trim();
          if (ip.isNotEmpty && ip != "0.0.0.0" && ip != "localhost") {
            setServerIp(ip);
          }
        }
        notifyListeners();
      }
    } catch (e) {
      print("Error decoding status packet: $e");
    }
  }

  void _handleDisconnect() {
    _isConnected = false;
    _isConnecting = false;
    _connectedDevice = null;
    _exprChar = null;
    _audioChar = null;
    _textChar = null;
    _statusChar = null;
    _imageChar = null;
    
    _uptimeSeconds = 0;
    _touchCount = 0;
    _batteryVoltage = 0.0;
    _activeExpressionId = 0;
    _activeExpressionLabel = "IDLE";

    _connectionStateSub?.cancel();
    _statusNotificationSub?.cancel();
    _audioStreamNotificationSub?.cancel();
    _audioStreamNotificationSub = null;
    
    addLog("Disconnected from Mr.&Ms Luna companion robot.", "BLE");
    
    // Update background service immediately
    const MethodChannel('com.mrmsluna/notifications').invokeMethod('updateConnectionStatus', {'connected': false});
    
    notifyListeners();

    // Trigger fast reconnection immediately when disconnected!
    Future.delayed(const Duration(milliseconds: 500), () => triggerReconnection());
  }

  Future<void> disconnect() async {
    _pairedDeviceId = null;
    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.remove("paired_device_id");
    } catch (e) {
      print("Failed to clear paired device setting: $e");
    }

    if (_connectedDevice != null) {
      addLog("Disconnecting device manually...", "BLE");
      await _connectedDevice!.disconnect();
    }
    _handleDisconnect();

    if (_companionDevice != null) {
      await disconnectCompanion();
    }
  }

  // Characteristics Write Helpers
  Future<void> transmitExpression(int expr, String label) async {
    // Instant optimistic state update for 0ms latency in UI
    _activeExpressionId = expr;
    _activeExpressionLabel = label;
    notifyListeners();

    if (!_isConnected || _exprChar == null) {
      final success = await _transmitWifiCommand("EXPR:$expr,$label");
      if (!success) {
        addLog("Cannot transmit expression: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      final labelBytes = utf8.encode(label.toUpperCase());
      final payload = Uint8List(1 + labelBytes.length);
      payload[0] = expr;
      payload.setRange(1, payload.length, labelBytes);
      
      final bool writeWithoutResp = _exprChar!.properties.writeWithoutResponse;
      await _exprChar!.write(payload, withoutResponse: writeWithoutResp);
      addLog("Sent expression: $expr ($label)", "BLE");
    } catch (e) {
      addLog("Failed to write expression characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitAudio(int soundId) async {
    if (!_isConnected || _audioChar == null) {
      final success = await _transmitWifiCommand("AUDIO:$soundId");
      if (!success) {
        addLog("Cannot transmit audio: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      final bool writeWithoutResp = _audioChar!.properties.writeWithoutResponse;
      await _audioChar!.write([soundId], withoutResponse: writeWithoutResp);
      addLog("Sent audio trigger: SFX $soundId", "BLE");
    } catch (e) {
      addLog("Failed to write audio characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitMarqueeText(String text) async {
    await _writeTextWithAck(text, "Marquee Text");
  }

  Future<void> transmitText(String text) async {
    await _writeTextWithAck(text, "Text Command");
  }

  Future<void> transmitNavTelemetry(String payload) async {
    if (!_isConnected || _textChar == null) {
      await _transmitWifiCommand(payload);
      return;
    }
    try {
      final bytes = utf8.encode(payload);
      final bool writeNR = _textChar!.properties.writeWithoutResponse;
      await _textChar!.write(bytes, withoutResponse: writeNR);
      addLog("Transmitted Nav: '$payload'", "NAV");
    } catch (e) {
      addLog("Nav transmit fallback: $e", "ERROR");
      try {
        await _textChar!.write(utf8.encode(payload), withoutResponse: false);
      } catch (e2) {
        addLog("Nav transmit write failed: $e2", "ERROR");
      }
    }
  }

  // ── Image Transfer Helpers ─────────────────────────────────────────────

  /// Returns true if the image BLE characteristic is available.
  bool get imageCharAvailable => _imageChar != null;

  /// Sends a raw binary chunk to the image transfer characteristic.
  /// Slices into 200-byte MTU-safe subpackets for reliable transmission.
  Future<bool> sendImageChunk(Uint8List chunk) async {
    if (_imageChar == null) {
      addLog('Image char not available — not connected or char missing', 'ERROR');
      return false;
    }
    try {
      const int maxPacketSize = 200;
      int offset = 0;
      while (offset < chunk.length) {
        final end = (offset + maxPacketSize < chunk.length) ? offset + maxPacketSize : chunk.length;
        final sub = chunk.sublist(offset, end);
        try {
          await _imageChar!.write(sub, withoutResponse: true);
        } catch (e) {
          // If writeWithoutResponse fails (e.g. MTU or stack buffer issue), try write with response
          await _imageChar!.write(sub, withoutResponse: false);
        }
        offset = end;
        await Future.delayed(const Duration(milliseconds: 2));
      }
      return true;
    } catch (e) {
      addLog('sendImageChunk failed: $e', 'ERROR');
      return false;
    }
  }

  Future<void> transmitImgStart(int size, String crc32Hex) async {
    await _writeTextWithAck('IMG_START:$size:$crc32Hex', 'Image Start');
  }

  Future<void> transmitImgEnd() async {
    await _writeTextWithAck('IMG_END', 'Image End');
  }

  Future<void> transmitImgCancel() async {
    await _writePrimaryTextDirect('IMG_CANCEL');
  }

  Future<void> transmitImgDelete() async {
    await _writeTextWithAck('IMG_DELETE', 'Delete Wallpaper');
  }

  Future<void> transmitImgShow() async {
    await _writeTextWithAck('IMG_SHOW', 'Show Wallpaper');
  }

  Future<void> syncClockToHardware() async {
    final now = DateTime.now();
    final hh = now.hour.toString().padLeft(2, '0');
    final mm = now.minute.toString().padLeft(2, '0');
    final ss = now.second.toString().padLeft(2, '0');
    
    final List<String> weekdays = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
    final List<String> months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    final weekday = weekdays[now.weekday - 1];
    final month = months[now.month - 1];
    final dateStr = "${now.day.toString().padLeft(2, '0')} $month";
    
    final payloadStr = 'TIME:$hh:$mm:$ss,$weekday,$dateStr';
    await _writeTextWithAck(payloadStr, "Clock Sync");
  }

  Future<void> updateTimeFormat(bool is12H) async {
    final payloadStr = '12HR:${is12H ? 1 : 0}';
    await _writeTextWithAck(payloadStr, "Time Format");
  }

  Future<void> transmitCalendarEvent(String type, String time, String title) async {
    final payloadStr = 'CAL:$type,$time,$title';
    await _writeTextWithAck(payloadStr, "Calendar Event");
  }

  Future<void> transmitHardwareEvent({
    required String id,
    required String type,
    required String date,
    required String time,
    required String title,
  }) async {
    final payloadStr = 'EVT_ADD:$id,$type,$date,$time,$title';
    await _writeTextWithAck(payloadStr, "Add Hardware Event");
  }

  Future<void> deleteHardwareEvent(String id) async {
    final payloadStr = 'EVT_DEL:$id';
    await _writeTextWithAck(payloadStr, "Delete Hardware Event");
  }

  Future<void> queryHardwareEvents() async {
    await _writeTextWithAck('EVT_GET', "Query Hardware Events");
  }

  Future<void> clearAllHardwareEvents() async {
    await _writeTextWithAck('EVT_CLEAR', "Clear Hardware Events");
  }

  Future<void> dismissHardwareAlarm() async {
    await _writeTextWithAck('EVT_DISMISS', "Dismiss Hardware Alarm");
  }

  Future<void> transmitSaveSettings({
    required bool bleEnabled,
    required int speedMs,
    required int defaultGif,
    required int introGif,
    required int touchSingle,
    required int touchDouble,
    required int touchLong,
    required bool negativeEnabled,
    required int introSpeedMs,
    required int introSoundSpeed,
    required int notificationDurationSec,
    required int reminderDurationSec,
    required int birthdayDurationSec,
    required int clockStyle,
    required int oledBrightness,
    required bool silentMode,
  }) async {
    final bleVal = bleEnabled ? "1" : "0";
    final negVal = negativeEnabled ? "1" : "0";
    final silentVal = silentMode ? "1" : "0";
    final payloadStr = 'SET:$bleVal,$speedMs,$defaultGif,$introGif,$touchSingle,$touchDouble,$touchLong,$negVal,$introSpeedMs,$introSoundSpeed,$notificationDurationSec,$reminderDurationSec,$birthdayDurationSec,$clockStyle,$oledBrightness,$silentVal';
    await _writeTextWithAck(payloadStr, "Settings Sync");
  }

  Future<void> transmitResetDevice() async {
    await _writeTextWithAck('RESET', "Reset Device");
  }

  Future<void> transmitWifiConfig(String ssid, String password) async {
    final payloadStr = 'WIFI:$ssid,$password';
    await _writeTextWithAck(payloadStr, "Wi-Fi Config");
  }

  Future<void> transmitCompanionPairing(String macAddress, String relationship) async {
    final payloadStr = 'PAIR:$macAddress,$relationship';
    await _writeTextWithAck(payloadStr, "Companion Pairing");
  }

  Future<void> transmitRelationship(String relationship) async {
    final payloadStr = 'RELATION:$relationship';
    await _writeTextWithAck(payloadStr, "Relationship Update");
  }

  Future<void> transmitModelVariant(String variant) async {
    final payloadStr = 'MODEL:$variant';
    await _writeTextWithAck(payloadStr, "Model Variant");
  }

  Future<void> transmitWake() async {
    await _writeTextWithAck('WAKE', "Wake Robot");
  }

  Future<void> transmitAnimationSelect(int animIndex) async {
    await _writeTextWithAck('ANIM:$animIndex', "Select Sprite AI Anim $animIndex");
  }

  Future<void> transmitMapClear() async {
    await _writeTextWithAck('MAPCLEAR', "Clear Live Map");
  }

  Future<void> transmitMapLine(int lineIdx, String base64Rgb565) async {
    if (_textChar == null) return;
    try {
      final payload = utf8.encode('MAPLINE:$lineIdx,$base64Rgb565');
      await _textChar!.write(payload, withoutResponse: true);
    } catch (e) {
      print("Error sending map line: $e");
    }
  }

  Future<void> transmitMapChunk(int chunkIdx, String base64Chunk) async {
    if (_textChar == null) return;
    try {
      final payload = utf8.encode('MAPCHUNK:$chunkIdx,$base64Chunk');
      await _textChar!.write(payload, withoutResponse: true);
    } catch (e) {
      print("Error sending map chunk: $e");
    }
  }

  void updateNavigation(
    String direction,
    String distance,
    String description, {
    String road = '',
    String totalTime = '',
    String totalDist = '',
    String eta = '',
  }) {
    _navDirection = direction;
    _navDistance = distance;
    _navDescription = description;
    _navRoad = road;
    _navTotalTime = totalTime;
    _navTotalDist = totalDist;
    _navEta = eta;
    _isNavActive = true;
    _activeScreenMode = "MAPS";
    notifyListeners();
  }

  void clearNavigation() {
    _isNavActive = false;
    _navDirection = "STRAIGHT";
    _navDistance = "--";
    _navDescription = "";
    _navRoad = "";
    _navTotalTime = "";
    _navTotalDist = "";
    _navEta = "";
    if (_activeScreenMode == "MAPS") {
      _activeScreenMode = "CLOCK";
    }
    notifyListeners();
  }

  Future<void> transmitSleep( ) async {
    await _writeTextWithAck('SLEEP', "Sleep Robot");
  }

  Future<void> transmitStartCall() async {
    await _writeTextWithAck('CALL:START', "Start Call");
  }

  Future<void> transmitStopCall() async {
    await _writeTextWithAck('CALL:STOP', "Stop Call");
  }

  Future<void> transmitStartLoopback() async {
    await _writeTextWithAck('LOOPBACK:START', "Start Loopback");
  }

  Future<void> transmitStopLoopback() async {
    await _writeTextWithAck('LOOPBACK:STOP', "Stop Loopback");
  }

  /// Instantly stops music on hardware with no ACK wait.
  /// Used for stop button and song switching — hardware reacts in <10ms.
  Future<void> transmitStopMusicInstant() async {
    if (_textChar == null) return;
    try {
      final payload = utf8.encode('MUSIC:STOP');
      // ignore: unawaited_futures
      _textChar!.write(payload, withoutResponse: true);
    } catch (_) {}
  }

  Future<void> transmitStartMusic() async {
    await _writeTextWithAck('MUSIC:START', "Start Music");
  }

  Future<void> transmitStopMusic() async {
    await _writeTextWithAck('MUSIC:STOP', "Stop Music");
  }

  /// Send volume level (0–100) to ESP32 speaker.
  /// ESP32 firmware maps this to I2S DMA amplitude scaling.
  Future<void> transmitVolume(int level) async {
    final clamped = level.clamp(0, 100);
    await _writeTextWithAck('VOL:$clamped', "Volume $clamped");
  }

  /// Send bass boost level (0–10) to ESP32 speaker.
  /// ESP32 firmware applies a simple low-shelf EQ boost.
  Future<void> transmitBass(int level) async {
    final clamped = level.clamp(0, 10);
    await _writeTextWithAck('BASS:$clamped', "Bass $clamped");
  }

  /// Sends a business card URL to Luna and waits for a QR_SAVED:OK log confirmation.
  Future<bool> transmitBusinessCardUrl(String url) async {
    if (!_isConnected) {
      addLog("Cannot sync business card: Device not connected", "ERROR");
      return false;
    }
    
    final completer = Completer<bool>();
    StreamSubscription? sub;
    Timer? timeoutTimer;
    
    sub = robotEvents.listen((event) {
      if (event == "QR_SAVED:OK") {
        if (!completer.isCompleted) {
          timeoutTimer?.cancel();
          sub?.cancel();
          completer.complete(true);
        }
      } else if (event == "QR_SAVED:FAIL" || event == "QR_SAVED:INVALID") {
        if (!completer.isCompleted) {
          timeoutTimer?.cancel();
          sub?.cancel();
          completer.complete(false);
        }
      }
    });
    
    timeoutTimer = Timer(const Duration(seconds: 5), () {
      if (!completer.isCompleted) {
        sub?.cancel();
        completer.complete(false);
      }
    });
    
    try {
      await _writeTextWithAck('QRCARD:$url', "Business Card URL");
      return await completer.future;
    } catch (e) {
      timeoutTimer.cancel();
      sub.cancel();
      addLog("Failed to sync business card URL: $e", "ERROR");
      return false;
    }
  }

  /// Clears the stored business card URL on Luna.
  Future<bool> transmitClearBusinessCard() async {
    if (!_isConnected) return false;
    
    final completer = Completer<bool>();
    StreamSubscription? sub;
    Timer? timeoutTimer;
    
    sub = robotEvents.listen((event) {
      if (event == "QR_CLEARED:OK") {
        if (!completer.isCompleted) {
          timeoutTimer?.cancel();
          sub?.cancel();
          completer.complete(true);
        }
      }
    });
    
    timeoutTimer = Timer(const Duration(seconds: 5), () {
      if (!completer.isCompleted) {
        sub?.cancel();
        completer.complete(false);
      }
    });
    
    try {
      await _writeTextWithAck('QRCARD:CLEAR', "Clear Business Card");
      return await completer.future;
    } catch (e) {
      timeoutTimer.cancel();
      sub.cancel();
      return false;
    }
  }

  // ─── Direct BLE write helper (no ACK_ID prefix) ───────────────────────────
  // Used for wallpaper transfers where the ACK_ID prefix would inflate the
  // packet beyond the negotiated BLE MTU.
  Future<bool> _writeRaw(String text) async {
    if (!_isConnected || _textChar == null) return false;
    try {
      final payload = utf8.encode(text);
      await _textChar!.write(payload, withoutResponse: false);
      return true;
    } catch (e) {
      addLog("Raw write error: $e", "ERROR");
      return false;
    }
  }

  // ─── Wait for a specific log event on the robotEvents stream ────────────────
  Future<bool> _waitForEvent(String expected, {int timeoutMs = 10000}) async {
    final completer = Completer<bool>();
    StreamSubscription? sub;
    Timer? timer;

    sub = robotEvents.listen((event) {
      final e = event.trim();
      if (e == expected) {
        if (!completer.isCompleted) {
          timer?.cancel();
          sub?.cancel();
          completer.complete(true);
        }
      } else if (e.endsWith(":FAIL") && expected.contains(e.split(":").first)) {
        if (!completer.isCompleted) {
          timer?.cancel();
          sub?.cancel();
          completer.complete(false);
        }
      }
    });

    timer = Timer(Duration(milliseconds: timeoutMs), () {
      if (!completer.isCompleted) {
        sub?.cancel();
        completer.complete(false);
      }
    });

    return completer.future;
  }

  /// Starts wallpaper transmission.
  Future<bool> transmitWallpaperStart(int expectedSize) async {
    if (!_isConnected || _textChar == null) return false;
    final future = _waitForEvent('WP_START:OK', timeoutMs: 10000);
    await Future.delayed(const Duration(milliseconds: 30)); // let listener attach
    final ok = await _writeRaw('WP_START:$expectedSize');
    if (!ok) return false;
    return future;
  }

  /// Sends one chunk of wallpaper hex data. Keep hexData ≤ 60 chars.
  Future<bool> transmitWallpaperChunk(String hexData) async {
    if (!_isConnected || _textChar == null) return false;
    final future = _waitForEvent('WP_CHUNK:OK', timeoutMs: 12000);
    await Future.delayed(const Duration(milliseconds: 20));
    final ok = await _writeRaw('WP_CHUNK:$hexData');
    if (!ok) return false;
    return future;
  }

  /// Concludes wallpaper transmission.
  Future<bool> transmitWallpaperEnd() async {
    if (!_isConnected || _textChar == null) return false;
    final future = _waitForEvent('WP_END:OK', timeoutMs: 10000);
    await Future.delayed(const Duration(milliseconds: 30));
    final ok = await _writeRaw('WP_END');
    if (!ok) return false;
    return future;
  }

  /// Clears wallpaper on the smartwatch.
  Future<bool> transmitWallpaperClear() async {
    if (!_isConnected || _textChar == null) return false;
    final future = _waitForEvent('WP_CLEAR:OK', timeoutMs: 8000);
    await Future.delayed(const Duration(milliseconds: 30));
    final ok = await _writeRaw('WP_CLEAR');
    if (!ok) return false;
    return future;
  }

  @override
  void dispose() {
    _scanSub?.cancel();
    _connectionStateSub?.cancel();
    _statusNotificationSub?.cancel();
    _audioStreamNotificationSub?.cancel();
    for (var pending in _pendingAcks.values) {
      pending.timer?.cancel();
    }
    _pendingAcks.clear();
    super.dispose();
  }

  Future<void> _writeTextWithAck(String text, String description) async {
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(text);
      if (success) {
        addLog("$description sent via Wi-Fi: '$text'", "WIFI");
      } else {
        addLog("Cannot send $description: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }

    final id = (_nextMsgId++).toString();
    final completer = Completer<bool>();
    final pending = _PendingAck(id: id, text: text, completer: completer, description: description);
    _pendingAcks[id] = pending;

    _sendTextPayload(pending);

    await completer.future;
  }

  void _sendTextPayload(_PendingAck pending) {
    if (!_isConnected || _textChar == null) {
      if (!pending.completer.isCompleted) {
        _pendingAcks.remove(pending.id);
        pending.completer.complete(false);
      }
      return;
    }

    final fullPayloadStr = "ACK_ID:${pending.id}|${pending.text}";
    try {
      final payload = utf8.encode(fullPayloadStr);
      _textChar!.write(payload, withoutResponse: false).then((_) {
        addLog("Sent ${pending.description} (ID: ${pending.id})", "BLE");
      }).catchError((e) {
        addLog("Write error (ID: ${pending.description} ${pending.id}): $e", "ERROR");
      });
    } catch (e) {
      addLog("Payload encoding error: $e", "ERROR");
    }

    pending.timer?.cancel();
    pending.timer = Timer(const Duration(milliseconds: 1000), () {
      if (pending.completer.isCompleted) return;

      if (pending.retryCount < 3) {
        pending.retryCount++;
        addLog("ACK timeout for ID ${pending.id}. Retrying (${pending.retryCount}/3)...", "BLE");
        _sendTextPayload(pending);
      } else {
        addLog("ACK failed for ID ${pending.id} after 3 retries.", "ERROR");
        _pendingAcks.remove(pending.id);
        pending.completer.complete(false);
      }
    });
  }

  void _handleAckReceived(String ackId) {
    final pending = _pendingAcks.remove(ackId);
    if (pending != null) {
      pending.timer?.cancel();
      addLog("ACK received for ID: $ackId", "BLE");
      if (!pending.completer.isCompleted) {
        pending.completer.complete(true);
      }
    }
  }

  // ==========================================
  // COMPANION / RELATIONSHIP METHODS
  // ==========================================

  Future<void> loadRelationshipSettings() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      relPrimaryTapExpr = prefs.getInt("rel_primary_tap_expr") ?? 1;
      relPrimaryTapSound = prefs.getInt("rel_primary_tap_sound") ?? 2;
      relPrimaryDoubleExpr = prefs.getInt("rel_primary_double_expr") ?? 6;
      relPrimaryDoubleSound = prefs.getInt("rel_primary_double_sound") ?? 6;
      relPrimaryLongExpr = prefs.getInt("rel_primary_long_expr") ?? 5;
      relPrimaryLongSound = prefs.getInt("rel_primary_long_sound") ?? 4;

      relCompanionTapExpr = prefs.getInt("rel_companion_tap_expr") ?? 1;
      relCompanionTapSound = prefs.getInt("rel_companion_tap_sound") ?? 2;
      relCompanionDoubleExpr = prefs.getInt("rel_companion_double_expr") ?? 6;
      relCompanionDoubleSound = prefs.getInt("rel_companion_double_sound") ?? 6;
      relCompanionLongExpr = prefs.getInt("rel_companion_long_expr") ?? 5;
      relCompanionLongSound = prefs.getInt("rel_companion_long_sound") ?? 4;

      isPrimaryCommEnabled = prefs.getBool("rel_primary_comm_enabled") ?? true;
      isCompanionCommEnabled = prefs.getBool("rel_companion_comm_enabled") ?? true;
    } catch (e) {
      print("Error loading relationship settings: $e");
    }
  }

  Future<void> togglePrimaryComm(bool enabled) async {
    isPrimaryCommEnabled = enabled;
    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setBool("rel_primary_comm_enabled", enabled);
    } catch (e) {
      print("Error saving primary comm setting: $e");
    }
    notifyListeners();
    if (isConnected) {
      await _writePrimaryTextDirect("REL_COMM:${enabled ? 1 : 0}");
    }
  }

  Future<void> toggleCompanionComm(bool enabled) async {
    isCompanionCommEnabled = enabled;
    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setBool("rel_companion_comm_enabled", enabled);
    } catch (e) {
      print("Error saving companion comm setting: $e");
    }
    notifyListeners();
    if (isCompanionConnected) {
      await _writeCompanionTextDirect("REL_COMM:${enabled ? 1 : 0}");
    }
  }

  Future<void> saveRelationshipSettings({
    required int primaryTapExpr,
    required int primaryTapSound,
    required int primaryDoubleExpr,
    required int primaryDoubleSound,
    required int primaryLongExpr,
    required int primaryLongSound,
    required int companionTapExpr,
    required int companionTapSound,
    required int companionDoubleExpr,
    required int companionDoubleSound,
    required int companionLongExpr,
    required int companionLongSound,
  }) async {
    relPrimaryTapExpr = primaryTapExpr;
    relPrimaryTapSound = primaryTapSound;
    relPrimaryDoubleExpr = primaryDoubleExpr;
    relPrimaryDoubleSound = primaryDoubleSound;
    relPrimaryLongExpr = primaryLongExpr;
    relPrimaryLongSound = primaryLongSound;

    relCompanionTapExpr = companionTapExpr;
    relCompanionTapSound = companionTapSound;
    relCompanionDoubleExpr = companionDoubleExpr;
    relCompanionDoubleSound = companionDoubleSound;
    relCompanionLongExpr = companionLongExpr;
    relCompanionLongSound = companionLongSound;

    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setInt("rel_primary_tap_expr", primaryTapExpr);
      await prefs.setInt("rel_primary_tap_sound", primaryTapSound);
      await prefs.setInt("rel_primary_double_expr", primaryDoubleExpr);
      await prefs.setInt("rel_primary_double_sound", primaryDoubleSound);
      await prefs.setInt("rel_primary_long_expr", primaryLongExpr);
      await prefs.setInt("rel_primary_long_sound", primaryLongSound);

      await prefs.setInt("rel_companion_tap_expr", companionTapExpr);
      await prefs.setInt("rel_companion_tap_sound", companionTapSound);
      await prefs.setInt("rel_companion_double_expr", companionDoubleExpr);
      await prefs.setInt("rel_companion_double_sound", companionDoubleSound);
      await prefs.setInt("rel_companion_long_expr", companionLongExpr);
      await prefs.setInt("rel_companion_long_sound", companionLongSound);
    } catch (e) {
      print("Error saving relationship settings: $e");
    }
    notifyListeners();
    
    // Sync mappings to robots if connected
    if (isConnected) {
      _writePrimaryTextDirect("SET_REL_MAP:1|$primaryTapExpr|$primaryTapSound");
      _writePrimaryTextDirect("SET_REL_MAP:2|$primaryDoubleExpr|$primaryDoubleSound");
      _writePrimaryTextDirect("SET_REL_MAP:4|$primaryLongExpr|$primaryLongSound");
    }
    if (isCompanionConnected) {
      _writeCompanionTextDirect("SET_REL_MAP:1|$companionTapExpr|$companionTapSound");
      _writeCompanionTextDirect("SET_REL_MAP:2|$companionDoubleExpr|$companionDoubleSound");
      _writeCompanionTextDirect("SET_REL_MAP:4|$companionLongExpr|$companionLongSound");
    }
  }

  String _getExpressionLabel(int exprId) {
    if (exprId >= 100) {
      final idx = exprId - 100;
      final keys = DatabaseService.animMapping.keys.toList();
      if (idx >= 0 && idx < keys.length) {
        return DatabaseService.animMapping[keys[idx]]!['label'] as String;
      }
    }
    switch (exprId) {
      case 1: return "HAPPY";
      case 2: return "SAD";
      case 3: return "ANGRY";
      case 4: return "SURPRISED";
      case 5: return "SLEEPING";
      case 6: return "WINK";
      default: return "IDLE";
    }
  }

  void _triggerRelationshipActionDirect({required bool fromPrimary, required String eventType, required int expr, required int sound}) {
    if (fromPrimary && onPrimaryTouchTriggered != null) {
      onPrimaryTouchTriggered!(eventType, expr, sound);
    }
    if (!isPrimaryCommEnabled || !isCompanionCommEnabled) {
      addLog("Relationship action blocked because communication is disabled on one or both devices.", "BLE");
      return;
    }
    if (fromPrimary) {
      if (!_isCompanionConnected) return;
      
      addLog("Trigger Relationship: Primary $eventType -> Companion respond (Expr $expr, Sound $sound)", "BLE");
      _companionActiveExpressionId = expr;
      _companionActiveExpressionLabel = _getExpressionLabel(expr);
      if (companionHasSpeaker) {
        transmitCompanionAudio(sound);
      }
      
      final senderName = _connectedDevice?.platformName.isNotEmpty == true 
          ? _connectedDevice!.platformName 
          : "Primary Luna";
      transmitCompanionNotification(senderName, "$eventType Action", expr: expr);
    } else {
      if (!_isConnected) return;
      
      addLog("Trigger Relationship: Companion $eventType -> Primary respond (Expr $expr, Sound $sound)", "BLE");
      _activeExpressionId = expr;
      _activeExpressionLabel = _getExpressionLabel(expr);
      if (sound > 0) {
        transmitAudio(sound);
      }
      
      final senderName = _companionDevice?.platformName.isNotEmpty == true 
          ? _companionDevice!.platformName 
          : "Companion Luna";
      transmitPrimaryNotification(senderName, "$eventType Action", expr: expr);
    }
  }

  void _triggerRelationshipAction({required bool fromPrimary, required String eventType}) {
    int expr = 1;
    int sound = 2;
    if (eventType == "TAP") {
      expr = relPrimaryTapExpr;
      sound = relPrimaryTapSound;
    } else if (eventType == "DOUBLE") {
      expr = relPrimaryDoubleExpr;
      sound = relPrimaryDoubleSound;
    } else if (eventType == "LONG") {
      expr = relPrimaryLongExpr;
      sound = relPrimaryLongSound;
    } else {
      return;
    }

    if (fromPrimary && onPrimaryTouchTriggered != null) {
      onPrimaryTouchTriggered!(eventType, expr, sound);
    }

    if (!isPrimaryCommEnabled || !isCompanionCommEnabled) {
      
      addLog("Trigger Relationship: Primary $eventType -> Companion respond (Expr $expr, Sound $sound)", "BLE");
      _companionActiveExpressionId = expr;
      _companionActiveExpressionLabel = _getExpressionLabel(expr);
      if (companionHasSpeaker) {
        transmitCompanionAudio(sound);
      }
      
      // Notify the companion robot from whom it received the touch event
      final senderName = _connectedDevice?.platformName.isNotEmpty == true 
          ? _connectedDevice!.platformName 
          : "Primary Luna";
      transmitCompanionNotification(senderName, "$eventType Action", expr: expr);
    } else {
      if (!_isConnected) return;
      int expr = 1;
      int sound = 2;
      if (eventType == "TAP") {
        expr = relCompanionTapExpr;
        sound = relCompanionTapSound;
      } else if (eventType == "DOUBLE") {
        expr = relCompanionDoubleExpr;
        sound = relCompanionDoubleSound;
      } else if (eventType == "LONG") {
        expr = relCompanionLongExpr;
        sound = relCompanionLongSound;
      } else {
        return;
      }
      
      addLog("Trigger Relationship: Companion $eventType -> Primary respond (Expr $expr, Sound $sound)", "BLE");
      _activeExpressionId = expr;
      _activeExpressionLabel = _getExpressionLabel(expr);
      if (sound > 0) {
        transmitAudio(sound);
      }
      
      // Notify the primary robot from whom it received the touch event
      final senderName = _companionDevice?.platformName.isNotEmpty == true 
          ? _companionDevice!.platformName 
          : "Companion Luna";
      transmitPrimaryNotification(senderName, "$eventType Action", expr: expr);
    }
  }

  void handleRemoteCloudTrigger(int expr, int sound, String friendName, String eventType, {String? customLabel}) {
    _activeExpressionId = expr;
    _activeExpressionLabel = customLabel ?? _getExpressionLabel(expr);
    if (sound > 0) {
      transmitAudio(sound);
    }
    transmitExpression(_activeExpressionId, _activeExpressionLabel);
    transmitPrimaryNotification(friendName, _activeExpressionLabel.toUpperCase(), expr: expr);
    notifyListeners();
  }

  Future<void> connectCompanion(BluetoothDevice device) async {
    if (_isCompanionConnecting) return;
    _isCompanionConnecting = true;
    notifyListeners();

    try {
      addLog("Connecting companion robot: ${device.remoteId.str}...", "BLE");
      
      final startState = await device.connectionState.first;
      if (startState == BluetoothConnectionState.connected) {
        _handleCompanionConnected(device);
        return;
      }

      _companionConnectionStateSub?.cancel();
      _companionConnectionStateSub = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.connected) {
          _handleCompanionConnected(device);
        } else if (state == BluetoothConnectionState.disconnected) {
          _handleCompanionDisconnect();
        }
      });

      await device.connect(autoConnect: false, timeout: const Duration(seconds: 5));

      final currentState = await device.connectionState.first.timeout(
        const Duration(milliseconds: 500),
        onTimeout: () => BluetoothConnectionState.disconnected,
      );
      if (currentState == BluetoothConnectionState.connected) {
        _handleCompanionConnected(device);
      }
    } catch (e) {
      addLog("Companion connection failed: $e", "WARNING");
      _isCompanionConnecting = false;
      _handleCompanionDisconnect();
    }
  }

  Future<void> connectCompanionById(String deviceId) async {
    if (_isCompanionConnecting) return;
    try {
      addLog("Connecting companion directly to ID: $deviceId...", "BLE");
      final device = BluetoothDevice.fromId(deviceId);
      await connectCompanion(device);
    } catch (e) {
      addLog("Companion direct connection failed: $e", "ERROR");
      _isCompanionConnecting = false;
      _handleCompanionDisconnect();
    }
  }

  void _handleCompanionConnected(BluetoothDevice device) {
    if (_isCompanionConnected) return;
    _isCompanionConnected = true;
    _isCompanionConnecting = false;
    _companionDevice = device;
    addLog("Connected to companion successfully!", "BLE");
    _setupCompanionServices(device);
    
    try {
      device.requestConnectionPriority(connectionPriorityRequest: ConnectionPriority.high);
    } catch (e) {}
    
    notifyListeners();
  }

  Future<void> _setupCompanionServices(BluetoothDevice device) async {
    try {
      await Future.delayed(const Duration(milliseconds: 1000));
      List<BluetoothService> services = await device.discoverServices();
      BluetoothService? targetService = services.firstWhere(
        (s) => s.uuid == Guid(serviceUuid),
      );

      for (var c in targetService.characteristics) {
        if (c.uuid == Guid(expressionCharUuid)) _companionExprChar = c;
        if (c.uuid == Guid(audioCharUuid)) _companionAudioChar = c;
        if (c.uuid == Guid(textCharUuid)) _companionTextChar = c;
        if (c.uuid == Guid(statusCharUuid)) _companionStatusChar = c;
        if (c.uuid == Guid(audioStreamCharUuid)) _companionAudioStreamChar = c;
      }

      if (_companionStatusChar != null) {
        await _companionStatusChar!.setNotifyValue(true);
        _companionStatusNotificationSub = _companionStatusChar!.lastValueStream.listen((value) {
          _parseCompanionStatusData(value);
        });
        addLog("Companion Status notifications enabled.", "BLE");
      }

      // Sync relationship communication state and mapping to companion
      Future.delayed(const Duration(milliseconds: 1200), () async {
        await _writeCompanionTextDirect("REL_COMM:${isCompanionCommEnabled ? 1 : 0}");
        await _writeCompanionTextDirect("SET_REL_MAP:1|$relCompanionTapExpr|$relCompanionTapSound");
        await _writeCompanionTextDirect("SET_REL_MAP:2|$relCompanionDoubleExpr|$relCompanionDoubleSound");
        await _writeCompanionTextDirect("SET_REL_MAP:4|$relCompanionLongExpr|$relCompanionLongSound");
      });
    } catch (e) {
      addLog("Companion Service discovery failed: $e", "ERROR");
    }
  }

  void _parseCompanionStatusData(List<int> value) {
    if (value.isEmpty) return;
    try {
      final dataStr = utf8.decode(value);
      
      if (dataStr.startsWith("LOG:")) {
        final logMsg = dataStr.substring(4);
        addLog("[Companion] $logMsg", "ROBOT");
        
        // Handle companion touch events (make primary respond)
        if (logMsg.startsWith("TOUCH_REL:")) {
          final payload = logMsg.substring(10);
          final parts = payload.split('|');
          if (parts.length >= 3) {
            final eventType = parts[0];
            final expr = int.tryParse(parts[1]) ?? 1;
            final sound = int.tryParse(parts[2]) ?? 2;
            _triggerRelationshipActionDirect(fromPrimary: false, eventType: eventType, expr: expr, sound: sound);
          }
        } else if (logMsg.startsWith("TOUCH:")) {
          final event = logMsg.substring(6); // TAP, DOUBLE, TRIPLE, LONG
          _triggerRelationshipAction(fromPrimary: false, eventType: event);
        }
        return;
      }
      
      final parts = dataStr.split(',');
      if (parts.length >= 3) {
        _companionUptimeSeconds = int.tryParse(parts[0]) ?? _companionUptimeSeconds;
        _companionTouchCount = int.tryParse(parts[1]) ?? _companionTouchCount;
        _companionBatteryVoltage = double.tryParse(parts[2]) ?? _companionBatteryVoltage;
        
        if (parts.length >= 4) {
          _companionActiveExpressionId = int.tryParse(parts[3]) ?? _companionActiveExpressionId;
        }
        if (parts.length >= 5) {
          _companionActiveExpressionLabel = parts[4].trim();
        }
        notifyListeners();
      }
    } catch (e) {
      print("Error decoding companion status packet: $e");
    }
  }

  void _handleCompanionDisconnect() {
    _isCompanionConnected = false;
    _isCompanionConnecting = false;
    _companionDevice = null;
    _companionExprChar = null;
    _companionAudioChar = null;
    _companionTextChar = null;
    _companionStatusChar = null;
    _companionAudioStreamChar = null;
    
    _companionUptimeSeconds = 0;
    _companionTouchCount = 0;
    _companionBatteryVoltage = 0.0;
    _companionActiveExpressionId = 0;
    _companionActiveExpressionLabel = "IDLE";

    _companionConnectionStateSub?.cancel();
    _companionStatusNotificationSub?.cancel();
    
    addLog("Companion disconnected.", "BLE");
    notifyListeners();
  }

  Future<void> disconnectCompanion() async {
    if (_companionDevice != null) {
      addLog("Disconnecting companion manually...", "BLE");
      await _companionDevice!.disconnect();
    }
    _handleCompanionDisconnect();
  }

  Future<void> transmitCompanionExpression(int expr, String label) async {
    if (!_isCompanionConnected || _companionExprChar == null) {
      addLog("Cannot transmit companion expression: Companion not connected.", "ERROR");
      return;
    }
    try {
      final labelBytes = utf8.encode(label.toUpperCase());
      final payload = Uint8List(1 + labelBytes.length);
      payload[0] = expr;
      payload.setRange(1, payload.length, labelBytes);
      
      await _companionExprChar!.write(payload, withoutResponse: false);
      _companionActiveExpressionId = expr;
      _companionActiveExpressionLabel = label;
      addLog("Sent companion expression: $expr ($label)", "BLE");
      notifyListeners();
    } catch (e) {
      addLog("Failed to write companion expression characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitCompanionAudio(int soundId) async {
    if (!_isCompanionConnected || _companionAudioChar == null) {
      addLog("Cannot transmit companion audio: Companion not connected.", "ERROR");
      return;
    }
    try {
      await _companionAudioChar!.write([soundId], withoutResponse: false);
      addLog("Sent companion audio trigger: SFX $soundId", "BLE");
    } catch (e) {
      addLog("Failed to write companion audio characteristic: $e", "ERROR");
    }
  }

  Future<void> _writePrimaryTextDirect(String text) async {
    if (!_isConnected || _textChar == null) return;
    try {
      final payload = utf8.encode(text);
      await _textChar!.write(payload, withoutResponse: false);
      addLog("Sent primary command: $text", "BLE");
    } catch (e) {
      addLog("Failed to write primary text characteristic: $e", "ERROR");
    }
  }

  Future<void> _writeCompanionTextDirect(String text) async {
    if (!_isCompanionConnected || _companionTextChar == null) return;
    try {
      final payload = utf8.encode(text);
      await _companionTextChar!.write(payload, withoutResponse: false);
      addLog("Sent companion command: $text", "BLE");
    } catch (e) {
      addLog("Failed to write companion text characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitCompanionNotification(String title, String body, {int? expr}) async {
    if (!_isCompanionConnected || _companionTextChar == null) {
      addLog("Cannot transmit companion notification: Companion not connected.", "ERROR");
      return;
    }
    try {
      final payloadStr = expr != null ? "NOTIF_EXPR:$title|$body|$expr" : "NOTIF:$title|$body";
      final payload = utf8.encode(payloadStr);
      await _companionTextChar!.write(payload, withoutResponse: false);
      addLog("Sent companion notification: $title - $body (Expr: $expr)", "BLE");
    } catch (e) {
      addLog("Failed to write companion text characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitPrimaryNotification(String title, String body, {int? expr}) async {
    if (!_isConnected || _textChar == null) {
      addLog("Cannot transmit primary notification: Not connected.", "ERROR");
      return;
    }
    try {
      final payloadStr = expr != null ? "NOTIF_EXPR:$title|$body|$expr" : "NOTIF:$title|$body";
      final payload = utf8.encode(payloadStr);
      await _textChar!.write(payload, withoutResponse: false);
      addLog("Sent primary notification: $title - $body (Expr: $expr)", "BLE");
    } catch (e) {
      addLog("Failed to write primary text characteristic: $e", "ERROR");
    }
  }
}

class _PendingAck {
  final String id;
  final String text;
  final Completer<bool> completer;
  final String description;
  int retryCount;
  Timer? timer;

  _PendingAck({
    required this.id,
    required this.text,
    required this.completer,
    required this.description,
    this.retryCount = 0,
  });
}
