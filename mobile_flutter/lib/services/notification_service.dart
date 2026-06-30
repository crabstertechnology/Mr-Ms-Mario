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

        // Check if the notification's app is allowed by user settings
        String matchedAppKey = 'other_apps';
        final pkgLower = packageName.toLowerCase();
        if (pkgLower == 'com.whatsapp') {
          matchedAppKey = 'whatsapp';
        } else if (pkgLower == 'com.whatsapp.w4b') {
          matchedAppKey = 'whatsapp_business';
        } else if (pkgLower == 'com.instagram.android') {
          matchedAppKey = 'instagram';
        } else if (pkgLower == 'com.snapchat.android') {
          matchedAppKey = 'snapchat';
        } else if (pkgLower == 'org.telegram.messenger') {
          matchedAppKey = 'telegram';
        } else if (pkgLower == 'com.facebook.orca') {
          matchedAppKey = 'messenger';
        } else if (pkgLower == 'com.google.android.apps.maps') {
          matchedAppKey = 'google_maps';
        } else if (pkgLower == 'com.google.android.gm') {
          matchedAppKey = 'gmail';
        } else if (pkgLower == 'com.google.android.youtube') {
          matchedAppKey = 'youtube';
        } else if (pkgLower.contains('messaging') || pkgLower.contains('sms') || pkgLower.contains('mms')) {
          matchedAppKey = 'sms';
        } else if (pkgLower.contains('dialer') || pkgLower.contains('telecom') || pkgLower.contains('phone') || pkgLower.contains('incallui')) {
          matchedAppKey = 'phone';
        }

        if (!_dbService.allowedNotificationApps.contains(matchedAppKey)) {
          return;
        }

        // Google Maps Navigation Notification
        if (packageName == 'com.google.android.apps.maps') {
          final mapInfo = _parseGoogleMapsNotification(title, text);
          if (mapInfo != null) {
            final String direction = mapInfo['direction']!;
            final String distance = mapInfo['distance']!;
            final String description = mapInfo['description']!;
            await _forwardToRobot("MAP:$direction,$distance,$description");
            break;
          }
        }
        
        // Forward notification to the robot in detailed NOTIF:Title|Body format!
        final String titleClean = title.replaceAll('|', ' ').trim();
        final String textClean = text.replaceAll('|', ' ').trim();
        final String displayMessage = "NOTIF:$titleClean|$textClean";
        await _forwardToRobot(displayMessage);
        break;

      case 'onNotificationRemoved':
        if (!_dbService.notificationSyncEnabled) return;
        final Map<dynamic, dynamic> data = call.arguments as Map<dynamic, dynamic>;
        final String packageName = data['package'] ?? '';
        if (packageName == 'com.google.android.apps.maps') {
          await _forwardToRobot("MAP:EXIT");
        }
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
    
    // Clean distance format: e.g. "500 M" -> "500 m"
    distance = distance.replaceAll(" ", "").toLowerCase();
    if (distance.endsWith("meters")) distance = distance.replaceAll("meters", "m");
    if (distance.endsWith("kilometers")) distance = distance.replaceAll("kilometers", "km");
    if (distance.endsWith("feet")) distance = distance.replaceAll("feet", "ft");
    if (distance.endsWith("miles")) distance = distance.replaceAll("miles", "mi");
    // Format distance nicely with space, e.g. "500m" -> "500 m", "1.2km" -> "1.2 km"
    final spaceMatch = RegExp(r'^(\d+(?:[\.,]\d+)?)([a-zA-Z]+)$').firstMatch(distance);
    if (spaceMatch != null) {
      distance = "${spaceMatch.group(1)} ${spaceMatch.group(2)}";
    }
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

    if (direction.isEmpty) {
      direction = "STRAIGHT";
    }

    // 3. Extract description (street/instruction)
    String description = title;
    if (description.isEmpty || description == "Google Maps" || description == "Notification") {
      description = text;
    }
    // Clean distance and time from description
    description = description.replaceAll(distanceRegex, "").trim();
    description = description.replaceAll(RegExp(r'(?i)\bin\b\s*\d+\s*(?:m|km|ft|mi|yards|yd|meters|kilometers)\b,?\s*'), "").trim();
    description = description.replaceAll(RegExp(r'(?i)\bIn\b\s*\d+\s*(?:m|km|ft|mi|yards|yd|meters|kilometers)\b,?\s*'), "").trim();
    description = description.replaceAll(RegExp(r'\s*-\s*\d+\s*(?:min|mins|hr|hrs|hour|hours).*'), "").trim();
    // Strip trailing/leading punctuation
    description = description.trim();
    if (description.endsWith(",") || description.endsWith(".") || description.endsWith("-")) {
      description = description.substring(0, description.length - 1).trim();
    }

    return {
      'direction': direction,
      'distance': distance.isNotEmpty ? distance : "--",
      'description': description.isNotEmpty ? description : "Maps Navigation",
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
