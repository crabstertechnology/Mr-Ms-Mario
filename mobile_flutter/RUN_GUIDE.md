# Mr.&Ms Luna Mobile Controller — Developer Run & Setup Guide

This guide details the step-by-step process to set up, build, and run the Mr.&Ms Luna Flutter controller app on a mobile device (Android or iOS) after cloning the repository.

---

## 📋 Prerequisites

Before starting, ensure your development machine has the following tools installed:

### 1. Flutter SDK
*   **Version:** Flutter 3.10+ (Dart 3.0+)
*   **Installation:** [Flutter Install Guide](https://docs.flutter.dev/get-started/install)
*   **Verification:** Run `flutter doctor` in your terminal to ensure there are no setup blockages.

### 2. Native Tools
*   **For Android:** Android Studio, Android SDK Command-line Tools, and JDK 17.
*   **For iOS (Mac only):** Xcode and CocoaPods.

---

## 🛠️ Step-by-Step Device Setup

### 🤖 Android Setup (Physical Device)
To test and run the app on a physical Android phone:
1.  **Enable Developer Options:**
    *   Open **Settings** ➔ **About Phone** (or **Version** information).
    *   Tap **Build Number** 7 times until you see the notification: *"You are now a developer!"*
2.  **Enable USB Debugging:**
    *   Go to **Settings** ➔ **System** ➔ **Developer Options** (or search for it in settings).
    *   Turn on **USB Debugging**.
    *   Turn on **Install via USB** (if prompted by your device variant, such as Xiaomi/Realme).
3.  **Connect to PC:**
    *   Plug the phone into your computer via a USB cable.
    *   Set the USB mode to **Transfer Files** or **MTP**.
    *   When the phone displays *"Allow USB Debugging?"*, check the box for **Always allow from this computer** and tap **Allow**.

### 🍏 iOS Setup (Physical Device)
To run on a physical iPhone or iPad:
1.  **Enable Developer Mode (iOS 16+):**
    *   On the device, go to **Settings** ➔ **Privacy & Security** ➔ **Developer Mode** and toggle it **On**.
    *   Restart the device and confirm when prompted.
2.  **Connect & Trust:**
    *   Connect the device to your Mac via USB.
    *   Tap **Trust This Computer** on the device.

---

## 🚀 Running the App

Navigate to the `mobile_flutter` directory inside the repository and follow these steps:

### Step 1: Clean & Fetch Dependencies
Run the following commands to download package dependencies:
```bash
cd mobile_flutter
flutter clean
flutter pub get
```

### Step 2: Generate Platform Configuration (If missing)
If the `android` or `ios` directories are not generated yet, run the standard generator:
```bash
flutter create --org com.mrmsluna.controller .
```

### Step 3: Configure Permissions
Ensure the native app containers have permission to access Bluetooth features.

> [!IMPORTANT]
> Without these permissions, the app will crash or fail to list nearby robots when scanning.

#### **Android Permissions (`android/app/src/main/AndroidManifest.xml`)**
Add the following lines inside the main `<manifest>` tags:
```xml
<uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

#### **iOS Permissions (`ios/Runner/Info.plist`)**
Add the following description keys inside the `<dict>` block:
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app requires Bluetooth access to connect and control the Mr.&Ms Luna toy robot.</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app requires Bluetooth access to control the Mr.&Ms Luna toy robot.</string>
```

### Step 4: Run the Application
Check if Flutter detects your connected device:
```bash
flutter devices
```
If your device is listed (e.g., `RMX3741`), deploy the app directly:

*   **For Development / Debug Mode:**
    ```bash
    flutter run
    ```
*   **For Performance Testing / Release Mode:**
    ```bash
    flutter run --release
    ```

---

## 📦 Building a Standalone APK / IPA

To compile a standalone package that you can share or install without developer tools:

### Android APK Build
```bash
flutter build apk --release
```
The output file will be saved at:
`build/app/outputs/flutter-apk/app-release.apk`

### iOS IPA Build
```bash
flutter build ipa --release
```

---

## 🔍 Troubleshooting Guide

| Issue | Cause | Solution |
| :--- | :--- | :--- |
| **Device not found** | ADB service is not running or device is unauthorized. | Run `adb devices` to check authorization state. Unplug and replug the USB cable, or check your USB cable for file transfer capability. |
| **App crash during BLE Scan** | Missing location or bluetooth permission flags. | Ensure all native permission flags in `AndroidManifest.xml` or `Info.plist` match the configuration listed above. |
| **Gradle build failed** | Java version mismatch. | Make sure your system's Java version is set to JDK 17 (recommended for newer Gradle versions used by Flutter). |
| **No devices detected** | Missing Bluetooth daemon or permissions on macOS. | Ensure Bluetooth is enabled on your host PC and you have granted Xcode full system permissions to manage Bluetooth devices. |
