import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:firebase_auth/firebase_auth.dart';
import '../services/bluetooth_service.dart';
import '../services/database_service.dart';
import 'main_dashboard.dart';

class OnboardingScreen extends StatefulWidget {
  const OnboardingScreen({Key? key}) : super(key: key);

  @override
  State<OnboardingScreen> createState() => _OnboardingScreenState();
}

class _OnboardingScreenState extends State<OnboardingScreen> {
  final PageController _pageController = PageController();
  int _currentPage = 0;

  final TextEditingController _nameController = TextEditingController(text: "Sasi");
  final TextEditingController _dobController = TextEditingController(text: "2000-01-01");
  DateTime _selectedDate = DateTime(2000, 1, 1);

  @override
  void initState() {
    super.initState();
    _loadExistingProfile();
  }

  Future<void> _loadExistingProfile() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      final savedName = prefs.getString('user_name');
      final savedDob = prefs.getString('user_dob');
      if (savedName != null && savedName.isNotEmpty) {
        _nameController.text = savedName;
      }
      if (savedDob != null && savedDob.isNotEmpty) {
        _dobController.text = savedDob;
      }
    } catch (_) {}
  }

  @override
  void dispose() {
    _pageController.dispose();
    _nameController.dispose();
    _dobController.dispose();
    super.dispose();
  }

  Future<void> _selectDate(BuildContext context, Color accentColor) async {
    final DateTime? picked = await showDatePicker(
      context: context,
      initialDate: _selectedDate,
      firstDate: DateTime(1950),
      lastDate: DateTime.now(),
      builder: (context, child) {
        return Theme(
          data: Theme.of(context).copyWith(
            colorScheme: ColorScheme.light(
              primary: accentColor,
              onPrimary: Colors.white,
              onSurface: const Color(0xFF0F172A),
            ),
          ),
          child: child!,
        );
      },
    );
    if (picked != null) {
      setState(() {
        _selectedDate = picked;
        _dobController.text =
            "${picked.year}-${picked.month.toString().padLeft(2, '0')}-${picked.day.toString().padLeft(2, '0')}";
      });
    }
  }

  Future<void> _completeOnboarding() async {
    final name = _nameController.text.trim().isEmpty ? "Sasi" : _nameController.text.trim();
    final dob = _dobController.text.trim();

    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool('onboarding_completed', true);
    await prefs.setString('user_name', name);
    await prefs.setString('user_dob', dob);

    // Sync to hardware if connected
    if (mounted) {
      final ble = Provider.of<BLEService>(context, listen: false);
      if (ble.isConnected) {
        ble.sendCommand("SET_NAME:$name");
        if (dob.isNotEmpty) {
          ble.sendCommand("SET_DOB:$dob");
        }
      }

      Navigator.of(context).pushReplacement(
        PageRouteBuilder(
          pageBuilder: (context, animation, secondaryAnimation) => const MainDashboard(),
          transitionsBuilder: (context, animation, secondaryAnimation, child) {
            return FadeTransition(opacity: animation, child: child);
          },
          transitionDuration: const Duration(milliseconds: 600),
        ),
      );
    }
  }

  void _nextPage() {
    if (_currentPage < 3) {
      _pageController.nextPage(
        duration: const Duration(milliseconds: 400),
        curve: Curves.easeInOutCubic,
      );
    } else {
      _completeOnboarding();
    }
  }

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);
    final isMsLuna = db.primaryRobot?.variant == 'ms_luna';
    final Color accentColor = isMsLuna ? const Color(0xFFEC4899) : const Color(0xFF0074D9);
    final Color secondaryColor = isMsLuna ? const Color(0xFFF43F5E) : const Color(0xFF0284C7);

    return Scaffold(
      backgroundColor: const Color(0xFFF8FAFC),
      body: SafeArea(
        child: Column(
          children: [
            // Top Bar with Skip Button
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 14),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Row(
                    children: [
                      Container(
                        padding: const EdgeInsets.all(8),
                        decoration: BoxDecoration(
                          gradient: LinearGradient(
                            colors: [accentColor, secondaryColor],
                          ),
                          borderRadius: BorderRadius.circular(12),
                          boxShadow: [
                            BoxShadow(
                              color: accentColor.withOpacity(0.25),
                              blurRadius: 10,
                              offset: const Offset(0, 3),
                            ),
                          ],
                        ),
                        child: const Icon(Icons.smart_toy_rounded, color: Colors.white, size: 20),
                      ),
                      const SizedBox(width: 10),
                      Text(
                        "LUNA STUDIO",
                        style: GoogleFonts.outfit(
                          color: const Color(0xFF0F172A),
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                          letterSpacing: 1.5,
                        ),
                      ),
                    ],
                  ),
                  if (_currentPage < 3)
                    TextButton(
                      onPressed: _completeOnboarding,
                      child: Text(
                        "Skip",
                        style: GoogleFonts.outfit(
                          color: const Color(0xFF64748B),
                          fontSize: 14,
                          fontWeight: FontWeight.w600,
                        ),
                      ),
                    ),
                ],
              ),
            ),

            // Page View Cards
            Expanded(
              child: PageView(
                controller: _pageController,
                onPageChanged: (index) {
                  setState(() => _currentPage = index);
                },
                children: [
                  _buildWelcomeCard(accentColor, secondaryColor),
                  _buildNameCard(accentColor),
                  _buildBirthdayCard(accentColor),
                  _buildEvolutionCard(accentColor),
                ],
              ),
            ),

            // Bottom Navigation & Dots Indicator
            Padding(
              padding: const EdgeInsets.all(24.0),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  // Dot Indicators
                  Row(
                    children: List.generate(4, (index) {
                      final bool isSelected = index == _currentPage;
                      return AnimatedContainer(
                        duration: const Duration(milliseconds: 300),
                        margin: const EdgeInsets.only(right: 8),
                        height: 8,
                        width: isSelected ? 24 : 8,
                        decoration: BoxDecoration(
                          color: isSelected ? accentColor : const Color(0xFFCBD5E1),
                          borderRadius: BorderRadius.circular(4),
                        ),
                      );
                    }),
                  ),

                  // Next / Get Started Action Button
                  ElevatedButton(
                    onPressed: _nextPage,
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(horizontal: 28, vertical: 14),
                      backgroundColor: accentColor,
                      elevation: 4,
                      shadowColor: accentColor.withOpacity(0.35),
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(14),
                      ),
                    ),
                    child: Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Text(
                          _currentPage == 3 ? "Get Started" : "Continue",
                          style: GoogleFonts.outfit(
                            color: Colors.white,
                            fontSize: 15,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const SizedBox(width: 8),
                        Icon(
                          _currentPage == 3 ? Icons.check_circle_rounded : Icons.arrow_forward_rounded,
                          color: Colors.white,
                          size: 18,
                        ),
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildCardContainer({required Widget child, required Color accentColor}) {
    return Container(
      margin: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
      padding: const EdgeInsets.all(26),
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(28),
        border: Border.all(color: const Color(0xFFE2E8F0), width: 1.2),
        boxShadow: [
          BoxShadow(
            color: const Color(0xFF64748B).withOpacity(0.08),
            blurRadius: 24,
            offset: const Offset(0, 8),
          ),
          BoxShadow(
            color: accentColor.withOpacity(0.05),
            blurRadius: 10,
            offset: const Offset(0, 2),
          ),
        ],
      ),
      child: child,
    );
  }

  Widget _buildWelcomeCard(Color accentColor, Color secondaryColor) {
    return _buildCardContainer(
      accentColor: accentColor,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Container(
            width: 110,
            height: 110,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              gradient: LinearGradient(
                colors: [accentColor, secondaryColor],
                begin: Alignment.topLeft,
                end: Alignment.bottomRight,
              ),
              boxShadow: [
                BoxShadow(
                  color: accentColor.withOpacity(0.35),
                  blurRadius: 24,
                  spreadRadius: 2,
                  offset: const Offset(0, 6),
                ),
              ],
            ),
            child: const Icon(Icons.smart_toy_outlined, color: Colors.white, size: 55),
          ),
          const SizedBox(height: 28),
          Text(
            "Meet Mr. & Ms. Luna",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF0F172A),
              fontSize: 26,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 12),
          Text(
            "Your intelligent desktop robot companion. Luna expresses 12 dynamic moods, feels hungry during meal times, and evolves as you bond together!",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF64748B),
              fontSize: 14,
              height: 1.5,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildNameCard(Color accentColor) {
    return _buildCardContainer(
      accentColor: accentColor,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Container(
            padding: const EdgeInsets.all(18),
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: accentColor.withOpacity(0.1),
              border: Border.all(color: accentColor.withOpacity(0.25)),
            ),
            child: Icon(Icons.person_rounded, color: accentColor, size: 48),
          ),
          const SizedBox(height: 24),
          Text(
            "What's your name?",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF0F172A),
              fontSize: 24,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 10),
          Text(
            "Luna greets you personally on boot and expresses thoughts tailored to you.",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF64748B),
              fontSize: 13,
              height: 1.4,
            ),
          ),
          const SizedBox(height: 28),
          TextField(
            controller: _nameController,
            style: GoogleFonts.outfit(color: const Color(0xFF0F172A), fontSize: 16, fontWeight: FontWeight.bold),
            decoration: InputDecoration(
              filled: true,
              fillColor: const Color(0xFFF8FAFC),
              hintText: "Enter your name (e.g. Sasi)",
              hintStyle: GoogleFonts.outfit(color: const Color(0xFF94A3B8), fontSize: 14),
              prefixIcon: Icon(Icons.badge_outlined, color: accentColor),
              border: OutlineInputBorder(
                borderRadius: BorderRadius.circular(16),
                borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
              ),
              enabledBorder: OutlineInputBorder(
                borderRadius: BorderRadius.circular(16),
                borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
              ),
              focusedBorder: OutlineInputBorder(
                borderRadius: BorderRadius.circular(16),
                borderSide: BorderSide(color: accentColor, width: 1.8),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildBirthdayCard(Color accentColor) {
    return _buildCardContainer(
      accentColor: accentColor,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Container(
            padding: const EdgeInsets.all(18),
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: const Color(0xFFEC4899).withOpacity(0.1),
              border: Border.all(color: const Color(0xFFEC4899).withOpacity(0.25)),
            ),
            child: const Icon(Icons.cake_rounded, color: Color(0xFFEC4899), size: 48),
          ),
          const SizedBox(height: 24),
          Text(
            "When is your Birthday?",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF0F172A),
              fontSize: 24,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 10),
          Text(
            "Luna counts the days and celebrates your special day with exclusive melodies and animations!",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF64748B),
              fontSize: 13,
              height: 1.4,
            ),
          ),
          const SizedBox(height: 28),
          InkWell(
            onTap: () => _selectDate(context, accentColor),
            borderRadius: BorderRadius.circular(16),
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 16),
              decoration: BoxDecoration(
                color: const Color(0xFFF8FAFC),
                borderRadius: BorderRadius.circular(16),
                border: Border.all(color: const Color(0xFFE2E8F0)),
              ),
              child: Row(
                children: [
                  const Icon(Icons.calendar_today_rounded, color: Color(0xFFEC4899), size: 20),
                  const SizedBox(width: 14),
                  Expanded(
                    child: Text(
                      _dobController.text.isEmpty ? "Tap to select Birthday" : _dobController.text,
                      style: GoogleFonts.outfit(
                        color: const Color(0xFF0F172A),
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ),
                  const Icon(Icons.edit_calendar_rounded, color: Color(0xFF94A3B8), size: 20),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildEvolutionCard(Color accentColor) {
    return _buildCardContainer(
      accentColor: accentColor,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Container(
            padding: const EdgeInsets.all(18),
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: const Color(0xFF10B981).withOpacity(0.1),
              border: Border.all(color: const Color(0xFF10B981).withOpacity(0.25)),
            ),
            child: const Icon(Icons.auto_awesome_rounded, color: Color(0xFF10B981), size: 48),
          ),
          const SizedBox(height: 24),
          Text(
            "Pet XP & Evolution",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF0F172A),
              fontSize: 24,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 12),
          Text(
            "Luna feels hungry during Breakfast, Lunch & Dinner. Feed via the power button to earn +50 XP and watch Luna evolve from Baby Luna to Omega Luna!",
            textAlign: TextAlign.center,
            style: GoogleFonts.outfit(
              color: const Color(0xFF64748B),
              fontSize: 13,
              height: 1.5,
            ),
          ),
          const SizedBox(height: 20),
          // Evolution Stages Pills
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: const Color(0xFFF1F5F9),
              borderRadius: BorderRadius.circular(16),
              border: Border.all(color: const Color(0xFFE2E8F0)),
            ),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _buildStagePill("Baby", "Lv.1-4", accentColor),
                const Icon(Icons.arrow_forward_ios, size: 12, color: Color(0xFF94A3B8)),
                _buildStagePill("Mochi", "Lv.5-9", accentColor),
                const Icon(Icons.arrow_forward_ios, size: 12, color: Color(0xFF94A3B8)),
                _buildStagePill("Cyber", "Lv.10-19", accentColor),
                const Icon(Icons.arrow_forward_ios, size: 12, color: Color(0xFF94A3B8)),
                _buildStagePill("Omega", "Lv.20+", accentColor),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildStagePill(String title, String level, Color accentColor) {
    return Column(
      children: [
        Text(
          title,
          style: GoogleFonts.outfit(color: const Color(0xFF0F172A), fontSize: 12, fontWeight: FontWeight.bold),
        ),
        const SizedBox(height: 2),
        Text(
          level,
          style: GoogleFonts.outfit(color: accentColor, fontSize: 10, fontWeight: FontWeight.w600),
        ),
      ],
    );
  }
}
