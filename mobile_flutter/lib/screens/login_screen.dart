import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:firebase_auth/firebase_auth.dart';
import '../services/firebase_service.dart';
import 'main_dashboard.dart';

class LoginScreen extends StatefulWidget {
  const LoginScreen({Key? key}) : super(key: key);

  @override
  State<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends State<LoginScreen> {
  bool _isSignUp = false;
  bool _isLoading = false;

  final _formKey = GlobalKey<FormState>();
  final _emailController = TextEditingController();
  final _passwordController = TextEditingController();
  final _nameController = TextEditingController();
  final _robotNameController = TextEditingController(text: "Lumina");
  
  String _selectedRobotVariant = 'ms_luna'; // Default to Ms. Luna

  @override
  void dispose() {
    _emailController.dispose();
    _passwordController.dispose();
    _nameController.dispose();
    _robotNameController.dispose();
    super.dispose();
  }

  Future<void> _handleEmailAuth() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isLoading = true);
    final firebaseService = Provider.of<FirebaseService>(context, listen: false);

    try {
      if (_isSignUp) {
        // 1. Create User in Firebase Auth
        final credential = await FirebaseAuth.instance.createUserWithEmailAndPassword(
          email: _emailController.text.trim(),
          password: _passwordController.text.trim(),
        );

        final user = credential.user;
        if (user != null) {
          // Update display name
          await user.updateDisplayName(_nameController.text.trim());
          
          // Re-trigger sign in profile registration
          await firebaseService.signInWithGoogle(
            user.email!,
            _nameController.text.trim(),
            user.uid,
            _robotNameController.text.trim(),
            _selectedRobotVariant,
          );
        }
      } else {
        // 2. Sign In User in Firebase Auth
        final credential = await FirebaseAuth.instance.signInWithEmailAndPassword(
          email: _emailController.text.trim(),
          password: _passwordController.text.trim(),
        );

        final user = credential.user;
        if (user != null) {
          await firebaseService.signInWithGoogle(
            user.email!,
            user.displayName ?? 'Luna User',
            user.uid,
            _robotNameController.text.trim(),
            _selectedRobotVariant,
          );
        }
      }

      // Navigate to Dashboard
      if (mounted) {
        Navigator.of(context).pushReplacement(
          MaterialPageRoute(builder: (context) => const MainDashboard()),
        );
      }
    } on FirebaseAuthException catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(e.message ?? "Authentication failed"),
          backgroundColor: Colors.redAccent,
        ),
      );
    } finally {
      if (mounted) setState(() => _isLoading = false);
    }
  }

  Future<void> _handleGoogleAuth() async {
    setState(() => _isLoading = true);
    final firebaseService = Provider.of<FirebaseService>(context, listen: false);

    try {
      await firebaseService.signInWithGoogle(
        "sasi.dev@gmail.com",
        "Sasi Dev",
        "sasi",
        _robotNameController.text.trim(),
        _selectedRobotVariant,
      );

      if (firebaseService.isSignedIn && mounted) {
        Navigator.of(context).pushReplacement(
          MaterialPageRoute(builder: (context) => const MainDashboard()),
        );
      }
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Google Sign-In failed: $e"),
          backgroundColor: Colors.redAccent,
        ),
      );
    } finally {
      if (mounted) setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final isMsLuna = _selectedRobotVariant == 'ms_luna';
    final accentColor = isMsLuna ? const Color(0xFFEC4899) : const Color(0xFF0074D9);

    return Scaffold(
      body: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [
              accentColor.withOpacity(0.05),
              Colors.white,
            ],
          ),
        ),
        child: SafeArea(
          child: Center(
            child: SingleChildScrollView(
              padding: const EdgeInsets.symmetric(horizontal: 28.0),
              child: Form(
                key: _formKey,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    // Header Logo or Title
                    Center(
                      child: Container(
                        padding: const EdgeInsets.all(16),
                        decoration: BoxDecoration(
                          color: Colors.white,
                          shape: BoxShape.circle,
                          boxShadow: [
                            BoxShadow(
                              color: accentColor.withOpacity(0.1),
                              blurRadius: 20,
                              spreadRadius: 5,
                            )
                          ],
                        ),
                        child: Icon(
                          Icons.favorite_rounded,
                          size: 56,
                          color: accentColor,
                        ),
                      ),
                    ),
                    const SizedBox(height: 24),
                    Text(
                      _isSignUp ? "Create Account" : "Welcome Back",
                      textAlign: TextAlign.center,
                      style: GoogleFonts.outfit(
                        fontSize: 32,
                        fontWeight: FontWeight.bold,
                        color: Colors.grey[900],
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      _isSignUp 
                          ? "Sign up to pair and interact with your companion robot."
                          : "Sign in to connect to your live Mr.&Ms Luna robot.",
                      textAlign: TextAlign.center,
                      style: GoogleFonts.outfit(
                        fontSize: 15,
                        color: Colors.grey[600],
                      ),
                    ),
                    const SizedBox(height: 32),

                    // Toggle Tab
                    Container(
                      height: 50,
                      decoration: BoxDecoration(
                        color: Colors.grey[200],
                        borderRadius: BorderRadius.circular(16),
                      ),
                      child: Row(
                        children: [
                          Expanded(
                            child: GestureDetector(
                              onTap: () => setState(() => _isSignUp = false),
                              child: Container(
                                decoration: BoxDecoration(
                                  color: !_isSignUp ? Colors.white : Colors.transparent,
                                  borderRadius: BorderRadius.circular(14),
                                  boxShadow: !_isSignUp 
                                      ? [BoxShadow(color: Colors.black.withOpacity(0.05), blurRadius: 5)]
                                      : null,
                                ),
                                alignment: Alignment.center,
                                child: Text(
                                  "Login",
                                  style: GoogleFonts.outfit(
                                    fontWeight: FontWeight.bold,
                                    color: !_isSignUp ? Colors.grey[900] : Colors.grey[500],
                                  ),
                                ),
                              ),
                            ),
                          ),
                          Expanded(
                            child: GestureDetector(
                              onTap: () => setState(() => _isSignUp = true),
                              child: Container(
                                decoration: BoxDecoration(
                                  color: _isSignUp ? Colors.white : Colors.transparent,
                                  borderRadius: BorderRadius.circular(14),
                                  boxShadow: _isSignUp 
                                      ? [BoxShadow(color: Colors.black.withOpacity(0.05), blurRadius: 5)]
                                      : null,
                                ),
                                alignment: Alignment.center,
                                child: Text(
                                  "Sign Up",
                                  style: GoogleFonts.outfit(
                                    fontWeight: FontWeight.bold,
                                    color: _isSignUp ? Colors.grey[900] : Colors.grey[500],
                                  ),
                                ),
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(height: 24),

                    // Inputs
                    if (_isSignUp) ...[
                      TextFormField(
                        controller: _nameController,
                        decoration: InputDecoration(
                          labelText: "Display Name",
                          prefixIcon: const Icon(Icons.person_outline),
                          border: OutlineInputBorder(borderRadius: BorderRadius.circular(16)),
                        ),
                        validator: (v) => v == null || v.isEmpty ? "Name is required" : null,
                      ),
                      const SizedBox(height: 16),
                    ],

                    TextFormField(
                      controller: _emailController,
                      keyboardType: TextInputType.emailAddress,
                      decoration: InputDecoration(
                        labelText: "Email Address",
                        prefixIcon: const Icon(Icons.email_outlined),
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(16)),
                      ),
                      validator: (v) => v == null || !v.contains('@') ? "Enter a valid email" : null,
                    ),
                    const SizedBox(height: 16),

                    TextFormField(
                      controller: _passwordController,
                      obscureText: true,
                      decoration: InputDecoration(
                        labelText: "Password",
                        prefixIcon: const Icon(Icons.lock_outline),
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(16)),
                      ),
                      validator: (v) => v == null || v.length < 6 ? "Password must be 6+ chars" : null,
                    ),
                    const SizedBox(height: 16),

                    // Robot Variant Config
                    TextFormField(
                      controller: _robotNameController,
                      decoration: InputDecoration(
                        labelText: "Robot Nickname",
                        prefixIcon: const Icon(Icons.android_outlined),
                        border: OutlineInputBorder(borderRadius: BorderRadius.circular(16)),
                      ),
                    ),
                    const SizedBox(height: 16),

                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          "Robot Variant:",
                          style: GoogleFonts.outfit(fontWeight: FontWeight.bold, fontSize: 16),
                        ),
                        Row(
                          children: [
                            ChoiceChip(
                              label: const Text("Ms. Luna (Pink)"),
                              selected: _selectedRobotVariant == 'ms_luna',
                              selectedColor: const Color(0xFFEC4899).withOpacity(0.2),
                              onSelected: (val) {
                                if (val) setState(() => _selectedRobotVariant = 'ms_luna');
                              },
                            ),
                            const SizedBox(width: 8),
                            ChoiceChip(
                              label: const Text("Mr. Luna (Blue)"),
                              selected: _selectedRobotVariant == 'mr_luna',
                              selectedColor: const Color(0xFF0074D9).withOpacity(0.2),
                              onSelected: (val) {
                                if (val) setState(() => _selectedRobotVariant = 'mr_luna');
                              },
                            ),
                          ],
                        ),
                      ],
                    ),
                    const SizedBox(height: 28),

                    // Action Buttons
                    ElevatedButton(
                      onPressed: _isLoading ? null : _handleEmailAuth,
                      style: ElevatedButton.styleFrom(
                        backgroundColor: accentColor,
                        padding: const EdgeInsets.symmetric(vertical: 16),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
                      ),
                      child: _isLoading
                          ? const SizedBox(
                              height: 20,
                              width: 20,
                              child: CircularProgressIndicator(color: Colors.white, strokeWidth: 2),
                            )
                          : Text(
                              _isSignUp ? "Create Account" : "Sign In",
                              style: GoogleFonts.outfit(fontSize: 16, fontWeight: FontWeight.bold, color: Colors.white),
                            ),
                    ),
                    const SizedBox(height: 16),

                    // Divider
                    Row(
                      children: [
                        Expanded(child: Divider(color: Colors.grey[300])),
                        Padding(
                          padding: const EdgeInsets.symmetric(horizontal: 16.0),
                          child: Text("OR", style: GoogleFonts.outfit(color: Colors.grey[400])),
                        ),
                        Expanded(child: Divider(color: Colors.grey[300])),
                      ],
                    ),
                    const SizedBox(height: 16),

                    // Google Sign-In Button
                    OutlinedButton.icon(
                      onPressed: _isLoading ? null : _handleGoogleAuth,
                      icon: Image.network(
                        'https://upload.wikimedia.org/wikipedia/commons/thumb/c/c1/Google_%22G%22_logo.svg/24px-Google_%22G%22_logo.svg.png',
                        height: 20,
                      ),
                      label: Text(
                        "Sign In with Google",
                        style: GoogleFonts.outfit(color: Colors.grey[700], fontWeight: FontWeight.bold),
                      ),
                      style: OutlinedButton.styleFrom(
                        padding: const EdgeInsets.symmetric(vertical: 16),
                        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
                        side: BorderSide(color: Colors.grey[300]!),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
