import 'package:flutter/services.dart';
import 'bluetooth_service.dart';

class PhoneNotificationService {
  static const MethodChannel _channel = MethodChannel('com.mrmario/notifications');
  final BLEService _bleService;

  PhoneNotificationService(this._bleService) {
    _channel.setMethodCallHandler(_handleMethodCall);
  }

  Future<dynamic> _handleMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'onNotification':
        final Map<dynamic, dynamic> data = call.arguments as Map<dynamic, dynamic>;
        final String title = data['title'] ?? '';
        final String text = data['text'] ?? '';
        final String packageName = data['package'] ?? '';
        
        // Skip system/empty notifications
        if (title.isEmpty && text.isEmpty) return;
        
        // Forward notification to the robot!
        final String displayMessage = "${title.isNotEmpty ? '$title: ' : ''}$text";
        await _forwardToRobot(displayMessage);
        break;
    }
  }

  Future<void> _forwardToRobot(String message) async {
    // Trim to 100 characters to fit OLED scrolling buffers safely
    final safeMessage = message.length > 100 ? message.substring(0, 97) + '...' : message;
    
    if (_bleService.isConnected) {
      // Send raw notification text to robot over BLE (it plays SOUND_CHIRP and scrolls text)
      try {
        await _bleService.writeTextCharacteristic(safeMessage);
      } catch (e) {
        print("Failed to forward notification via BLE: $e");
      }
    } else {
      // Attempt to transmit via Wi-Fi if connected
      try {
        await _bleService.transmitWifiCommand(safeMessage);
      } catch (e) {
        print("Failed to forward notification via Wi-Fi: $e");
      }
    }
  }

  Future<bool> isPermissionGranted() async {
    try {
      final bool granted = await _channel.invokeMethod('isPermissionGranted');
      return granted;
    } on PlatformException catch (e) {
      print("Failed to check permission: $e");
      return false;
    }
  }

  Future<void> openSettings() async {
    try {
      await _channel.invokeMethod('openSettings');
    } on PlatformException catch (e) {
      print("Failed to open notification settings: $e");
    }
  }
}
