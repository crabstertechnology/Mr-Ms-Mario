# Mr.&Ms Luna Mobile Controller (Flutter Native App)

This is the native **Flutter companion mobile application** for the **Mr.&Ms Luna Smart Toy Robot**. It is a direct port of the custom Web Bluetooth dashboard, rebuilt from scratch with a premium mobile-first UI, responsive glassmorphism, offline square-wave synthesizer, and native Bluetooth low energy drivers.

---

## 📱 Features

1. **Expressions & Library Grid**:
   - Live **SSD1306 OLED simulator** rendering active eyes or scrolling marquee banners.
   - Scan, pair, and connect directly to the ESP32-C3 via native Bluetooth.
   - Live diagnostics displaying robot uptime, gesture tap counts, and battery status bar.
   - Dynamic search, alphabetical/favorite sorting, hiding, and deleting.
   - **Custom GIF conversions**: Import any external GIF from local phone storage, configure compression rates, estimate bytes, and save to the library!
   - Active console logs tracking real-time status packets, sync operations, and command history.

2. **8-bit Synthesizer & Soundboard**:
   - Synthesizer composer note writer parsing standard frequency duration notation.
   - **Local Square-Wave Synth**: Generates custom PCM WAV audio streams dynamically in pure Dart to preview melodies offline without lagging.
   - AI Music Composer mapping user prompts (coin, jump, victory, death) to custom note configurations.
   - Tap board triggering nostalgic Luna audio sound effects.

3. **Chronos RTC Sync**:
   - One-tap RTC synchronization alignment matching your smartphone's system clock.

4. **Hardware Config Panel**:
   - Contrast/Brightness parameters adjustments.
   - Custom mappings configuring triggers for Single Tap, Double Tap, and Long Touch gestures.
   - BLE device broadcast name adjustments and factory reset command syncs.

5. **16x8 Pixel Canvas**:
   - Touch drag grids painting and erasing custom bitmap sprites.
   - Automatically formats and copies raw C++ PROGMEM hexadecimal bytes array structure.

---

## 🛠️ Getting Started & Run Instructions

To compile and launch the application on a physical device:

### 1. Initialize Platforms
Run the standard Flutter generator command in this folder. It will parse the existing `pubspec.yaml` and `lib/` directory to generate the platform-specific native configuration envelopes (Android / iOS):
```bash
flutter create --org com.mrmsluna.controller .
```

### 2. Configure Native Permissions

#### **Android (`android/app/src/main/AndroidManifest.xml`)**
Add the following Bluetooth permissions inside the `<manifest>` tag:
```xml
<uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

#### **iOS (`ios/Runner/Info.plist`)**
Add the following descriptions inside the main `<dict>` block:
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app requires Bluetooth access to connect and control the Mr.&Ms Luna toy robot.</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app requires Bluetooth access to control the Mr.&Ms Luna toy robot.</string>
```

### 3. Fetch Dependencies & Run
Connect your smartphone via USB-C or open an emulator and execute:
```bash
flutter pub get
flutter run --release
```
