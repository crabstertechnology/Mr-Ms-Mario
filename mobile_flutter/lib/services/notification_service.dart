import 'package:flutter/services.dart';
import 'bluetooth_service.dart';
import 'database_service.dart';

class PhoneNotificationService {
  static const MethodChannel _channel = MethodChannel('com.mrmario/notifications');
  final BLEService _bleService;
  final DatabaseService _dbService;

  PhoneNotificationService(this._bleService, this._dbService) {
    _channel.setMethodCallHandler(_handleMethodCall);
  }

  Future<dynamic> _handleMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'onNotification':
        if (!_dbService.notificationSyncEnabled) return;
        final Map<dynamic, dynamic> data = call.arguments as Map<dynamic, dynamic>;
        final String title = data['title'] ?? '';
        final String text = data['text'] ?? '';
        final String packageName = data['package'] ?? '';
        
        // Skip system/empty notifications
        if (title.isEmpty && text.isEmpty) return;

        // Google Maps Navigation Notification
        if (packageName == 'com.google.android.apps.maps') {
          final mapInfo = _parseGoogleMapsNotification(title, text);
          if (mapInfo != null) {
            final String direction = mapInfo['direction']!;
            final String distance = mapInfo['distance']!;
            await _forwardToRobot("MAP:$direction,$distance");
            break;
          }
        }
        
        // Forward notification to the robot in detailed NOTIF:Title|Body format!
        final String titleClean = title.replaceAll('|', ' ').trim();
        final String textClean = text.replaceAll('|', ' ').trim();
        final String displayMessage = "NOTIF:$titleClean|$textClean";
        await _forwardToRobot(displayMessage);
        break;
    }
  }

  Map<String, String>? _parseGoogleMapsNotification(String title, String text) {
    final combined = "$title $text".toLowerCase();
    
    // 1. Extract Distance / Meter
    final distanceRegex = RegExp(r'\b\d+(?:[\.,]\d+)?\s*(?:m|km|ft|mi|yards|yd|meters|kilometers|feet|miles)\b');
    final match = distanceRegex.firstMatch(combined);
    String distance = "";
    if (match != null) {
      distance = match.group(0)!.toUpperCase();
    }
    
    // Clean distance format: e.g. "500 M" -> "500M"
    distance = distance.replaceAll(" ", "").toLowerCase();
    if (distance.endsWith("meters")) distance = distance.replaceAll("meters", "m");
    if (distance.endsWith("kilometers")) distance = distance.replaceAll("kilometers", "km");
    if (distance.endsWith("feet")) distance = distance.replaceAll("feet", "ft");
    if (distance.endsWith("miles")) distance = distance.replaceAll("miles", "mi");
    distance = distance.toUpperCase();

    // 2. Extract Direction / Maneuver
    String direction = "";
    if (combined.contains("turn left") || combined.contains("take left") || combined.contains("keep left") || combined.contains("slight left")) {
      direction = "LEFT";
    } else if (combined.contains("turn right") || combined.contains("take right") || combined.contains("keep right") || combined.contains("slight right")) {
      direction = "RIGHT";
    } else if (combined.contains("u-turn") || combined.contains("uturn") || combined.contains("make a u-turn")) {
      direction = "UTURN";
    } else if (combined.contains("roundabout") || combined.contains("exit")) {
      direction = "ROUNDABOUT";
    } else if (combined.contains("straight") || combined.contains("continue") || combined.contains("head north") || combined.contains("head south") || combined.contains("head east") || combined.contains("head west") || combined.contains("keep straight")) {
      direction = "STRAIGHT";
    }

    // Default to STRAIGHT if distance is found but no specific turn word is matched
    if (direction.isEmpty && distance.isNotEmpty) {
      direction = "STRAIGHT";
    }

    if (direction.isEmpty && distance.isEmpty) {
      return null;
    }

    return {
      'direction': direction,
      'distance': distance.isNotEmpty ? distance : "--",
    };
  }

  Future<void> _forwardToRobot(String message) async {
    // Trim to 100 characters to fit OLED scrolling buffers safely
    final safeMessage = message.length > 100 ? message.substring(0, 97) + '...' : message;
    
    if (_bleService.isConnected) {
      try {
        await _bleService.transmitMarqueeText(safeMessage);
      } catch (e) {
        print("Failed to forward notification via BLE: $e");
      }
    } else {
      // Attempt to transmit via Wi-Fi if connected
      try {
        await _bleService.transmitMarqueeText(safeMessage);
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

  Future<bool> isPostNotificationsPermissionGranted() async {
    try {
      final bool granted = await _channel.invokeMethod('isPostNotificationsPermissionGranted');
      return granted;
    } on PlatformException catch (e) {
      print("Failed to check post notification permission: $e");
      return false;
    }
  }

  Future<void> requestPostNotificationsPermission() async {
    try {
      await _channel.invokeMethod('requestPostNotificationsPermission');
    } on PlatformException catch (e) {
      print("Failed to request post notification permission: $e");
    }
  }

  Future<void> startBackgroundService() async {
    try {
      await _channel.invokeMethod('startBackgroundService');
    } on PlatformException catch (e) {
      print("Failed to start background service: $e");
    }
  }
}
