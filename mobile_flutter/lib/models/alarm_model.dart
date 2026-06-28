class AlarmModel {
  final String id;
  final int hour;
  final int minute;
  final String label;
  final bool isEnabled;

  AlarmModel({
    required this.id,
    required this.hour,
    required this.minute,
    required this.label,
    this.isEnabled = true,
  });

  AlarmModel copyWith({
    String? id,
    int? hour,
    int? minute,
    String? label,
    bool? isEnabled,
  }) {
    return AlarmModel(
      id: id ?? this.id,
      hour: hour ?? this.hour,
      minute: minute ?? this.minute,
      label: label ?? this.label,
      isEnabled: isEnabled ?? this.isEnabled,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'hour': hour,
      'minute': minute,
      'label': label,
      'isEnabled': isEnabled,
    };
  }

  factory AlarmModel.fromJson(Map<String, dynamic> json) {
    return AlarmModel(
      id: json['id'] as String,
      hour: json['hour'] as int,
      minute: json['minute'] as int,
      label: json['label'] as String? ?? 'Alarm',
      isEnabled: json['isEnabled'] as bool? ?? true,
    );
  }

  String formatTime(bool is12HourFormat) {
    if (is12HourFormat) {
      final dispHour = hour % 12 == 0 ? 12 : hour % 12;
      final ampm = hour >= 12 ? 'PM' : 'AM';
      final minStr = minute.toString().padLeft(2, '0');
      return '$dispHour:$minStr $ampm';
    } else {
      final hourStr = hour.toString().padLeft(2, '0');
      final minStr = minute.toString().padLeft(2, '0');
      return '$hourStr:$minStr';
    }
  }
}
