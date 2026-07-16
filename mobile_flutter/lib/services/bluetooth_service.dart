import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:http/http.dart' as http;

class BLEService with ChangeNotifier {
  static const String serviceUuid = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
  static const String expressionCharUuid = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
  static const String audioCharUuid = 'd90e0c03-51ee-4c31-893c-cf572db85700';
  static const String textCharUuid = 'c8a00d04-62ff-4b32-843d-0f1c6db8a101';
  static const String statusCharUuid = 'fb2f0e05-73ee-4f32-833d-1f2c6db8a102';
  static const String audioStreamCharUuid = 'a823e50b-71ee-48c5-9276-2e8c6db8a103';

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

  // Diagnostics variables
  int _uptimeSeconds = 0;
  int _touchCount = 0;
  double _batteryVoltage = 0.0;
  int _activeExpressionId = 0;
  String _activeExpressionLabel = "IDLE";

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
  int get activeExpressionId => _activeExpressionId;
  String get activeExpressionLabel => _activeExpressionLabel;
  List<String> get consoleLogs => _consoleLogs;
  String get serverIp => _serverIp;

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
        // ignore: unawaited_futures
        _audioStreamChar!.write(
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
      }

      if (_statusChar != null) {
        await _statusChar!.setNotifyValue(true);
        _statusNotificationSub = _statusChar!.lastValueStream.listen((value) {
          _parseStatusData(value);
        });
        addLog("Status notifications enabled.", "BLE");
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
      Future.delayed(const Duration(milliseconds: 800), () => syncClockToHardware());

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
  }

  // Characteristics Write Helpers
  Future<void> transmitExpression(int expr, String label) async {
    if (!_isConnected || _exprChar == null) {
      final success = await _transmitWifiCommand("EXPR:$expr,$label");
      if (success) {
        _activeExpressionId = expr;
        _activeExpressionLabel = label;
        notifyListeners();
      } else {
        addLog("Cannot transmit expression: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      final labelBytes = utf8.encode(label.toUpperCase());
      final payload = Uint8List(1 + labelBytes.length);
      payload[0] = expr;
      payload.setRange(1, payload.length, labelBytes);
      
      await _exprChar!.write(payload, withoutResponse: false);
      _activeExpressionId = expr;
      _activeExpressionLabel = label;
      addLog("Sent expression: $expr ($label)", "BLE");
      notifyListeners();
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
      await _audioChar!.write([soundId], withoutResponse: false);
      addLog("Sent audio trigger: SFX $soundId", "BLE");
    } catch (e) {
      addLog("Failed to write audio characteristic: $e", "ERROR");
    }
  }

  Future<void> transmitMarqueeText(String text) async {
    await _writeTextWithAck(text, "Marquee Text");
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
  }) async {
    final bleVal = bleEnabled ? "1" : "0";
    final negVal = negativeEnabled ? "1" : "0";
    final payloadStr = 'SET:$bleVal,$speedMs,$defaultGif,$introGif,$touchSingle,$touchDouble,$touchLong,$negVal,$introSpeedMs,$introSoundSpeed,$notificationDurationSec,$reminderDurationSec,$birthdayDurationSec,$clockStyle,$oledBrightness';
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

  Future<void> transmitWake() async {
    await _writeTextWithAck('WAKE', "Wake Robot");
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
