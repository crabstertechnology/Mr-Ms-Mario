import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:qr_flutter/qr_flutter.dart';
import 'package:share_plus/share_plus.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:url_launcher/url_launcher.dart';

import '../services/bluetooth_service.dart';
import '../widgets/glass_card.dart';

class BusinessCardScreen extends StatefulWidget {
  const BusinessCardScreen({Key? key}) : super(key: key);

  @override
  State<BusinessCardScreen> createState() => _BusinessCardScreenState();
}

class _BusinessCardScreenState extends State<BusinessCardScreen> {
  final _formKey = GlobalKey<FormState>();
  bool _useCustomUrl = true;

  // Custom URL Field
  final _customUrlCtrl = TextEditingController();

  // Card Builder Fields
  final _nameCtrl = TextEditingController();
  final _companyCtrl = TextEditingController();
  final _titleCtrl = TextEditingController();
  final _phoneCtrl = TextEditingController();
  final _emailCtrl = TextEditingController();
  final _websiteCtrl = TextEditingController();
  final _instagramCtrl = TextEditingController();
  final _linkedinCtrl = TextEditingController();
  final _locationCtrl = TextEditingController();
  final _bioCtrl = TextEditingController();
  final _avatarUrlCtrl = TextEditingController();
  final _baseDomainCtrl = TextEditingController(text: "https://luna.ezcirkit.com/card/");

  String _generatedUrl = "";
  bool _isSyncing = false;

  @override
  void initState() {
    super.initState();
    _loadSavedCardData();
  }

  @override
  void dispose() {
    _customUrlCtrl.dispose();
    _nameCtrl.dispose();
    _companyCtrl.dispose();
    _titleCtrl.dispose();
    _phoneCtrl.dispose();
    _emailCtrl.dispose();
    _websiteCtrl.dispose();
    _instagramCtrl.dispose();
    _linkedinCtrl.dispose();
    _locationCtrl.dispose();
    _bioCtrl.dispose();
    _avatarUrlCtrl.dispose();
    _baseDomainCtrl.dispose();
    super.dispose();
  }

  Future<void> _loadSavedCardData() async {
    final prefs = await SharedPreferences.getInstance();
    setState(() {
      _useCustomUrl = true;
      _customUrlCtrl.text = prefs.getString("card_custom_url") ?? prefs.getString("card_generated_url") ?? "";
      
      _nameCtrl.text = prefs.getString("card_name") ?? "";
      _companyCtrl.text = prefs.getString("card_company") ?? "";
      _titleCtrl.text = prefs.getString("card_title") ?? "";
      _phoneCtrl.text = prefs.getString("card_phone") ?? "";
      _emailCtrl.text = prefs.getString("card_email") ?? "";
      _websiteCtrl.text = prefs.getString("card_website") ?? "";
      _instagramCtrl.text = prefs.getString("card_instagram") ?? "";
      _linkedinCtrl.text = prefs.getString("card_linkedin") ?? "";
      _locationCtrl.text = prefs.getString("card_location") ?? "";
      _bioCtrl.text = prefs.getString("card_bio") ?? "";
      _avatarUrlCtrl.text = prefs.getString("card_avatar") ?? "";
      _baseDomainCtrl.text = prefs.getString("card_base_domain") ?? "https://luna.ezcirkit.com/card/";
      
      _generatedUrl = prefs.getString("card_generated_url") ?? "";
    });
  }

  Future<bool> _saveAndGenerateCard({bool showSnackBar = true}) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("card_use_custom_url", _useCustomUrl);

    if (_useCustomUrl) {
      final url = _customUrlCtrl.text.trim();
      if (url.isEmpty) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text("Please enter a valid URL")),
        );
        return false;
      }
      await prefs.setString("card_custom_url", url);
      await prefs.setString("card_generated_url", url);
      setState(() {
        _generatedUrl = url;
      });
    } else {
      if (_formKey.currentState == null || !_formKey.currentState!.validate()) return false;

      await prefs.setString("card_name", _nameCtrl.text.trim());
      await prefs.setString("card_company", _companyCtrl.text.trim());
      await prefs.setString("card_title", _titleCtrl.text.trim());
      await prefs.setString("card_phone", _phoneCtrl.text.trim());
      await prefs.setString("card_email", _emailCtrl.text.trim());
      await prefs.setString("card_website", _websiteCtrl.text.trim());
      await prefs.setString("card_instagram", _instagramCtrl.text.trim());
      await prefs.setString("card_linkedin", _linkedinCtrl.text.trim());
      await prefs.setString("card_location", _locationCtrl.text.trim());
      await prefs.setString("card_bio", _bioCtrl.text.trim());
      await prefs.setString("card_avatar", _avatarUrlCtrl.text.trim());
      await prefs.setString("card_base_domain", _baseDomainCtrl.text.trim());

      final sanitizedName = _nameCtrl.text.trim().toLowerCase().replaceAll(RegExp(r'[^a-z0-9]'), '');
      final uniqueId = sanitizedName.isNotEmpty ? sanitizedName : "user${DateTime.now().millisecondsSinceEpoch % 100000}";
      
      var base = _baseDomainCtrl.text.trim();
      if (!base.endsWith("/")) base += "/";
      final finalUrl = "$base$uniqueId";

      await prefs.setString("card_generated_url", finalUrl);
      setState(() {
        _generatedUrl = finalUrl;
      });
    }

    if (showSnackBar) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Business card generated! QR matches: $_generatedUrl"),
          backgroundColor: Colors.green,
        ),
      );
    }
    return true;
  }

  Future<void> _syncToLuna(BLEService ble) async {
    if (!ble.isConnected) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("Luna not connected. Please connect via BLE first."),
          backgroundColor: Colors.redAccent,
        ),
      );
      return;
    }

    // Automatically generate and validate before syncing so changes sync immediately
    final generatedSuccessfully = await _saveAndGenerateCard(showSnackBar: false);
    if (!generatedSuccessfully || _generatedUrl.isEmpty) {
      return;
    }

    setState(() {
      _isSyncing = true;
    });

    final success = await ble.transmitBusinessCardUrl(_generatedUrl);

    setState(() {
      _isSyncing = false;
    });

    if (success) {
      showDialog(
        context: context,
        builder: (ctx) => AlertDialog(
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
          title: Row(
            children: const [
              Icon(Icons.check_circle, color: Colors.green),
              SizedBox(width: 8),
              Text("Sync Success"),
            ],
          ),
          content: const Text(
            "QR Successfully Saved to Luna!\n\nIt is now permanently stored in NVS. Press Button 2 on your watch to display it at any time.",
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(ctx),
              child: const Text("Awesome"),
            )
          ],
        ),
      );
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("Unable to save QR to Luna. Please try again."),
          backgroundColor: Colors.redAccent,
        ),
      );
    }
  }

  Future<void> _clearLunaCard(BLEService ble) async {
    if (!ble.isConnected) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text("Luna not connected.")),
      );
      return;
    }

    final success = await ble.transmitClearBusinessCard();
    if (success) {
      final prefs = await SharedPreferences.getInstance();
      await prefs.remove("card_generated_url");
      await prefs.remove("card_custom_url");
      
      setState(() {
        _generatedUrl = "";
        _customUrlCtrl.clear();
      });

      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("Business card cleared from Luna and app memory."),
          backgroundColor: Colors.green,
        ),
      );
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text("Failed to clear card from Luna.")),
      );
    }
  }


  @override
  Widget build(BuildContext context) {
    final ble = Provider.of<BLEService>(context);
    final accentColor = ble.connectedDevice?.platformName.contains('Ms. Luna') == true
        ? const Color(0xFFEC4899)
        : const Color(0xFF0074D9);

    return Scaffold(
      backgroundColor: Colors.transparent,
      body: SingleChildScrollView(
        padding: const EdgeInsets.only(top: 10, bottom: 40),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              "DIGITAL BUSINESS CARD",
              style: GoogleFonts.outfit(
                color: const Color(0xFF0F172A),
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 4),
            Text(
              "Create your contact link, generate a QR code, and permanently sync it to your Luna smartwatch.",
              style: GoogleFonts.outfit(
                color: const Color(0xFF475569),
                fontSize: 13,
              ),
            ),
            const SizedBox(height: 20),

            // Direct Link Configuration
            GlassCard(
              padding: const EdgeInsets.all(20),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Text(
                    "Direct Link Configuration",
                    style: GoogleFonts.outfit(
                      fontSize: 14,
                      fontWeight: FontWeight.bold,
                      color: const Color(0xFF0F172A),
                    ),
                  ),
                  const SizedBox(height: 6),
                  Text(
                    "Enter any existing URL you want the watch QR code to open directly (e.g. Linktree, LinkedIn, GitHub, personal website).",
                    style: GoogleFonts.outfit(fontSize: 12, color: const Color(0xFF64748B)),
                  ),
                  const SizedBox(height: 16),
                  _buildTextField(
                    _customUrlCtrl,
                    "Custom URL (e.g. https://linktr.ee/myname)",
                    Icons.link,
                    isRequired: true,
                    keyboardType: TextInputType.url,
                  ),
                  const SizedBox(height: 20),
                  ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: accentColor,
                      padding: const EdgeInsets.symmetric(vertical: 14),
                      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                      elevation: 0,
                    ),
                    onPressed: _saveAndGenerateCard,
                    child: Text(
                      "Generate Link QR Code",
                      style: GoogleFonts.outfit(fontWeight: FontWeight.bold, color: Colors.white),
                    ),
                  ),
                ],
              ),
            ),

            const SizedBox(height: 20),

            // QR Preview & Action Section
            if (_generatedUrl.isNotEmpty) ...[
              GlassCard(
                padding: const EdgeInsets.all(20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    Text(
                      "YOUR CARD QR CODE",
                      style: GoogleFonts.outfit(
                        fontSize: 14,
                        fontWeight: FontWeight.bold,
                        color: const Color(0xFF0F172A),
                      ),
                    ),
                    const SizedBox(height: 6),
                    Text(
                      "Scannable preview for 1.8\" TFT display calibration",
                      style: GoogleFonts.outfit(fontSize: 11, color: const Color(0xFF64748B)),
                    ),
                    const SizedBox(height: 16),

                    // QR Container with quiet zone
                    Container(
                      padding: const EdgeInsets.all(12),
                      decoration: BoxDecoration(
                        color: Colors.white,
                        borderRadius: BorderRadius.circular(16),
                        border: Border.all(color: const Color(0xFFE2E8F0)),
                      ),
                      child: QrImageView(
                        data: _generatedUrl,
                        version: QrVersions.auto,
                        size: 160.0,
                        gapless: false,
                      ),
                    ),
                    const SizedBox(height: 12),
                    Text(
                      _generatedUrl,
                      style: GoogleFonts.firaCode(fontSize: 11, color: const Color(0xFF475569)),
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 20),

                    // URL Actions Row
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        _buildActionIcon(Icons.open_in_browser, "Preview", () async {
                          final uri = Uri.parse(_generatedUrl);
                          if (await canLaunchUrl(uri)) {
                            await launchUrl(uri, mode: LaunchMode.externalApplication);
                          }
                        }),
                        _buildActionIcon(Icons.copy, "Copy Link", () {
                          Clipboard.setData(ClipboardData(text: _generatedUrl));
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(content: Text("Link copied to clipboard!")),
                          );
                        }),
                        _buildActionIcon(Icons.share, "Share", () {
                          Share.share("Check out my digital business card: $_generatedUrl");
                        }),
                      ],
                    ),

                    const Divider(height: 32, color: Color(0xFFE2E8F0)),

                    // Sync Options
                    if (_isSyncing)
                      const CircularProgressIndicator()
                    else ...[
                      ElevatedButton.icon(
                        icon: const Icon(Icons.sync, color: Colors.white),
                        label: Text(
                          "Send / Sync to Luna",
                          style: GoogleFonts.outfit(fontWeight: FontWeight.bold, color: Colors.white),
                        ),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.green.shade600,
                          padding: const EdgeInsets.symmetric(vertical: 14, horizontal: 24),
                          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
                          elevation: 0,
                        ),
                        onPressed: () => _syncToLuna(ble),
                      ),
                      const SizedBox(height: 8),
                      TextButton.icon(
                        icon: const Icon(Icons.delete_outline, color: Colors.redAccent, size: 18),
                        label: Text(
                          "Clear Stored Card on Luna",
                          style: GoogleFonts.outfit(color: Colors.redAccent, fontSize: 13),
                        ),
                        onPressed: () => _clearLunaCard(ble),
                      ),
                    ]
                  ],
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  Widget _buildTextField(
    TextEditingController controller,
    String label,
    IconData icon, {
    bool isRequired = false,
    int maxLines = 1,
    TextInputType keyboardType = TextInputType.text,
  }) {
    return TextFormField(
      controller: controller,
      keyboardType: keyboardType,
      maxLines: maxLines,
      style: GoogleFonts.outfit(color: const Color(0xFF0F172A), fontSize: 14),
      decoration: InputDecoration(
        labelText: label,
        labelStyle: GoogleFonts.outfit(fontSize: 12, color: const Color(0xFF64748B)),
        prefixIcon: Icon(icon, size: 18, color: const Color(0xFF64748B)),
        border: OutlineInputBorder(borderRadius: BorderRadius.circular(12)),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(12),
          borderSide: BorderSide(color: const Color(0xFF0F172A).withOpacity(0.15)),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(12),
          borderSide: const BorderSide(color: Color(0xFF0074D9), width: 1.5),
        ),
        contentPadding: const EdgeInsets.symmetric(vertical: 12, horizontal: 16),
      ),
      validator: isRequired
          ? (val) => val == null || val.trim().isEmpty ? "$label is required" : null
          : null,
    );
  }

  Widget _buildActionIcon(IconData icon, String label, VoidCallback onTap) {
    return InkWell(
      onTap: onTap,
      borderRadius: BorderRadius.circular(8),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: [
            Icon(icon, color: const Color(0xFF475569), size: 22),
            const SizedBox(height: 4),
            Text(
              label,
              style: GoogleFonts.outfit(fontSize: 11, color: const Color(0xFF475569), fontWeight: FontWeight.w500),
            )
          ],
        ),
      ),
    );
  }
}
