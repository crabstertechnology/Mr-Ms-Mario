class CalendarEvent {
  final String id;
  final String title;
  final DateTime dateTime;
  final String type; // 'meeting' or 'birthday'

  CalendarEvent({
    required this.id,
    required this.title,
    required this.dateTime,
    required this.type,
  });

  Map<String, dynamic> toJson() => {
        'id': id,
        'title': title,
        'dateTime': dateTime.toIso8601String(),
        'type': type,
      };

  factory CalendarEvent.fromJson(Map<String, dynamic> json) => CalendarEvent(
        id: json['id'] as String,
        title: json['title'] as String,
        dateTime: DateTime.parse(json['dateTime'] as String),
        type: json['type'] as String,
      );
}
