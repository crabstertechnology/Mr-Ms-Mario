import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:provider/provider.dart';
import 'package:firebase_core/firebase_core.dart';

import 'services/bluetooth_service.dart';
import 'services/database_service.dart';
import 'services/notification_service.dart';
import 'services/audio_stream_service.dart';
import 'services/firebase_service.dart';
import 'screens/main_dashboard.dart';
import 'screens/splash_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  try {
    await Firebase.initializeApp();
  } catch (e) {
    debugPrint("Firebase initialization failed: $e");
  }
  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => DatabaseService()),
        ChangeNotifierProvider(create: (_) => BLEService()),
        ChangeNotifierProvider(create: (_) => AudioStreamService()),
        ChangeNotifierProvider(create: (_) => FirebaseService()),
        ProxyProvider2<BLEService, DatabaseService, PhoneNotificationService>(
          create: (context) => PhoneNotificationService(
            Provider.of<BLEService>(context, listen: false),
            Provider.of<DatabaseService>(context, listen: false),
          ),
          update: (_, ble, db, previous) => previous ?? PhoneNotificationService(ble, db),
          lazy: false,
        ),
      ],
      child: const LunaControllerApp(),
    ),
  );
}

class LunaControllerApp extends StatelessWidget {
  const LunaControllerApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final db = Provider.of<DatabaseService>(context);
    final isMsLuna = db.primaryRobot?.variant == 'ms_luna';

    // Accent Colors
    final Color accentColor = isMsLuna ? const Color(0xFFEC4899) : const Color(0xFF0074D9);
    final Color accentColorLight = isMsLuna ? const Color(0x1FEC4899) : const Color(0x1F0074D9);
    final Color accentTextColor = isMsLuna ? const Color(0xFFDB2777) : const Color(0xFF1D4ED8);

    return MaterialApp(
      title: 'Mr.&Ms Luna',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        brightness: Brightness.light,
        scaffoldBackgroundColor: const Color(0xFFF8FAFC),
        primaryColor: accentColor,
        cardColor: Colors.white,
        sliderTheme: SliderThemeData(
          activeTrackColor: accentColor,
          inactiveTrackColor: Colors.black.withOpacity(0.06),
          thumbColor: accentColor,
          valueIndicatorColor: accentColor,
          overlayColor: accentColor.withOpacity(0.12),
        ),
        switchTheme: SwitchThemeData(
          thumbColor: WidgetStateProperty.resolveWith<Color>((states) {
            if (states.contains(WidgetState.selected)) {
              return Colors.white;
            }
            return Colors.grey.shade400;
          }),
          trackColor: WidgetStateProperty.resolveWith<Color>((states) {
            if (states.contains(WidgetState.selected)) {
              return accentColor;
            }
            return Colors.black.withOpacity(0.08);
          }),
        ),
        textTheme: GoogleFonts.outfitTextTheme(
          ThemeData.light().textTheme,
        ).apply(
          bodyColor: const Color(0xFF1E293B),
          displayColor: const Color(0xFF0F172A),
        ),
        elevatedButtonTheme: ElevatedButtonThemeData(
          style: ElevatedButton.styleFrom(
            backgroundColor: accentColor,
            foregroundColor: Colors.white,
            textStyle: GoogleFonts.outfit(
              fontWeight: FontWeight.bold,
              fontSize: 14,
            ),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(10),
            ),
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
          ),
        ),
        outlinedButtonTheme: OutlinedButtonThemeData(
          style: OutlinedButton.styleFrom(
            foregroundColor: accentTextColor,
            side: BorderSide(color: accentColor.withOpacity(0.3), width: 1),
            textStyle: GoogleFonts.outfit(
              fontWeight: FontWeight.bold,
              fontSize: 14,
            ),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(10),
            ),
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
          ),
        ),
      ),
      home: const SplashScreen(),
    );
  }
}
