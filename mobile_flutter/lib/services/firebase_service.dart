import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:http/http.dart' as http;
import 'package:firebase_auth/firebase_auth.dart';
import 'package:google_sign_in/google_sign_in.dart';

class FirebaseService with ChangeNotifier {
  bool _useLiveConfig = false;
  Map<String, dynamic>? _currentUser;
  
  // Realtime Database URL
  final String _firebaseUrl = "https://mrluna-66b68-default-rtdb.firebaseio.com";
  Timer? _livePollTimer;

  // Lists
  List<Map<String, dynamic>> _registeredUsers = [];
  List<Map<String, dynamic>> _friends = [];
  List<Map<String, dynamic>> _friendRequests = [];
  
  // Remote pairing state
  String? _pairedFriendUid;
  String? _pairedFriendRobotId;
  String? _pairedFriendRobotName;
  String? _pairedFriendRobotVariant;
  
  // Active remote logs / history
  final List<String> _cloudLogs = [];

  // Listeners or stream for incoming remote triggers
  final StreamController<Map<String, dynamic>> _remoteTriggerController = StreamController<Map<String, dynamic>>.broadcast();
  Stream<Map<String, dynamic>> get remoteTriggers => _remoteTriggerController.stream;

  // Getters
  bool get useLiveConfig => _useLiveConfig;
  Map<String, dynamic>? get currentUser => _currentUser;
  bool get isSignedIn => _currentUser != null;
  List<Map<String, dynamic>> get friends => _friends;
  List<Map<String, dynamic>> get friendRequests => _friendRequests;
  List<Map<String, dynamic>> get registeredUsers => _registeredUsers;
  String? get pairedFriendUid => _pairedFriendUid;
  String? get pairedFriendRobotName => _pairedFriendRobotName;
  String? get pairedFriendRobotVariant => _pairedFriendRobotVariant;
  List<String> get cloudLogs => _cloudLogs;

  FirebaseService() {
    _initService();
  }

  Future<void> _initService() async {
    final prefs = await SharedPreferences.getInstance();
    _useLiveConfig = prefs.getBool("firebase_use_live_config") ?? false;
    
    // Load currentUser if saved
    final userJson = prefs.getString("firebase_current_user");
    if (userJson != null) {
      try {
        _currentUser = jsonDecode(userJson);
      } catch (_) {}
    }
    
    // Load remote pairing info
    _pairedFriendUid = prefs.getString("firebase_paired_friend_uid");
    _pairedFriendRobotId = prefs.getString("firebase_paired_friend_robot_id");
    _pairedFriendRobotName = prefs.getString("firebase_paired_friend_robot_name");
    _pairedFriendRobotVariant = prefs.getString("firebase_paired_friend_robot_variant");

    // Initialize mock database users
    _setupMockDatabase();
    
    // Load friends/requests from local storage
    _loadFriendsData();

    _startLivePolling();
  }

  void _setupMockDatabase() {
    _registeredUsers = [
      {
        'uid': 'alice_123',
        'email': 'alice.luna@gmail.com',
        'displayName': 'Alice',
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Alice',
        'robotId': 'MS_LUNA_ALICE',
        'robotName': 'Lumina',
        'robotVariant': 'ms_luna',
        'isOnline': true,
      },
      {
        'uid': 'bob_456',
        'email': 'bob.luna@gmail.com',
        'displayName': 'Bob',
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Bob',
        'robotId': 'MR_LUNA_BOB',
        'robotName': 'RoboBob',
        'robotVariant': 'mr_luna',
        'isOnline': false,
      },
      {
        'uid': 'sasi_789',
        'email': 'sasi.dev@gmail.com',
        'displayName': 'Sasi Dev',
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Sasi',
        'robotId': 'MR_LUNA_SASI',
        'robotName': 'LunaMax',
        'robotVariant': 'mr_luna',
        'isOnline': true,
      },
      {
        'uid': 'luna_fan_99',
        'email': 'luna.fanatic@gmail.com',
        'displayName': 'Luna Fanatic',
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Fanatic',
        'robotId': 'MS_LUNA_FAN',
        'robotName': 'Rosy',
        'robotVariant': 'ms_luna',
        'isOnline': true,
      },
      {
        'uid': 'google_eng_0',
        'email': 'google.engineer@gmail.com',
        'displayName': 'Google Eng',
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Google',
        'robotId': 'MR_LUNA_GOOG',
        'robotName': 'Tensor',
        'robotVariant': 'mr_luna',
        'isOnline': true,
      }
    ];
  }

  Future<void> _loadFriendsData() async {
    final prefs = await SharedPreferences.getInstance();
    
    // Load friends
    final friendsJson = prefs.getString("firebase_friends_list");
    if (friendsJson != null) {
      try {
        final List<dynamic> list = jsonDecode(friendsJson);
        _friends = list.map((item) => Map<String, dynamic>.from(item)).toList();
      } catch (_) {}
    } else {
      _friends = [
        {
          'uid': 'sasi_789',
          'email': 'sasi.dev@gmail.com',
          'displayName': 'Sasi Dev',
          'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Sasi',
          'robotId': 'MR_LUNA_SASI',
          'robotName': 'LunaMax',
          'robotVariant': 'mr_luna',
          'isOnline': true,
        }
      ];
    }

    // Load friend requests
    final requestsJson = prefs.getString("firebase_requests_list");
    if (requestsJson != null) {
      try {
        final List<dynamic> list = jsonDecode(requestsJson);
        _friendRequests = list.map((item) => Map<String, dynamic>.from(item)).toList();
      } catch (_) {}
    } else {
      _friendRequests = [
        {
          'id': 'req_alice',
          'fromUid': 'alice_123',
          'email': 'alice.luna@gmail.com',
          'displayName': 'Alice',
          'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=Alice',
          'robotId': 'MS_LUNA_ALICE',
          'robotName': 'Lumina',
          'robotVariant': 'ms_luna',
          'timestamp': DateTime.now().subtract(const Duration(minutes: 5)).toIso8601String(),
        }
      ];
    }
    notifyListeners();
  }

  Future<void> _saveFriendsData() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString("firebase_friends_list", jsonEncode(_friends));
    await prefs.setString("firebase_requests_list", jsonEncode(_friendRequests));
  }

  Future<void> toggleLiveConfig(bool value) async {
    _useLiveConfig = value;
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("firebase_use_live_config", value);
    addCloudLog("Toggled mode: ${value ? 'LIVE FIREBASE' : 'LOCAL SANDBOX'}");
    notifyListeners();
  }

  void _startLivePolling() {
    _livePollTimer?.cancel();
    _livePollTimer = Timer.periodic(const Duration(seconds: 3), (timer) {
      if (_useLiveConfig && isSignedIn) {
        _pollLiveDatabase();
      }
    });
  }

  Future<void> _pollLiveDatabase() async {
    if (_currentUser == null) return;
    final uid = _currentUser!['uid'];

    try {
      // 1. Poll incoming triggers
      final triggerUrl = Uri.parse("$_firebaseUrl/triggers/$uid.json");
      final triggerRes = await http.get(triggerUrl);
      if (triggerRes.statusCode == 200 && triggerRes.body != "null") {
        final data = jsonDecode(triggerRes.body);
        if (data is Map<String, dynamic>) {
          _remoteTriggerController.add(data);
          addCloudLog("Cloud Receive (Live): ${data['senderName']} sent remote ${data['eventType']}");
          await http.delete(triggerUrl);
        }
      }

      // 2. Poll incoming friend requests
      final reqUrl = Uri.parse("$_firebaseUrl/friend_requests/$uid.json");
      final reqRes = await http.get(reqUrl);
      if (reqRes.statusCode == 200) {
        if (reqRes.body == "null") {
          if (_friendRequests.isNotEmpty) {
            _friendRequests = [];
            notifyListeners();
          }
        } else {
          final data = jsonDecode(reqRes.body) as Map<String, dynamic>;
          final List<Map<String, dynamic>> newRequests = [];
          data.forEach((fromUid, val) {
            if (val is Map<String, dynamic>) {
              newRequests.add(Map<String, dynamic>.from(val));
            }
          });
          _friendRequests = newRequests;
          notifyListeners();
        }
      }

      // 3. Poll friends list
      final friendsUrl = Uri.parse("$_firebaseUrl/friends/$uid.json");
      final friendsRes = await http.get(friendsUrl);
      if (friendsRes.statusCode == 200) {
        if (friendsRes.body == "null") {
          if (_friends.isNotEmpty) {
            _friends = [];
            notifyListeners();
          }
        } else {
          final data = jsonDecode(friendsRes.body) as Map<String, dynamic>;
          final List<Map<String, dynamic>> newFriends = [];
          data.forEach((friendUid, val) {
            if (val is Map<String, dynamic>) {
              newFriends.add(Map<String, dynamic>.from(val));
            }
          });
          _friends = newFriends;
          notifyListeners();
        }
      }

      // 4. Sync online users profiles
      final usersUrl = Uri.parse("$_firebaseUrl/users.json");
      final usersRes = await http.get(usersUrl);
      if (usersRes.statusCode == 200 && usersRes.body != "null") {
        final data = jsonDecode(usersRes.body) as Map<String, dynamic>;
        final List<Map<String, dynamic>> newUsers = [];
        data.forEach((userUid, val) {
          if (val is Map<String, dynamic>) {
            newUsers.add(Map<String, dynamic>.from(val));
          }
        });
        _registeredUsers = newUsers;
        notifyListeners();
      }
    } catch (_) {}
  }

  // Real Google Sign In & Firebase Auth (with fallback to mock simulation if needed)
  Future<void> signInWithGoogle(String email, String displayName, String avatarSeed, String robotName, String robotVariant) async {
    try {
      addCloudLog("Starting Google Sign-In sequence...");
      
      final GoogleSignIn googleSignIn = GoogleSignIn();
      final GoogleSignInAccount? googleUser = await googleSignIn.signIn();
      
      if (googleUser == null) {
        addCloudLog("Google Sign-In canceled by user.");
        return;
      }
      
      final GoogleSignInAuthentication googleAuth = await googleUser.authentication;
      final AuthCredential credential = GoogleAuthProvider.credential(
        accessToken: googleAuth.accessToken,
        idToken: googleAuth.idToken,
      );
      
      final UserCredential userCredential = await FirebaseAuth.instance.signInWithCredential(credential);
      final User? firebaseUser = userCredential.user;
      
      if (firebaseUser == null) {
        addCloudLog("Firebase Auth credential sign-in failed.");
        return;
      }
      
      final uid = firebaseUser.uid;
      final userEmail = firebaseUser.email ?? email;
      final userName = firebaseUser.displayName ?? displayName;
      final userPhoto = firebaseUser.photoURL ?? 'https://api.dicebear.com/7.x/adventurer/png?seed=$avatarSeed';

      _currentUser = {
        'uid': uid,
        'email': userEmail,
        'displayName': userName,
        'photoUrl': userPhoto,
        'robotId': 'ROBOT_$uid',
        'robotName': robotName,
        'robotVariant': robotVariant,
        'isOnline': true,
      };

      final prefs = await SharedPreferences.getInstance();
      await prefs.setString("firebase_current_user", jsonEncode(_currentUser));
      
      addCloudLog("User '$userName' logged in successfully via Firebase Auth.");

      if (_useLiveConfig) {
        try {
          final url = Uri.parse("$_firebaseUrl/users/$uid.json");
          await http.put(url, body: jsonEncode(_currentUser));
          addCloudLog("Registered user profile to Live Database.");
        } catch (e) {
          addCloudLog("Live register error: $e");
        }
      }

      _startLivePolling();
      notifyListeners();
    } catch (e) {
      addCloudLog("Google/Firebase Auth Error: $e");
      debugPrint("Google/Firebase Auth Error: $e");
      
      // Fallback to simulation if native bindings or config are missing
      addCloudLog("Falling back to local simulated credentials.");
      final uid = "user_${email.replaceAll(RegExp(r'[^a-zA-Z0-9]'), '')}";
      _currentUser = {
        'uid': uid,
        'email': email,
        'displayName': displayName,
        'photoUrl': 'https://api.dicebear.com/7.x/adventurer/png?seed=$avatarSeed',
        'robotId': 'ROBOT_$uid',
        'robotName': robotName,
        'robotVariant': robotVariant,
        'isOnline': true,
      };
      
      final prefs = await SharedPreferences.getInstance();
      await prefs.setString("firebase_current_user", jsonEncode(_currentUser));
      
      if (_useLiveConfig) {
        try {
          final url = Uri.parse("$_firebaseUrl/users/$uid.json");
          await http.put(url, body: jsonEncode(_currentUser));
          addCloudLog("Registered fallback profile to Live Database.");
        } catch (err) {
          addCloudLog("Live register error: $err");
        }
      }
      
      _startLivePolling();
      notifyListeners();
    }
  }

  Future<void> signOut() async {
    try {
      await FirebaseAuth.instance.signOut();
      await GoogleSignIn().signOut();
    } catch (_) {}

    if (_useLiveConfig && _currentUser != null) {
      try {
        final uid = _currentUser!['uid'];
        final url = Uri.parse("$_firebaseUrl/users/$uid.json");
        final offlineUser = Map<String, dynamic>.from(_currentUser!)..['isOnline'] = false;
        await http.put(url, body: jsonEncode(offlineUser));
      } catch (_) {}
    }

    _currentUser = null;
    _pairedFriendUid = null;
    _pairedFriendRobotId = null;
    _pairedFriendRobotName = null;
    _pairedFriendRobotVariant = null;

    final prefs = await SharedPreferences.getInstance();
    await prefs.remove("firebase_current_user");
    await prefs.remove("firebase_paired_friend_uid");
    await prefs.remove("firebase_paired_friend_robot_id");
    await prefs.remove("firebase_paired_friend_robot_name");
    await prefs.remove("firebase_paired_friend_robot_variant");

    _livePollTimer?.cancel();
    addCloudLog("User logged out.");
    notifyListeners();
  }

  // Search User Profile
  List<Map<String, dynamic>> searchProfiles(String query) {
    if (query.trim().isEmpty) return [];
    final lowercaseQuery = query.toLowerCase();
    return _registeredUsers.where((user) {
      final matchesEmail = user['email'].toString().toLowerCase().contains(lowercaseQuery);
      final matchesName = user['displayName'].toString().toLowerCase().contains(lowercaseQuery);
      final isSelf = _currentUser != null && _currentUser!['uid'] == user['uid'];
      return (matchesEmail || matchesName) && !isSelf;
    }).toList();
  }

  // Send Friend Request
  Future<bool> sendFriendRequest(Map<String, dynamic> targetUser) async {
    if (_friends.any((f) => f['uid'] == targetUser['uid'])) {
      return false;
    }
    if (_friendRequests.any((r) => r['fromUid'] == targetUser['uid'])) {
      return false;
    }

    addCloudLog("Sent friend request to ${targetUser['displayName']}");

    if (_useLiveConfig) {
      try {
        final url = Uri.parse("$_firebaseUrl/friend_requests/${targetUser['uid']}/${_currentUser!['uid']}.json");
        await http.put(url, body: jsonEncode({
          'id': 'req_${_currentUser!['uid']}',
          'fromUid': _currentUser!['uid'],
          'email': _currentUser!['email'],
          'displayName': _currentUser!['displayName'],
          'photoUrl': _currentUser!['photoUrl'],
          'robotId': _currentUser!['robotId'],
          'robotName': _currentUser!['robotName'],
          'robotVariant': _currentUser!['robotVariant'],
          'timestamp': DateTime.now().toIso8601String(),
        }));
      } catch (e) {
        addCloudLog("Live request error: $e");
      }
    } else {
      if (targetUser['isOnline'] == true) {
        Timer(const Duration(seconds: 3), () {
          if (_currentUser == null) return;
          if (!_friends.any((f) => f['uid'] == targetUser['uid'])) {
            _friends.add(targetUser);
            _saveFriendsData();
            addCloudLog("Friend Request ACCEPTED by ${targetUser['displayName']}!");
            notifyListeners();
          }
        });
      }
    }

    notifyListeners();
    return true;
  }

  // Accept Friend Request
  Future<void> acceptFriendRequest(String requestId) async {
    final reqIdx = _friendRequests.indexWhere((r) => r['id'] == requestId);
    if (reqIdx != -1) {
      final req = _friendRequests[reqIdx];
      _friendRequests.removeAt(reqIdx);
      
      final friendData = {
        'uid': req['fromUid'],
        'email': req['email'],
        'displayName': req['displayName'],
        'photoUrl': req['photoUrl'],
        'robotId': req['robotId'],
        'robotName': req['robotName'],
        'robotVariant': req['robotVariant'],
        'isOnline': true,
      };

      if (_useLiveConfig) {
        try {
          final deleteUrl = Uri.parse("$_firebaseUrl/friend_requests/${_currentUser!['uid']}/${req['fromUid']}.json");
          await http.delete(deleteUrl);

          final myFriendsUrl = Uri.parse("$_firebaseUrl/friends/${_currentUser!['uid']}/${req['fromUid']}.json");
          await http.put(myFriendsUrl, body: jsonEncode(friendData));

          final targetFriendsUrl = Uri.parse("$_firebaseUrl/friends/${req['fromUid']}/${_currentUser!['uid']}.json");
          await http.put(targetFriendsUrl, body: jsonEncode({
            'uid': _currentUser!['uid'],
            'email': _currentUser!['email'],
            'displayName': _currentUser!['displayName'],
            'photoUrl': _currentUser!['photoUrl'],
            'robotId': _currentUser!['robotId'],
            'robotName': _currentUser!['robotName'],
            'robotVariant': _currentUser!['robotVariant'],
            'isOnline': true,
          }));
        } catch (e) {
          addCloudLog("Live accept error: $e");
        }
      } else {
        if (!_friends.any((f) => f['uid'] == req['fromUid'])) {
          _friends.add(friendData);
        }
        await _saveFriendsData();
      }

      addCloudLog("Accepted friend request from ${req['displayName']}");
      notifyListeners();
    }
  }

  // Reject Friend Request
  Future<void> rejectFriendRequest(String requestId) async {
    final reqIdx = _friendRequests.indexWhere((r) => r['id'] == requestId);
    if (reqIdx != -1) {
      final req = _friendRequests[reqIdx];
      _friendRequests.removeAt(reqIdx);

      if (_useLiveConfig) {
        try {
          final deleteUrl = Uri.parse("$_firebaseUrl/friend_requests/${_currentUser!['uid']}/${req['fromUid']}.json");
          await http.delete(deleteUrl);
        } catch (e) {
          addCloudLog("Live reject error: $e");
        }
      } else {
        await _saveFriendsData();
      }

      addCloudLog("Rejected friend request from ${req['displayName']}");
      notifyListeners();
    }
  }

  // Unfriend
  Future<void> unfriend(String uid) async {
    _friends.removeWhere((f) => f['uid'] == uid);
    if (_pairedFriendUid == uid) {
      await unpairRobot();
    }

    if (_useLiveConfig) {
      try {
        final deleteUrl1 = Uri.parse("$_firebaseUrl/friends/${_currentUser!['uid']}/$uid.json");
        await http.delete(deleteUrl1);
        final deleteUrl2 = Uri.parse("$_firebaseUrl/friends/$uid/${_currentUser!['uid']}.json");
        await http.delete(deleteUrl2);
      } catch (e) {
        addCloudLog("Live unfriend error: $e");
      }
    } else {
      await _saveFriendsData();
    }

    addCloudLog("Removed friend with UID: $uid");
    notifyListeners();
  }

  // Pair Robot with Friend's Robot (Cloud Bonding Mode)
  Future<void> pairRobotWithFriend(Map<String, dynamic> friend) async {
    _pairedFriendUid = friend['uid'];
    _pairedFriendRobotId = friend['robotId'];
    _pairedFriendRobotName = friend['robotName'] ?? "Companion";
    _pairedFriendRobotVariant = friend['robotVariant'] ?? "mr_luna";

    final prefs = await SharedPreferences.getInstance();
    await prefs.setString("firebase_paired_friend_uid", _pairedFriendUid!);
    await prefs.setString("firebase_paired_friend_robot_id", _pairedFriendRobotId!);
    await prefs.setString("firebase_paired_friend_robot_name", _pairedFriendRobotName!);
    await prefs.setString("firebase_paired_friend_robot_variant", _pairedFriendRobotVariant!);

    addCloudLog("Paired primary robot with friend's robot '${_pairedFriendRobotName}' (${_pairedFriendRobotVariant.toString().toUpperCase()}) via cloud!");
    notifyListeners();
  }

  Future<void> unpairRobot() async {
    _pairedFriendUid = null;
    _pairedFriendRobotId = null;
    _pairedFriendRobotName = null;
    _pairedFriendRobotVariant = null;

    final prefs = await SharedPreferences.getInstance();
    await prefs.remove("firebase_paired_friend_uid");
    await prefs.remove("firebase_paired_friend_robot_id");
    await prefs.remove("firebase_paired_friend_robot_name");
    await prefs.remove("firebase_paired_friend_robot_variant");

    addCloudLog("Unpaired cloud robot link.");
    notifyListeners();
  }

  // Send Cloud Trigger (e.g. Tap Sequence Expression / SFX)
  Future<void> sendCloudTrigger(String eventType, int expr, int sound) async {
    if (_pairedFriendUid == null) return;
    
    final senderName = _currentUser != null ? _currentUser!['displayName'] : "Luna Owner";
    addCloudLog("Cloud Send: $eventType trigger -> '${_pairedFriendRobotName}' (Expr: $expr, Sound: $sound)");
    
    final expressions = ["HAPPY", "SAD", "ANGRY", "SURPRISED", "SLEEPING", "WINK"];
    final reactionExprLabel = (expr >= 1 && expr <= expressions.length) ? expressions[expr - 1] : "HAPPY";

    if (_useLiveConfig) {
      try {
        final url = Uri.parse("$_firebaseUrl/triggers/${_pairedFriendUid}.json");
        await http.put(url, body: jsonEncode({
          'senderName': senderName,
          'eventType': eventType,
          'exprLabel': reactionExprLabel,
          'soundId': sound,
          'timestamp': DateTime.now().millisecondsSinceEpoch,
        }));
      } catch (e) {
        addCloudLog("Live send trigger error: $e");
      }
    } else {
      Timer(const Duration(milliseconds: 2500), () {
        if (_pairedFriendUid == null) return;
        
        final expressions = ["HAPPY", "WINK", "SURPRISED", "ADORE", "SMILE"];
        final sounds = [2, 6, 3, 1, 7];
        final randomIndex = DateTime.now().millisecondsSinceEpoch % expressions.length;
        final reactionExprLabel = expressions[randomIndex];
        final reactionSoundId = sounds[randomIndex];
        
        final incomingData = {
          'senderName': _pairedFriendRobotName,
          'eventType': 'REMOTE_TAP',
          'exprLabel': reactionExprLabel,
          'soundId': reactionSoundId,
        };

        _remoteTriggerController.add(incomingData);
        addCloudLog("Cloud Receive: '${_pairedFriendRobotName}' reacted with expression $reactionExprLabel and Sound $reactionSoundId!");
      });
    }
  }

  void addCloudLog(String message) {
    final timestamp = DateTime.now().toLocal().toString().split(' ')[1].substring(0, 8);
    _cloudLogs.add('[$timestamp] $message');
    if (_cloudLogs.length > 50) {
      _cloudLogs.removeAt(0);
    }
    notifyListeners();
  }

  void clearCloudLogs() {
    _cloudLogs.clear();
    notifyListeners();
  }

  @override
  void dispose() {
    _livePollTimer?.cancel();
    _remoteTriggerController.close();
    super.dispose();
  }
}
