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
        final Map<dynamic, dynamic> data = call.arguments as Map<dynamic, dynamic>;
        final String title = data['title'] ?? '';
        final String text = data['text'] ?? '';
        final String subText = data['subText'] ?? '';
        final String bigText = data['bigText'] ?? '';
        final String packageName = data['package'] ?? '';

        _bleService.addLog("Recv Notif: pkg=$packageName, title='$title', text='$text', subText='$subText', bigText='$bigText', syncEnabled=${_dbService.notificationSyncEnabled}", "NOTIF");

        if (!_dbService.notificationSyncEnabled) return;
        
        // Skip system/empty notifications
        if (title.isEmpty && text.isEmpty && subText.isEmpty && bigText.isEmpty) return;

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

        _bleService.addLog("Checking if app key '$matchedAppKey' is allowed by user settings...", "NOTIF");
        if (!_dbService.allowedNotificationApps.contains(matchedAppKey)) {
          _bleService.addLog("App key '$matchedAppKey' is NOT allowed in settings.", "NOTIF");
          return;
        }

        // Google Maps Navigation Notification
        if (pkgLower == 'com.google.android.apps.maps') {
          _bleService.addLog("Parsing Google Maps navigation payload...", "NOTIF");
          final mapInfo = _parseGoogleMapsNotification(title, text, subText, bigText);
          if (mapInfo != null) {
            final String direction = mapInfo['direction']!;
            final String distance = mapInfo['distance']!;
            final String description = mapInfo['description']!;
            _bleService.addLog("Maps Parsed: dir=$direction, dist=$distance, desc=$description", "NOTIF");
            await _forwardToRobot("MAP:$direction,$distance,$description");
            break;
          } else {
            _bleService.addLog("Maps parsing returned null", "NOTIF");
          }
        }
        
        // Forward notification to the robot in detailed NOTIF:Title|Body format!
        final String titleClean = title.replaceAll('|', ' ').trim();
        final String textClean = text.replaceAll('|', ' ').trim();
        final String displayMessage = "NOTIF:$titleClean|$textClean";
        _bleService.addLog("Forwarding non-Maps notification to robot: $displayMessage", "NOTIF");
        await _forwardToRobot(displayMessage);
        break;

      case 'onNotificationRemoved':
        final Map<dynamic, dynamic> data = call.arguments as Map<dynamic, dynamic>;
        final String packageName = data['package'] ?? '';
        final pkgRemovedLower = packageName.toLowerCase();
        _bleService.addLog("Notification removed: pkg=$packageName, syncEnabled=${_dbService.notificationSyncEnabled}", "NOTIF");
        if (!_dbService.notificationSyncEnabled) return;
        if (pkgRemovedLower == 'com.google.android.apps.maps') {
          _bleService.addLog("Forwarding MAP:EXIT to robot", "NOTIF");
          await _forwardToRobot("MAP:EXIT");
        }
    }
  }

  Map<String, String>? _parseGoogleMapsNotification(String title, String text, String subText, String bigText) {
    final combined = "$title $text $subText $bigText".toLowerCase();
    
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

    // Clean "left" when it means "remaining" (e.g., "12 min left") to avoid matching it as a left turn instruction
    String cleanedForDirection = combined
        .replaceAll(RegExp(r'\b\d+\s*(?:min|mins|minute|minutes|hr|hrs|hour|hours|h)\s+left\b', caseSensitive: false), '')
        .replaceAll(RegExp(r'\bleft\b\s*•', caseSensitive: false), '')
        .replaceAll(RegExp(r'\bleft\b\s*$', caseSensitive: false), '');

    // 2. Extract Direction / Maneuver (UTURN and ROUNDABOUT first, then LEFT and RIGHT)
    String direction = "";
    final leftKeywords = ["left", "gauche", "links", "sinistra", "izquierda", "esquerda", "налево", "←", "↖", "↙", "lft", "turn left", "keep left", "bear left", "slight left", "बायें"];
    final rightKeywords = ["right", "droite", "rechts", "destra", "derecha", "direita", "направо", "→", "↗", "↘", "rgt", "turn right", "keep right", "bear right", "slight right", "दायें"];
    final uturnKeywords = ["u-turn", "uturn", "↶", "↷", "↺", "↻", "demi-tour", "wenden", "u turn"];
    final roundaboutKeywords = ["roundabout", "rotary", "exit", "⟳", "⟲", "rond-point", "kreisverkehr", "rotonda"];
    final straightKeywords = ["straight", "continue", "head north", "head south", "head east", "head west", "keep straight", "↑", "↓", "straighten"];

    if (uturnKeywords.any((k) => cleanedForDirection.contains(k))) {
      direction = "UTURN";
    } else if (roundaboutKeywords.any((k) => cleanedForDirection.contains(k))) {
      direction = "ROUNDABOUT";
    } else if (leftKeywords.any((k) => cleanedForDirection.contains(k))) {
      direction = "LEFT";
    } else if (rightKeywords.any((k) => cleanedForDirection.contains(k))) {
      direction = "RIGHT";
    } else if (straightKeywords.any((k) => cleanedForDirection.contains(k))) {
      direction = "STRAIGHT";
    }

    if (direction.isEmpty) {
      direction = "STRAIGHT";
    }

    // 3. Extract remaining time (e.g. "15 min", "1 hr 12 min", "1h 30m")
    final timeRemainingRegex = RegExp(
      r'\b\d+\s*(?:hr|hrs|hour|hours|h)\s*\d+\s*(?:min|mins|minutes)\b|\b\d+\s*(?:min|mins|minutes)\b|\b\d+\s*(?:hr|hrs|hour|hours|h)\b',
      caseSensitive: false
    );
    final timeMatch = timeRemainingRegex.firstMatch("$subText $bigText $text $title");
    String remainingTime = "";
    if (timeMatch != null) {
      remainingTime = timeMatch.group(0)!.trim().toLowerCase();
      // Format to short representation, e.g. "1h 15m" or "15min"
      remainingTime = remainingTime
        .replaceAll(RegExp(r'\s*(?:minutes|minute|mins)\b'), 'min')
        .replaceAll(RegExp(r'\s*(?:hours|hour|hrs|hr)\b'), 'h')
        .replaceAll(RegExp(r'\s+'), '');
    }

    return {
      'direction': direction,
      'distance': distance.isNotEmpty ? distance : "--",
      'description': remainingTime.isNotEmpty ? remainingTime : "",
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
