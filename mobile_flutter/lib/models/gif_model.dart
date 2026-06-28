class GifModel {
  final String id;
  final String name;
  final String category;
  bool favorite;
  bool selected;
  bool hidden;
  final int size;
  final String flashSize;
  final String? customData; // Base64 data or custom path if uploaded

  GifModel({
    required this.id,
    required this.name,
    required this.category,
    this.favorite = false,
    this.selected = true,
    this.hidden = false,
    required this.size,
    required this.flashSize,
    this.customData,
  });

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'category': category,
      'favorite': favorite,
      'selected': selected,
      'hidden': hidden,
      'size': size,
      'flashSize': flashSize,
      'customData': customData,
    };
  }

  factory GifModel.fromJson(Map<String, dynamic> json) {
    return GifModel(
      id: json['id'] as String,
      name: json['name'] as String,
      category: json['category'] as String,
      favorite: json['favorite'] as bool? ?? false,
      selected: json['selected'] as bool? ?? true,
      hidden: json['hidden'] as bool? ?? false,
      size: json['size'] as int? ?? 0,
      flashSize: json['flashSize'] as String? ?? '0.0KB',
      customData: json['customData'] as String?,
    );
  }

  GifModel copyWith({
    String? id,
    String? name,
    String? category,
    bool? favorite,
    bool? selected,
    bool? hidden,
    int? size,
    String? flashSize,
    String? customData,
  }) {
    return GifModel(
      id: id ?? this.id,
      name: name ?? this.name,
      category: category ?? this.category,
      favorite: favorite ?? this.favorite,
      selected: selected ?? this.selected,
      hidden: hidden ?? this.hidden,
      size: size ?? this.size,
      flashSize: flashSize ?? this.flashSize,
      customData: customData ?? this.customData,
    );
  }
}
