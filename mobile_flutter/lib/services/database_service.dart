import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import '../models/gif_model.dart';
import '../models/robot_profile.dart';
import '../models/calendar_event.dart';
import '../models/alarm_model.dart';

class DatabaseService with ChangeNotifier {
  List<GifModel> _gifs = [];
  List<RobotProfile> _robots = [];
  List<CalendarEvent> _events = [];
  List<AlarmModel> _alarms = [];
  SharedPreferences? _prefs;

  // Settings cached values
  bool _is12HourFormat = false;
  bool _notificationSyncEnabled = true;
  double _gifSpeed = 100.0;
  double _gifDelay = 0.0;
  double _gifIntroSpeed = 100.0;
  double _introSoundSpeed = 100.0;
  bool _negativeEnabled = false;
  String _defaultGif = 'default';
  String _introGif = 'default';
  String _touchSingle = 'default';
  String _touchDouble = 'default';
  String _touchLong = 'default';
  double _oledBrightness = 1.0;
  double _oledContrast = 1.0;
  bool _oledInvert = false;
  double _oledRotation = 0.0;
  int _clockStyle = 0;
  double _notificationDuration = 5.0;
  double _reminderDuration = 10.0;
  double _birthdayDuration = 15.0;
  double _bleSleepTime = 45.0;
  String _bleName = 'Mr. Luna Robot';
  List<String> _allowedNotificationApps = [
    'whatsapp',
    'whatsapp_business',
    'instagram',
    'snapchat',
    'telegram',
    'messenger',
    'google_maps',
    'gmail',
    'youtube',
    'sms',
    'phone',
    'other_apps'
  ];

  List<GifModel> get gifs => _gifs;
  List<RobotProfile> get robots => _robots;
  List<CalendarEvent> get events => _events;
  List<AlarmModel> get alarms => _alarms;
  bool get is12HourFormat => _is12HourFormat;
  bool get notificationSyncEnabled => _notificationSyncEnabled;
  List<String> get allowedNotificationApps => _allowedNotificationApps;

  RobotProfile? get primaryRobot {
    if (_robots.isEmpty) return null;
    final primary = _robots.where((r) => r.isPrimary);
    if (primary.isNotEmpty) return primary.first;
    return _robots.first;
  }

  // Getters for settings
  double get gifSpeed => _gifSpeed;
  double get gifDelay => _gifDelay;
  double get gifIntroSpeed => _gifIntroSpeed;
  double get introSoundSpeed => _introSoundSpeed;
  bool get negativeEnabled => _negativeEnabled;
  String get defaultGif => _defaultGif;
  String get introGif => _introGif;
  String get touchSingle => _touchSingle;
  String get touchDouble => _touchDouble;
  String get touchLong => _touchLong;
  double get oledBrightness => _oledBrightness;
  double get oledContrast => _oledContrast;
  bool get oledInvert => _oledInvert;
  double get oledRotation => _oledRotation;
  int get clockStyle => _clockStyle;
  double get notificationDuration => _notificationDuration;
  double get reminderDuration => _reminderDuration;
  double get birthdayDuration => _birthdayDuration;
  double get bleSleepTime => _bleSleepTime;
  String get bleName => _bleName;

  // 63 original eye animations mapping
  static const Map<String, Map<String, dynamic>> animMapping = {
    "adore": { "expr": 1, "sound": 2, "label": "Adore", "cat": "Adore" },
    "angry": { "expr": 3, "sound": 5, "label": "Angry", "cat": "Angry" },
    "blank": { "expr": 0, "sound": 0, "label": "Blank", "cat": "Blank" },
    "blinding": { "expr": 4, "sound": 6, "label": "Blinding", "cat": "Blinding" },
    "brave": { "expr": 1, "sound": 3, "label": "Brave", "cat": "Brave" },
    "buzzing": { "expr": 4, "sound": 6, "label": "Buzzing", "cat": "Buzzing" },
    "contempt": { "expr": 3, "sound": 4, "label": "Contempt", "cat": "Contempt" },
    "crying": { "expr": 2, "sound": 4, "label": "Crying", "cat": "Crying" },
    "dancing": { "expr": 1, "sound": 2, "label": "Dancing", "cat": "Dancing" },
    "devil": { "expr": 3, "sound": 5, "label": "Devil", "cat": "Devil" },
    "distracted": { "expr": 0, "sound": 6, "label": "Distracted", "cat": "Distracted" },
    "dizzy": { "expr": 4, "sound": 6, "label": "Dizzy", "cat": "Dizzy" },
    "down": { "expr": 2, "sound": 4, "label": "Down", "cat": "Down" },
    "drowsy": { "expr": 5, "sound": 0, "label": "Drowsy", "cat": "Drowsy" },
    "encouragement": { "expr": 1, "sound": 3, "label": "Encouragement", "cat": "Encouragement" },
    "energetic": { "expr": 1, "sound": 3, "label": "Energetic", "cat": "Energetic" },
    "enraged": { "expr": 3, "sound": 5, "label": "Enraged", "cat": "Enraged" },
    "evil": { "expr": 3, "sound": 5, "label": "Evil", "cat": "Evil" },
    "fast": { "expr": 1, "sound": 1, "label": "Fast", "cat": "Fast" },
    "fierce": { "expr": 3, "sound": 5, "label": "Fierce", "cat": "Fierce" },
    "furious": { "expr": 3, "sound": 5, "label": "Furious", "cat": "Furious" },
    "giggle": { "expr": 1, "sound": 2, "label": "Giggle", "cat": "Giggle" },
    "glowing": { "expr": 1, "sound": 2, "label": "Glowing", "cat": "Glowing" },
    "growing": { "expr": 1, "sound": 3, "label": "Growing", "cat": "Growing" },
    "handsome": { "expr": 1, "sound": 2, "label": "Handsome", "cat": "Handsome" },
    "happy": { "expr": 1, "sound": 2, "label": "Happy", "cat": "Happy" },
    "hello": { "expr": 1, "sound": 2, "label": "Hello", "cat": "Hello" },
    "irritated": { "expr": 3, "sound": 5, "label": "Irritated", "cat": "Irritated" },
    "laughing": { "expr": 1, "sound": 2, "label": "Laughing", "cat": "Laughing" },
    "left": { "expr": 0, "sound": 0, "label": "Look Left", "cat": "Look Left" },
    "love": { "expr": 1, "sound": 2, "label": "Love", "cat": "Love" },
    "menacing": { "expr": 3, "sound": 5, "label": "Menacing", "cat": "Menacing" },
    "mistake": { "expr": 4, "sound": 4, "label": "Mistake", "cat": "Mistake" },
    "playful": { "expr": 1, "sound": 2, "label": "Playful", "cat": "Playful" },
    "police": { "expr": 4, "sound": 6, "label": "Police", "cat": "Police" },
    "rain": { "expr": 2, "sound": 4, "label": "Rain", "cat": "Rain" },
    "relaxed": { "expr": 0, "sound": 0, "label": "Relaxed", "cat": "Relaxed" },
    "right": { "expr": 0, "sound": 0, "label": "Look Right", "cat": "Look Right" },
    "rush": { "expr": 1, "sound": 1, "label": "Rush", "cat": "Rush" },
    "scared": { "expr": 4, "sound": 6, "label": "Scared", "cat": "Scared" },
    "serene": { "expr": 5, "sound": 0, "label": "Serene", "cat": "Serene" },
    "shrink": { "expr": 2, "sound": 4, "label": "Shrink", "cat": "Shrink" },
    "shy": { "expr": 6, "sound": 6, "label": "Shy", "cat": "Shy" },
    "sick": { "expr": 2, "sound": 4, "label": "Sick", "cat": "Sick" },
    "sleepy": { "expr": 5, "sound": 0, "label": "Sleepy", "cat": "Sleepy" },
    "smile": { "expr": 1, "sound": 2, "label": "Smile", "cat": "Smile" },
    "smirk": { "expr": 1, "sound": 2, "label": "Smirk", "cat": "Smirk" },
    "smoke": { "expr": 3, "sound": 0, "label": "Smoke", "cat": "Smoke" },
    "sneeze": { "expr": 4, "sound": 6, "label": "Sneeze", "cat": "Sneeze" },
    "sobbing": { "expr": 2, "sound": 4, "label": "Sobbing", "cat": "Sobbing" },
    "sparkle": { "expr": 1, "sound": 2, "label": "Sparkle", "cat": "Sparkle" },
    "speed": { "expr": 1, "sound": 1, "label": "Speed", "cat": "Speed" },
    "splash": { "expr": 1, "sound": 6, "label": "Splash", "cat": "Splash" },
    "spraying": { "expr": 4, "sound": 6, "label": "Spraying", "cat": "Spraying" },
    "squint": { "expr": 0, "sound": 0, "label": "Squint", "cat": "Squint" },
    "surprised": { "expr": 4, "sound": 6, "label": "Surprised", "cat": "Surprised" },
    "sushi": { "expr": 1, "sound": 2, "label": "Sushi", "cat": "Sushi" },
    "swinging": { "expr": 1, "sound": 2, "label": "Swinging", "cat": "Swinging" },
    "teasing": { "expr": 6, "sound": 6, "label": "Teasing", "cat": "Teasing" },
    "tough": { "expr": 3, "sound": 5, "label": "Tough", "cat": "Tough" },
    "weeping": { "expr": 2, "sound": 4, "label": "Weeping", "cat": "Weeping" },
    "wink": { "expr": 6, "sound": 2, "label": "Wink", "cat": "Wink" },
    "yawn": { "expr": 5, "sound": 0, "label": "Yawn", "cat": "Yawn" }
  };

  DatabaseService() {
    _initDatabase();
  }

  Future<void> _initDatabase() async {
    _prefs = await SharedPreferences.getInstance();
    _loadSettings();
    _loadGifs();
    _loadRobots();
    _loadEvents();
    _loadAlarms();
  }

  void _loadSettings() {
    if (_prefs == null) return;
    _is12HourFormat = _prefs!.getBool('is12HourFormat') ?? false;
    _notificationSyncEnabled = _prefs!.getBool('notificationSyncEnabled') ?? true;
    _gifSpeed = _prefs!.getDouble('gifSpeed') ?? 100.0;
    _gifDelay = _prefs!.getDouble('gifDelay') ?? 0.0;
    _gifIntroSpeed = _prefs!.getDouble('gifIntroSpeed') ?? 100.0;
    _introSoundSpeed = _prefs!.getDouble('introSoundSpeed') ?? 100.0;
    _negativeEnabled = _prefs!.getBool('negativeEnabled') ?? false;
    _defaultGif = _prefs!.getString('defaultGif') ?? 'default';
    _introGif = _prefs!.getString('introGif') ?? 'default';
    _touchSingle = _prefs!.getString('touchSingle') ?? 'default';
    _touchDouble = _prefs!.getString('touchDouble') ?? 'default';
    _touchLong = _prefs!.getString('touchLong') ?? 'default';
    _oledBrightness = _prefs!.getDouble('oledBrightness') ?? 1.0;
    _oledContrast = _prefs!.getDouble('oledContrast') ?? 1.0;
    _oledInvert = _prefs!.getBool('oledInvert') ?? false;
    _oledRotation = _prefs!.getDouble('oledRotation') ?? 0.0;
    _clockStyle = _prefs!.getInt('clockStyle') ?? 0;
    _notificationDuration = _prefs!.getDouble('notificationDuration') ?? 5.0;
    _reminderDuration = _prefs!.getDouble('reminderDuration') ?? 10.0;
    _birthdayDuration = _prefs!.getDouble('birthdayDuration') ?? 15.0;
    _bleSleepTime = _prefs!.getDouble('bleSleepTime') ?? 45.0;
    _bleName = _prefs!.getString('bleName') ?? 'Mr. Luna Robot';
    _allowedNotificationApps = _prefs!.getStringList('allowedNotificationApps') ?? [
      'whatsapp',
      'whatsapp_business',
      'instagram',
      'snapchat',
      'telegram',
      'messenger',
      'google_maps',
      'gmail',
      'youtube',
      'sms',
      'phone',
      'other_apps'
    ];
    notifyListeners();
  }

  void _loadGifs() {
    if (_prefs == null) return;
    final jsonStr = _prefs!.getString('gifs_database');
    if (jsonStr != null) {
      try {
        final List<dynamic> decoded = jsonDecode(jsonStr);
        _gifs = decoded.map((item) => GifModel.fromJson(item)).toList();
        
        // Merge missing GIFs from animMapping
        bool modified = false;
        animMapping.forEach((key, val) {
          final exists = _gifs.any((g) => g.id == key);
          if (!exists) {
            final int sizeBytes = 15000 + (key.hashCode % 40000);
            final String flashKb = '${(sizeBytes / 1024.0).toStringAsFixed(1)}KB';
            _gifs.add(GifModel(
              id: key,
              name: val['label'] as String,
              category: val['cat'] as String,
              favorite: key == 'happy' || key == 'relaxed',
              selected: true,
              hidden: false,
              size: sizeBytes,
              flashSize: flashKb,
              soundId: val['sound'] as int? ?? 0,
            ));
            modified = true;
          }
        });
        if (modified) {
          _saveGifsToDisk();
        }
      } catch (e) {
        print("Failed to decode GIFs database: $e");
        _seedDefaultGifs();
      }
    } else {
      _seedDefaultGifs();
    }
    notifyListeners();
  }

  void _seedDefaultGifs() {
    _gifs = [];
    animMapping.forEach((key, val) {
      // Random mock sizes to mirror web app behavior
      final int sizeBytes = 15000 + (key.hashCode % 40000);
      final String flashKb = '${(sizeBytes / 1024.0).toStringAsFixed(1)}KB';

      _gifs.add(GifModel(
        id: key,
        name: val['label'] as String,
        category: val['cat'] as String,
        favorite: key == 'happy' || key == 'relaxed',
        selected: true,
        hidden: false,
        size: sizeBytes,
        flashSize: flashKb,
        soundId: val['sound'] as int? ?? 0,
      ));
    });
    _saveGifsToDisk();
  }

  Future<void> _saveGifsToDisk() async {
    if (_prefs == null) return;
    final String encoded = jsonEncode(_gifs.map((g) => g.toJson()).toList());
    await _prefs!.setString('gifs_database', encoded);
  }

  // Setters for Settings with Persistence
  Future<void> updateGifSpeed(double speed) async {
    _gifSpeed = speed;
    await _prefs?.setDouble('gifSpeed', speed);
    notifyListeners();
  }

  Future<void> updateGifDelay(double delay) async {
    _gifDelay = delay;
    await _prefs?.setDouble('gifDelay', delay);
    notifyListeners();
  }

  Future<void> updateGifIntroSpeed(double speed) async {
    _gifIntroSpeed = speed;
    await _prefs?.setDouble('gifIntroSpeed', speed);
    notifyListeners();
  }

  Future<void> updateIntroSoundSpeed(double speed) async {
    _introSoundSpeed = speed;
    await _prefs?.setDouble('introSoundSpeed', speed);
    notifyListeners();
  }

  Future<void> updateNegativeEnabled(bool val) async {
    _negativeEnabled = val;
    await _prefs?.setBool('negativeEnabled', val);
    notifyListeners();
  }

  Future<void> updateDefaultGif(String val) async {
    _defaultGif = val;
    await _prefs?.setString('defaultGif', val);
    notifyListeners();
  }

  Future<void> updateIntroGif(String val) async {
    _introGif = val;
    await _prefs?.setString('introGif', val);
    notifyListeners();
  }

  Future<void> updateTouchSingle(String val) async {
    _touchSingle = val;
    await _prefs?.setString('touchSingle', val);
    notifyListeners();
  }

  Future<void> updateTouchDouble(String val) async {
    _touchDouble = val;
    await _prefs?.setString('touchDouble', val);
    notifyListeners();
  }

  Future<void> updateTouchLong(String val) async {
    _touchLong = val;
    await _prefs?.setString('touchLong', val);
    notifyListeners();
  }

  Future<void> updateClockStyle(int val) async {
    _clockStyle = val;
    await _prefs?.setInt('clockStyle', val);
    notifyListeners();
  }

  Future<void> updateOledBrightness(double val) async {
    _oledBrightness = val;
    await _prefs?.setDouble('oledBrightness', val);
    notifyListeners();
  }

  Future<void> updateOledContrast(double val) async {
    _oledContrast = val;
    await _prefs?.setDouble('oledContrast', val);
    notifyListeners();
  }

  Future<void> updateOledInvert(bool val) async {
    _oledInvert = val;
    await _prefs?.setBool('oledInvert', val);
    notifyListeners();
  }

  Future<void> updateOledRotation(double val) async {
    _oledRotation = val;
    await _prefs?.setDouble('oledRotation', val);
    notifyListeners();
  }

  Future<void> updateNotificationDuration(double val) async {
    _notificationDuration = val;
    await _prefs?.setDouble('notificationDuration', val);
    notifyListeners();
  }

  Future<void> updateReminderDuration(double val) async {
    _reminderDuration = val;
    await _prefs?.setDouble('reminderDuration', val);
    notifyListeners();
  }

  Future<void> updateBirthdayDuration(double val) async {
    _birthdayDuration = val;
    await _prefs?.setDouble('birthdayDuration', val);
    notifyListeners();
  }

  Future<void> updateBleSleepTime(double val) async {
    _bleSleepTime = val;
    await _prefs?.setDouble('bleSleepTime', val);
    notifyListeners();
  }

  Future<void> updateBleName(String val) async {
    _bleName = val;
    await _prefs?.setString('bleName', val);
    notifyListeners();
  }

  // GIF Metadata operations
  void toggleFavorite(String id) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      _gifs[idx].favorite = !_gifs[idx].favorite;
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void toggleHidden(String id) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      _gifs[idx].hidden = !_gifs[idx].hidden;
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void toggleSelected(String id) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      _gifs[idx].selected = !_gifs[idx].selected;
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void addCustomGif(GifModel newGif) {
    final idx = _gifs.indexWhere((g) => g.id == newGif.id);
    if (idx != -1) {
      _gifs[idx] = newGif;
    } else {
      _gifs.add(newGif);
    }
    _saveGifsToDisk();
    notifyListeners();
  }

  void updateGifSound(String id, int soundId) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      _gifs[idx] = _gifs[idx].copyWith(soundId: soundId);
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void deleteCustomGif(String id) {
    // Original assets cannot be deleted
    if (animMapping.containsKey(id)) return;
    _gifs.removeWhere((g) => g.id == id);
    _saveGifsToDisk();
    notifyListeners();
  }

  void renameGif(String id, String newName) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      final old = _gifs[idx];
      _gifs[idx] = old.copyWith(name: newName);
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void duplicateGif(String id) {
    final idx = _gifs.indexWhere((g) => g.id == id);
    if (idx != -1) {
      final old = _gifs[idx];
      final duplicate = old.copyWith(
        id: '${old.id}_copy',
        name: '${old.name} (Copy)',
      );
      _gifs.add(duplicate);
      _saveGifsToDisk();
      notifyListeners();
    }
  }

  void selectAll(bool select) {
    for (int i = 0; i < _gifs.length; i++) {
      _gifs[i].selected = select;
    }
    _saveGifsToDisk();
    notifyListeners();
  }

  Future<void> factoryReset() async {
    _gifSpeed = 100.0;
    _gifDelay = 0.0;
    _gifIntroSpeed = 100.0;
    _introSoundSpeed = 100.0;
    _negativeEnabled = false;
    _defaultGif = 'default';
    _introGif = 'default';
    _touchSingle = 'default';
    _touchDouble = 'default';
    _touchLong = 'default';
    _oledBrightness = 1.0;
    _oledContrast = 1.0;
    _oledInvert = false;
    _oledRotation = 0.0;
    _clockStyle = 0;
    _notificationDuration = 5.0;
    _reminderDuration = 10.0;
    _birthdayDuration = 15.0;
    _bleSleepTime = 45.0;
    _bleName = 'Mr. Luna Robot';

    await _prefs?.clear();
    _seedDefaultGifs();
    _loadSettings();
    _robots = [];
    await _saveRobotsToDisk();
  }

  void _loadRobots() {
    if (_prefs == null) return;
    final jsonStr = _prefs!.getString('robots_database');
    if (jsonStr != null) {
      try {
        final List<dynamic> decoded = jsonDecode(jsonStr);
        _robots = decoded.map((item) => RobotProfile.fromJson(item)).toList();
      } catch (e) {
        print("Failed to decode robots database: $e");
        _seedDefaultRobots();
      }
    } else {
      _seedDefaultRobots();
    }
    notifyListeners();
  }

  void _seedDefaultRobots() {
    _robots = [];
    _saveRobotsToDisk();
  }

  Future<void> _saveRobotsToDisk() async {
    if (_prefs == null) return;
    final String encoded = jsonEncode(_robots.map((r) => r.toJson()).toList());
    await _prefs!.setString('robots_database', encoded);
  }

  Future<void> addRobot(RobotProfile robot) async {
    final idx = _robots.indexWhere((r) => r.id == robot.id);
    // If it's the first robot, make it primary
    bool makePrimary = _robots.isEmpty || robot.isPrimary;
    
    final preparedRobot = robot.copyWith(
      isPrimary: makePrimary,
    );

    if (idx != -1) {
      _robots[idx] = preparedRobot;
    } else {
      _robots.add(preparedRobot);
    }

    if (makePrimary) {
      for (int i = 0; i < _robots.length; i++) {
        if (_robots[i].id != preparedRobot.id) {
          _robots[i] = _robots[i].copyWith(isPrimary: false);
        }
      }
    }

    await _saveRobotsToDisk();
    notifyListeners();
  }

  Future<void> renameRobot(String id, String newName) async {
    final idx = _robots.indexWhere((r) => r.id == id);
    if (idx != -1) {
      _robots[idx] = _robots[idx].copyWith(name: newName);
      await _saveRobotsToDisk();
      notifyListeners();
    }
  }

  Future<void> removeRobot(String id) async {
    final idx = _robots.indexWhere((r) => r.id == id);
    if (idx != -1) {
      final wasPrimary = _robots[idx].isPrimary;
      _robots.removeAt(idx);
      
      if (wasPrimary && _robots.isNotEmpty) {
        _robots[0] = _robots[0].copyWith(isPrimary: true);
      }

      // Also clean up relationships referencing this robot
      for (int i = 0; i < _robots.length; i++) {
        if (_robots[i].companionDeviceId == id) {
          _robots[i] = _robots[i].copyWith(
            companionDeviceId: null,
            relationshipType: 'none',
          );
        }
      }

      await _saveRobotsToDisk();
      notifyListeners();
    }
  }

  Future<void> setPrimaryRobot(String id) async {
    for (int i = 0; i < _robots.length; i++) {
      if (_robots[i].id == id) {
        _robots[i] = _robots[i].copyWith(isPrimary: true);
      } else {
        _robots[i] = _robots[i].copyWith(isPrimary: false);
      }
    }
    await _saveRobotsToDisk();
    notifyListeners();
  }

  Future<void> updateRobotWifi(String id, String ssid) async {
    final idx = _robots.indexWhere((r) => r.id == id);
    if (idx != -1) {
      _robots[idx] = _robots[idx].copyWith(wifiSSID: ssid);
      await _saveRobotsToDisk();
      notifyListeners();
    }
  }

  Future<void> updateRobotCloudStatus(String id, String status) async {
    final idx = _robots.indexWhere((r) => r.id == id);
    if (idx != -1) {
      _robots[idx] = _robots[idx].copyWith(cloudStatus: status);
      await _saveRobotsToDisk();
      notifyListeners();
    }
  }

  Future<void> updateRobotRelationship(String id, String? companionId, String type) async {
    final idx = _robots.indexWhere((r) => r.id == id);
    if (idx != -1) {
      _robots[idx] = _robots[idx].copyWith(
        companionDeviceId: companionId,
        relationshipType: type,
      );

      // Bidirectional update if the companion exists
      if (companionId != null) {
        final compIdx = _robots.indexWhere((r) => r.id == companionId);
        if (compIdx != -1) {
          _robots[compIdx] = _robots[compIdx].copyWith(
            companionDeviceId: id,
            relationshipType: type,
          );
        }
      }
      
      await _saveRobotsToDisk();
      notifyListeners();
    }
  }

  void _loadEvents() {
    if (_prefs == null) return;
    final jsonStr = _prefs!.getString('calendar_events');
    if (jsonStr != null) {
      try {
        final List<dynamic> decoded = jsonDecode(jsonStr);
        _events = decoded.map((item) => CalendarEvent.fromJson(item)).toList();
      } catch (e) {
        print("Failed to decode calendar events: $e");
      }
    }
    notifyListeners();
  }

  Future<void> _saveEventsToDisk() async {
    if (_prefs == null) return;
    final String encoded = jsonEncode(_events.map((e) => e.toJson()).toList());
    await _prefs!.setString('calendar_events', encoded);
  }

  Future<void> addEvent(CalendarEvent event) async {
    _events.add(event);
    await _saveEventsToDisk();
    notifyListeners();
  }

  Future<void> deleteEvent(String id) async {
    _events.removeWhere((e) => e.id == id);
    await _saveEventsToDisk();
    notifyListeners();
  }

  void _loadAlarms() {
    if (_prefs == null) return;
    final jsonStr = _prefs!.getString('alarms_database');
    if (jsonStr != null) {
      try {
        final List<dynamic> decoded = jsonDecode(jsonStr);
        _alarms = decoded.map((item) => AlarmModel.fromJson(item)).toList();
      } catch (e) {
        print("Failed to decode alarms database: $e");
      }
    }
    notifyListeners();
  }

  Future<void> _saveAlarmsToDisk() async {
    if (_prefs == null) return;
    final String encoded = jsonEncode(_alarms.map((a) => a.toJson()).toList());
    await _prefs!.setString('alarms_database', encoded);
  }

  Future<void> addAlarm(AlarmModel alarm) async {
    _alarms.add(alarm);
    await _saveAlarmsToDisk();
    notifyListeners();
  }

  Future<void> toggleAlarm(String id) async {
    final idx = _alarms.indexWhere((a) => a.id == id);
    if (idx != -1) {
      _alarms[idx] = _alarms[idx].copyWith(isEnabled: !_alarms[idx].isEnabled);
      await _saveAlarmsToDisk();
      notifyListeners();
    }
  }

  Future<void> deleteAlarm(String id) async {
    _alarms.removeWhere((a) => a.id == id);
    await _saveAlarmsToDisk();
    notifyListeners();
  }

  Future<void> updateIs12HourFormat(bool val) async {
    _is12HourFormat = val;
    await _prefs?.setBool('is12HourFormat', val);
    notifyListeners();
  }

  Future<void> updateNotificationSyncEnabled(bool val) async {
    _notificationSyncEnabled = val;
    await _prefs?.setBool('notificationSyncEnabled', val);
    notifyListeners();
  }

  Future<void> updateAllowedNotificationApps(List<String> apps) async {
    _allowedNotificationApps = apps;
    await _prefs?.setStringList('allowedNotificationApps', apps);
    notifyListeners();
  }
}
