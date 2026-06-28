import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter/material.dart';
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

  BluetoothDevice? _connectedDevice;
  bool _isConnected = false;
  bool _isScanning = false;
  List<ScanResult> _scanResults = [];
  
  // Reconnection and pairing states
  String? _pairedDeviceId;
  bool _isConnecting = false;
  Timer? _reconnectTimer;
  int _lastReconnectLogTime = 0;

  // Subscriptions
  StreamSubscription? _scanSub;
  StreamSubscription? _connectionStateSub;
  StreamSubscription? _statusNotificationSub;

  // Characteristics
  BluetoothCharacteristic? _exprChar;
  BluetoothCharacteristic? _audioChar;
  BluetoothCharacteristic? _textChar;
  BluetoothCharacteristic? _statusChar;

  // Diagnostics variables
  int _uptimeSeconds = 0;
  int _touchCount = 0;
  double _batteryVoltage = 0.0;
  int _activeExpressionId = 0;
  String _activeExpressionLabel = "IDLE";

  // Console log entries
  final List<String> _consoleLogs = [];

  // Server IP & Wi-Fi control helper variables
  String _serverIp = 'localhost';

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

  BLEService() {
    _initBLE();
    _loadPairedDevice();
    _startReconnectTimer();
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

  void _startReconnectTimer() {
    _reconnectTimer?.cancel();
    _reconnectTimer = Timer.periodic(const Duration(seconds: 1), (timer) async {
      if (_pairedDeviceId != null && !_isConnected && !_isConnecting && !_isScanning) {
        if (await FlutterBluePlus.adapterState.first == BluetoothAdapterState.on) {
          final now = DateTime.now().millisecondsSinceEpoch;
          if (now - _lastReconnectLogTime > 15000) {
            _lastReconnectLogTime = now;
            addLog("Checking connection status for paired robot...", "BLE");
          }

          List<BluetoothDevice> systemDevices = await FlutterBluePlus.systemDevices([Guid(serviceUuid)]);
          BluetoothDevice? targetDevice;
          for (var d in systemDevices) {
            if (d.remoteId.str == _pairedDeviceId) {
              targetDevice = d;
              break;
            }
          }

          if (targetDevice != null) {
            _lastReconnectLogTime = now;
            addLog("Paired robot found in system cache. Connecting...", "BLE");
            connect(targetDevice);
            return;
          }

          // 2. Perform a brief scan to discover if advertising
          _lastReconnectLogTime = now;
          addLog("Scanning to locate paired robot...", "BLE");
          _isScanning = true;
          notifyListeners();

          StreamSubscription? tempSub;
          tempSub = FlutterBluePlus.scanResults.listen((results) {
            for (var r in results) {
              if (r.device.remoteId.str == _pairedDeviceId) {
                addLog("Paired robot located! Connecting...", "BLE");
                tempSub?.cancel();
                FlutterBluePlus.stopScan();
                _isScanning = false;
                connect(r.device);
                break;
              }
            }
          });

          try {
            await FlutterBluePlus.startScan(
              timeout: const Duration(seconds: 3),
            );
          } catch (e) {
            print("Auto-reconnect scan failed: $e");
          }

          await Future.delayed(const Duration(seconds: 3));
          tempSub?.cancel();
          _isScanning = false;
          notifyListeners();
        }
      }
    });
  }

  void addLog(String message, String type) {
    final timestamp = DateTime.now().toLocal().toString().split(' ')[1].substring(0, 8);
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
    addLog("Scanning for Mr. Mario companion robot...", "BLE");
    notifyListeners();

    bool hasPermissions = await _requestPermissions();
    if (!hasPermissions) {
      _isScanning = false;
      notifyListeners();
      return;
    }

    try {
      _scanSub = FlutterBluePlus.scanResults.listen((results) {
        _scanResults = results.where((r) => r.device.platformName == 'Mr. Mario Robot' || r.advertisementData.serviceUuids.contains(Guid(serviceUuid))).toList();
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
      _connectionStateSub?.cancel();
      _connectionStateSub = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.connected) {
          _handleConnected(device);
        } else if (state == BluetoothConnectionState.disconnected) {
          _handleDisconnect();
        }
      });

      // Connect directly with standard timeout
      await device.connect(autoConnect: false, timeout: const Duration(seconds: 10));

      // Synchronously verify if it's already connected (or completed immediately)
      final currentState = await device.connectionState.first.timeout(
        const Duration(milliseconds: 500),
        onTimeout: () => BluetoothConnectionState.disconnected,
      );
      if (currentState == BluetoothConnectionState.connected) {
        _handleConnected(device);
      }
    } catch (e) {
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
    addLog("Connected to Mr. Mario successfully!", "BLE");
    _setupServices(device);
    notifyListeners();
  }

  Future<void> _setupServices(BluetoothDevice device) async {
    try {
      List<BluetoothService> services = await device.discoverServices();
      BluetoothService? targetService = services.firstWhere(
        (s) => s.uuid == Guid(serviceUuid),
      );

      for (var c in targetService.characteristics) {
        if (c.uuid == Guid(expressionCharUuid)) _exprChar = c;
        if (c.uuid == Guid(audioCharUuid)) _audioChar = c;
        if (c.uuid == Guid(textCharUuid)) _textChar = c;
        if (c.uuid == Guid(statusCharUuid)) _statusChar = c;
      }

      if (_statusChar != null) {
        await _statusChar!.setNotifyValue(true);
        _statusNotificationSub = _statusChar!.lastValueStream.listen((value) {
          _parseStatusData(value);
        });
        addLog("Status notifications enabled.", "BLE");
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
    
    addLog("Disconnected from Mr. Mario companion robot.", "BLE");
    notifyListeners();
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
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(text);
      if (!success) {
        addLog("Cannot transmit marquee text: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      final payload = utf8.encode(text);
      await _textChar!.write(payload, withoutResponse: false);
      addLog("Sent Marquee Text: '$text'", "BLE");
    } catch (e) {
      addLog("Failed to write text characteristic: $e", "ERROR");
    }
  }

  Future<void> syncClockToHardware() async {
    final now = DateTime.now();
    final hh = now.hour.toString().padLeft(2, '0');
    final mm = now.minute.toString().padLeft(2, '0');
    final ss = now.second.toString().padLeft(2, '0');
    final payloadStr = 'TIME:$hh:$mm:$ss';

    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Clock synced via Wi-Fi: $hh:$mm:$ss", "CLOCK");
      } else {
        addLog("Clock sync failed: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    
    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Clock synced via BLE: $hh:$mm:$ss", "CLOCK");
    } catch (e) {
      addLog("Clock sync failed: $e", "ERROR");
    }
  }

  Future<void> updateTimeFormat(bool is12H) async {
    final payloadStr = '12HR:${is12H ? 1 : 0}';
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Time format updated via Wi-Fi: ${is12H ? '12H' : '24H'}", "CLOCK");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Time format updated via BLE: ${is12H ? '12H' : '24H'}", "CLOCK");
    } catch (e) {
      addLog("Time format update failed: $e", "ERROR");
    }
  }

  Future<void> transmitCalendarEvent(String type, String time, String title) async {
    final payloadStr = 'CAL:$type,$time,$title';
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Transmitted calendar event via Wi-Fi: $type, $time, $title", "CALENDAR");
      } else {
        addLog("Cannot transmit calendar event: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Transmitted calendar event: $type, $time, $title", "CALENDAR");
    } catch (e) {
      addLog("Failed to transmit calendar event: $e", "ERROR");
    }
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
  }) async {
    final bleVal = bleEnabled ? "1" : "0";
    final negVal = negativeEnabled ? "1" : "0";
    final payloadStr = 'SET:$bleVal,$speedMs,$defaultGif,$introGif,$touchSingle,$touchDouble,$touchLong,$negVal,$introSpeedMs,$introSoundSpeed';

    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Synced settings to robot via Wi-Fi: $payloadStr", "SETTINGS");
      } else {
        addLog("Settings saved locally! Connect to Mr. Mario via BLE or Wi-Fi to sync.", "SETTINGS");
      }
      return;
    }

    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Synced settings to robot: $payloadStr", "BLE");
    } catch (e) {
      addLog("BLE settings sync failed: $e", "ERROR");
    }
  }

  Future<void> transmitResetDevice() async {
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand('RESET');
      if (success) {
        addLog("RESET command sent to Mr. Mario via Wi-Fi.", "BLE");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode('RESET'), withoutResponse: false);
      addLog("RESET command sent to Mr. Mario.", "BLE");
    } catch (e) {
      addLog("Failed to write reset command: $e", "ERROR");
    }
  }

  Future<void> transmitWifiConfig(String ssid, String password) async {
    final payloadStr = 'WIFI:$ssid,$password';
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Sent Wi-Fi credentials via Wi-Fi link: SSID: $ssid", "BLE");
      } else {
        addLog("Cannot configure Wi-Fi: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Sent Wi-Fi credentials for SSID: $ssid", "BLE");
    } catch (e) {
      addLog("Failed to write Wi-Fi config: $e", "ERROR");
    }
  }

  Future<void> transmitCompanionPairing(String macAddress, String relationship) async {
    final payloadStr = 'PAIR:$macAddress,$relationship';
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Sent companion pairing via Wi-Fi: MAC=$macAddress, Type=$relationship", "BLE");
      } else {
        addLog("Cannot pair companion: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      final payloadStr = 'PAIR:$macAddress,$relationship';
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Sent companion pairing: MAC=$macAddress, Type=$relationship", "BLE");
    } catch (e) {
      addLog("Failed to write companion pairing: $e", "ERROR");
    }
  }

  Future<void> transmitRelationship(String relationship) async {
    final payloadStr = 'RELATION:$relationship';
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand(payloadStr);
      if (success) {
        addLog("Sent relationship update via Wi-Fi: $relationship", "BLE");
      } else {
        addLog("Cannot set relationship: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode(payloadStr), withoutResponse: false);
      addLog("Sent relationship update: $relationship", "BLE");
    } catch (e) {
      addLog("Failed to write relationship update: $e", "ERROR");
    }
  }

  Future<void> transmitWake() async {
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand('WAKE');
      if (success) {
        addLog("Sent WAKE command to robot via Wi-Fi", "BLE");
      } else {
        addLog("Cannot send wake action: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode('WAKE'), withoutResponse: false);
      addLog("Sent WAKE command to robot", "BLE");
    } catch (e) {
      addLog("Failed to write wake command: $e", "ERROR");
    }
  }

  Future<void> transmitSleep() async {
    if (!_isConnected || _textChar == null) {
      final success = await _transmitWifiCommand('SLEEP');
      if (success) {
        addLog("Sent SLEEP command to robot via Wi-Fi", "BLE");
      } else {
        addLog("Cannot send sleep action: Not connected via BLE or Wi-Fi.", "ERROR");
      }
      return;
    }
    try {
      await _textChar!.write(utf8.encode('SLEEP'), withoutResponse: false);
      addLog("Sent SLEEP command to robot", "BLE");
    } catch (e) {
      addLog("Failed to write sleep command: $e", "ERROR");
    }
  }

  @override
  void dispose() {
    _scanSub?.cancel();
    _connectionStateSub?.cancel();
    _statusNotificationSub?.cancel();
    super.dispose();
  }
}
