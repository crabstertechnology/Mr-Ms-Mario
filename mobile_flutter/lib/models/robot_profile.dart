class RobotProfile {
  final String id;
  final String name;
  final String variant; // 'mr_luna' or 'ms_luna'
  final String remoteId; // BLE Remote ID/MAC Address
  final String wifiSSID;
  final String cloudStatus; // 'online' or 'offline'
  final String? companionDeviceId;
  final String relationshipType; // 'friends', 'couple', or 'none'
  final bool isPrimary;
  final DateTime lastConnected;

  RobotProfile({
    required this.id,
    required this.name,
    required this.variant,
    required this.remoteId,
    this.wifiSSID = '',
    this.cloudStatus = 'offline',
    this.companionDeviceId,
    this.relationshipType = 'none',
    this.isPrimary = false,
    required this.lastConnected,
  });

  RobotProfile copyWith({
    String? id,
    String? name,
    String? variant,
    String? remoteId,
    String? wifiSSID,
    String? cloudStatus,
    String? companionDeviceId,
    String? relationshipType,
    bool? isPrimary,
    DateTime? lastConnected,
  }) {
    return RobotProfile(
      id: id ?? this.id,
      name: name ?? this.name,
      variant: variant ?? this.variant,
      remoteId: remoteId ?? this.remoteId,
      wifiSSID: wifiSSID ?? this.wifiSSID,
      cloudStatus: cloudStatus ?? this.cloudStatus,
      companionDeviceId: companionDeviceId ?? this.companionDeviceId,
      relationshipType: relationshipType ?? this.relationshipType,
      isPrimary: isPrimary ?? this.isPrimary,
      lastConnected: lastConnected ?? this.lastConnected,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'variant': variant,
      'remoteId': remoteId,
      'wifiSSID': wifiSSID,
      'cloudStatus': cloudStatus,
      'companionDeviceId': companionDeviceId,
      'relationshipType': relationshipType,
      'isPrimary': isPrimary,
      'lastConnected': lastConnected.toIso8601String(),
    };
  }

  factory RobotProfile.fromJson(Map<String, dynamic> json) {
    return RobotProfile(
      id: json['id'] as String,
      name: json['name'] as String,
      variant: json['variant'] as String? ?? 'mr_luna',
      remoteId: json['remoteId'] as String? ?? '',
      wifiSSID: json['wifiSSID'] as String? ?? '',
      cloudStatus: json['cloudStatus'] as String? ?? 'offline',
      companionDeviceId: json['companionDeviceId'] as String?,
      relationshipType: json['relationshipType'] as String? ?? 'none',
      isPrimary: json['isPrimary'] as bool? ?? false,
      lastConnected: json['lastConnected'] != null
          ? DateTime.parse(json['lastConnected'] as String)
          : DateTime.now(),
    );
  }
}
