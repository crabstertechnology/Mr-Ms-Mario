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
    // From live logs: Google Maps sends text='Turn right', text='Turn left', text='Head west'
    // subText='19 min · 8.2 km · 4:18 pm ETA'
    // title is often empty during navigation

    final combined = "$title $text $subText $bigText".toLowerCase();

    // ── 1. DISTANCE ──────────────────────────────────────────────────────────
    // Use ALL fields so we get the km value from subText when title is empty.
    // e.g. subText='19 min · 8.2 km · ...' → shows "8.2 KM" as remaining distance
    final distanceRegex = RegExp(r'\b(\d+(?:[.,]\d+)?)\s*(m|km|ft|mi|meters|kilometers|feet|miles|yards|yd)\b');
    final distMatch = distanceRegex.firstMatch(combined);
    String distance = "--";
    if (distMatch != null) {
      String num  = distMatch.group(1)!;
      String unit = distMatch.group(2)!.toLowerCase();
      // Normalize unit
      if (unit == "meters")     unit = "m";
      if (unit == "kilometers") unit = "km";
      if (unit == "feet")       unit = "ft";
      if (unit == "miles")      unit = "mi";
      if (unit == "yards")      unit = "yd";
      distance = "$num $unit".toUpperCase();
    }
    _bleService.addLog("Maps dist: '$distance'", "NOTIF");

    // ── 2. DIRECTION ─────────────────────────────────────────────────────────
    // Use the text field directly (most reliable per live logs: text='Turn right')
    // Strip any "N min left" phrases first to prevent false LEFT matches
    final cleanedText = combined
        .replaceAll(RegExp(r'\b\d+\s*(?:min|mins|minute|minutes|hr|hrs|h)\s+left\b', caseSensitive: false), '')
        .replaceAll(RegExp(r'\bleft\b(?=\s*[·•])', caseSensitive: false), '');

    String direction = "STRAIGHT";

    if (RegExp(r'\bu.?turn\b').hasMatch(cleanedText)) {
      direction = "UTURN";
    } else if (RegExp(r'\b(roundabout|rotary|rond.point)\b').hasMatch(cleanedText)) {
      direction = "ROUNDABOUT";
    } else if (RegExp(r'\bright\b').hasMatch(cleanedText)) {
      // Check RIGHT before LEFT to avoid false positives
      direction = "RIGHT";
    } else if (RegExp(r'\bleft\b').hasMatch(cleanedText)) {
      direction = "LEFT";
    } else if (RegExp(r'\b(straight|continue|head\s+(north|south|east|west))\b').hasMatch(cleanedText)) {
      direction = "STRAIGHT";
    }

    _bleService.addLog("Maps dir: '$direction' from: '$cleanedText'", "NOTIF");

    // ── 3. REMAINING TIME ────────────────────────────────────────────────────
    final timeRegex = RegExp(
      r'\b\d+\s*(?:hr|hrs|hour|hours|h)\s*\d+\s*(?:min|mins|minutes)\b|\b\d+\s*(?:min|mins|minutes)\b|\b\d+\s*(?:hr|hrs|hour|hours|h)\b',
      caseSensitive: false,
    );
    final timeMatch = timeRegex.firstMatch("$subText $bigText $text $title");
    String remainingTime = "";
    if (timeMatch != null) {
      remainingTime = timeMatch.group(0)!.trim().toLowerCase()
          .replaceAll(RegExp(r'\s*(?:minutes|minute|mins)\b'), 'min')
          .replaceAll(RegExp(r'\s*(?:hours|hour|hrs|hr)\b'), 'h')
          .replaceAll(RegExp(r'\s+'), '');
    }

    return {
      'direction': direction,
      'distance': distance,
      'description': remainingTime,
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
