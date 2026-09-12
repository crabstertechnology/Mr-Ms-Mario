import 'package:flutter/services.dart';
import 'bluetooth_service.dart';
import 'database_service.dart';

class PhoneNotificationService {
  static const MethodChannel _channel = MethodChannel('com.mrmsluna/notifications');
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
        final String smallIcon = data['smallIcon'] ?? '';
        final String directionFromIcon = data['directionFromIcon'] ?? '';

        _bleService.addLog("Recv Notif: pkg=$packageName, title='$title', text='$text', subText='$subText', bigText='$bigText', smallIcon='$smallIcon', directionFromIcon='$directionFromIcon', syncEnabled=${_dbService.notificationSyncEnabled}", "NOTIF");

        if (!_dbService.notificationSyncEnabled) return;
        
        // Skip system/empty notifications
        if (title.isEmpty && text.isEmpty && subText.isEmpty && bigText.isEmpty) return;

        final pkgLower = packageName.toLowerCase();

        // ── 0. NAVIGATION APPS TELEMETRY (Google Maps, etc.) ──────────────────
        // Intercept navigation apps IMMEDIATELY!
        // 1. Navigation is a core system service: bypass settings whitelist.
        // 2. Transmit MAP: packet directly to show on hardware display.
        // 3. NEVER fall through to NOTIF: so it NEVER shows as a screen card popup!
        final isNavApp = pkgLower == 'com.google.android.apps.maps' ||
            pkgLower.contains('ola') ||
            pkgLower.contains('mappls') ||
            pkgLower.contains('mapmyindia') ||
            pkgLower.contains('waze') ||
            pkgLower.contains('maps');

        if (isNavApp) {
          _bleService.addLog("Handling navigation telemetry for '$pkgLower'...", "NOTIF");
          final mapInfo = _parseGoogleMapsNotification(title, text, subText, bigText, smallIcon, directionFromIcon);
          final String direction    = mapInfo['direction'] ?? 'STRAIGHT';
          final String turnDistance = mapInfo['turnDistance'] ?? '--';
          final String road         = mapInfo['road'] ?? '';
          final String totalTime    = mapInfo['totalTime'] ?? '';
          final String totalDist    = mapInfo['totalDist'] ?? '';
          final String eta          = mapInfo['eta'] ?? '';

          _bleService.addLog("Nav Telemetry: dir=$direction, turn=$turnDistance, road='$road', time=$totalTime, dist=$totalDist, eta=$eta", "NOTIF");

          // Update local state for simulator / dashboard
          final summary = totalTime.isNotEmpty && totalDist.isNotEmpty
              ? "$totalTime · $totalDist"
              : (totalTime.isNotEmpty ? totalTime : eta);
          _bleService.updateNavigation(
            direction,
            turnDistance,
            summary,
            road: road,
            totalTime: totalTime,
            totalDist: totalDist,
            eta: eta,
          );

          // Fast direct unblocked BLE transmission (sub-5ms)
          final navPayload = "MAP:$direction,$turnDistance,$road,$totalTime,$totalDist,$eta";
          await _bleService.transmitNavTelemetry(navPayload);
          return; // CRITICAL: NEVER fall through to NOTIF: (screen card)
        }

        // Check if the notification's app is allowed by user settings
        bool isAllowed = _dbService.allowedNotificationApps.contains(pkgLower);

        // Also check preset mapping for backward compatibility and convenience
        if (!isAllowed) {
          String matchedAppKey = '';
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

          if (matchedAppKey.isNotEmpty && _dbService.allowedNotificationApps.contains(matchedAppKey)) {
            isAllowed = true;
          }
        }

        // Support fallback to 'other_apps' if specified
        if (!isAllowed && _dbService.allowedNotificationApps.contains('other_apps')) {
          isAllowed = true;
        }

        _bleService.addLog("Checking if app '$pkgLower' is allowed by user settings: $isAllowed", "NOTIF");
        if (!isAllowed) {
          _bleService.addLog("App '$pkgLower' is NOT allowed in settings.", "NOTIF");
          return;
        }

        // Forward non-navigation notification to the robot in detailed NOTIF:Title|Body format!
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
        final isRemovedNavApp = pkgRemovedLower == 'com.google.android.apps.maps' ||
            pkgRemovedLower.contains('ola') ||
            pkgRemovedLower.contains('mappls') ||
            pkgRemovedLower.contains('mapmyindia') ||
            pkgRemovedLower.contains('waze') ||
            pkgRemovedLower.contains('maps');
        if (isRemovedNavApp) {
          _bleService.addLog("Navigation finished: Forwarding MAP:EXIT to robot", "NOTIF");
          _bleService.clearNavigation();
          await _bleService.transmitNavTelemetry("MAP:EXIT");
        }
    }
  }

  Map<String, String> _parseGoogleMapsNotification(String title, String text, String subText, String bigText, String smallIcon, String directionFromIcon) {
    // Clean all non-breaking spaces, narrow spaces, and tabs
    final cleanTitle = title.replaceAll('\u00a0', ' ').replaceAll('\u202f', ' ').replaceAll(',', ' ').trim();
    final cleanText = text.replaceAll('\u00a0', ' ').replaceAll('\u202f', ' ').replaceAll(',', ' ').trim();
    final cleanSubText = subText.replaceAll('\u00a0', ' ').replaceAll('\u202f', ' ').replaceAll(',', ' ').trim();
    final cleanBigText = bigText.replaceAll('\u00a0', ' ').replaceAll('\u202f', ' ').replaceAll(',', ' ').trim();

    final combined = "$cleanTitle $cleanText $cleanSubText $cleanBigText".toLowerCase();

    // ── 1. NEXT TURN DISTANCE (e.g. "0 m", "200 m", "1.5 km") ──────────────
    final distRegex = RegExp(r'\b(\d+(?:[.,]\d+)?)\s*(m|km|ft|mi|meters|kilometers|feet|miles|yards|yd)\b', caseSensitive: false);
    var turnDistMatch = distRegex.firstMatch("$cleanTitle $cleanText");
    String turnDistance = "--";
    if (turnDistMatch != null) {
      String num = turnDistMatch.group(1)!;
      String unit = turnDistMatch.group(2)!.toLowerCase();
      if (unit == "meters") unit = "m";
      if (unit == "kilometers") unit = "km";
      if (unit == "feet") unit = "ft";
      if (unit == "miles") unit = "mi";
      if (unit == "yards") unit = "yd";
      turnDistance = "$num $unit".toUpperCase();
    }

    // ── 2. TOTAL REMAINING DISTANCE (e.g. "23 km" from subText) ─────────────
    var totalDistMatch = distRegex.firstMatch("$cleanSubText $cleanBigText");
    String totalDistance = "";
    if (totalDistMatch != null) {
      String num = totalDistMatch.group(1)!;
      String unit = totalDistMatch.group(2)!.toLowerCase();
      if (unit == "meters") unit = "m";
      if (unit == "kilometers") unit = "km";
      if (unit == "feet") unit = "ft";
      if (unit == "miles") unit = "mi";
      if (unit == "yards") unit = "yd";
      totalDistance = "$num $unit".toUpperCase();
    }
    if (turnDistance == "--" && totalDistance.isNotEmpty) {
      turnDistance = totalDistance;
    }

    // ── 3. TOTAL REMAINING TIME (e.g. "46 min", "1 hr 15 min") ──────────────
    final durRegex = RegExp(
      r'\b(?:\d+\s*(?:hr|hrs|hour|hours|h)\s*)?\d+\s*(?:min|mins|minutes)\b|\b\d+\s*(?:hr|hrs|hour|hours|h)\b',
      caseSensitive: false,
    );
    final durMatch = durRegex.firstMatch("$cleanSubText $cleanBigText $cleanTitle $cleanText");
    String totalTime = "";
    if (durMatch != null) {
      totalTime = durMatch.group(0)!.trim().toUpperCase()
          .replaceAll(RegExp(r'\s*(?:MINUTES|MINS)\b'), ' MIN')
          .replaceAll(RegExp(r'\s*(?:HOURS|HRS|HR)\b'), ' H');
    }

    // ── 4. ETA CLOCK TIME (e.g. "11:28 AM") ──────────────────────────────────
    final etaRegex = RegExp(
      r'\b\d{1,2}:\d{2}(?:\s*[AaPp][Mm])?\b',
    );
    final etaMatch = etaRegex.firstMatch("$cleanSubText $cleanBigText $cleanTitle $cleanText");
    String eta = "";
    if (etaMatch != null) {
      eta = etaMatch.group(0)!.trim().toUpperCase();
    }

    // ── 5. ROAD / INSTRUCTION LABEL ──────────────────────────────────────────
    String road = "";
    if (cleanText.isNotEmpty && !cleanText.contains(RegExp(r'^\d+\s*[a-zA-Z]+$'))) {
      road = cleanText;
    } else if (cleanTitle.isNotEmpty && !cleanTitle.contains(RegExp(r'^\d+\s*[a-zA-Z]+$'))) {
      road = cleanTitle;
    }

    // ── 6. DIRECTION ─────────────────────────────────────────────────────────
    final cleanedText = combined
        .replaceAll(RegExp(r'\b\d+\s*(?:min|mins|minute|minutes|hr|hrs|h)\s+left\b', caseSensitive: false), '')
        .replaceAll(RegExp(r'\bleft\b(?=\s*[·•])', caseSensitive: false), '');

    String direction = "STRAIGHT";
    final smallIconLower = smallIcon.toLowerCase();

    if (directionFromIcon == "LEFT" || directionFromIcon == "RIGHT") {
      direction = directionFromIcon;
    } else if (smallIconLower.contains("left") && !smallIconLower.contains("right")) {
      direction = "LEFT";
    } else if (smallIconLower.contains("right") && !smallIconLower.contains("left")) {
      direction = "RIGHT";
    } else if (smallIconLower.contains("uturn") || smallIconLower.contains("u_turn")) {
      direction = "UTURN";
    } else if (smallIconLower.contains("roundabout")) {
      direction = "ROUNDABOUT";
    } else if (smallIconLower.contains("straight") || smallIconLower.contains("continue") || smallIconLower.contains("keep_ahead") || smallIconLower.contains("keep_straight")) {
      direction = "STRAIGHT";
    } else {
      if (RegExp(r'\bu.?turn\b').hasMatch(cleanedText)) {
        direction = "UTURN";
      } else if (RegExp(r'\b(roundabout|rotary|rond.point)\b').hasMatch(cleanedText)) {
        direction = "ROUNDABOUT";
      } else {
        final int leftIdx = cleanedText.indexOf('left');
        final int rightIdx = cleanedText.indexOf('right');
        if (leftIdx != -1 && rightIdx != -1) {
          direction = leftIdx < rightIdx ? "LEFT" : "RIGHT";
        } else if (leftIdx != -1) {
          direction = "LEFT";
        } else if (rightIdx != -1) {
          direction = "RIGHT";
        } else if (RegExp(r'\b(straight|continue|head\s+(north|south|east|west))\b').hasMatch(cleanedText)) {
          direction = "STRAIGHT";
        } else if (directionFromIcon.isNotEmpty) {
          direction = directionFromIcon;
        }
      }
    }

    return {
      'direction': direction,
      'turnDistance': turnDistance,
      'road': road,
      'totalTime': totalTime,
      'totalDist': totalDistance,
      'eta': eta,
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
