#include <WiFi.h>
#include <Network.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

#include "config.h"
#include "expressions.h"
#include "audio.h"
#include "bluetooth.h"
#include "interaction.h"

// Forward declaration for network callbacks
void handleRobotCommand(String cmd);

#include "luna_network.h"

// Hardware Interface Objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite display = TFT_eSprite(&tft);
LunaFace face(display);
LunaAudio audio;
volatile int micAmplitude = 0;
LunaBLE ble;
LunaInteraction interaction(TOUCH_PIN);
LunaNetwork network;

// NVS Settings Persistence
Preferences preferences;
bool bleActive = true;
int gifSpeed = 100;
int gifIntroSpeed = 100;
int introSoundSpeed = 100;
int defaultGif = 99;      // default GIF expression (0-6, or 99 for Cycle Mode)
bool isCycleMode = true;
int gifIntro = 1;        // intro GIF expression (0-6)
int touchSingle = 2;     // action for single tap: 0=default, 1=clock, 2=skip_anim, 3=ble_toggle, 10-16=specific expr
int touchDouble = 0;     // action for double tap
int touchLong = 0;       // action for long press
bool negativeDisplay = false; // SSD1306 display color inversion
int clockStyle = 0; // clock style selector (0 to 3)
int oledBrightness = 2; // screen brightness (1: Low, 2: Med, 3: High)
bool inSettingsMenu = false;
int menuOption = 0; // 0: BLE, 1: GIF Speed, 2: Clock Style, 3: Invert, 4: Brightness, 5: Save, 6: Exit
volatile bool hardwareLoopbackActive = false;
bool optionSelected = false;
SmartwatchScreen currentScreen = SCREEN_FACE;

// System State Variables
unsigned int touchCount = 0;
unsigned long lastInteractionTime = 0;
const unsigned long SLEEP_TIMEOUT = 45000; // 45 seconds of inactivity -> sleep
bool isAsleep = false;
bool inIntroPhase = true;
bool isAlarmRinging = false;
unsigned long lastAlarmSoundTime = 0;
bool isReminderRinging = false;
unsigned long lastReminderSoundTime = 0;
int notificationDurationMs = 5000; // default 5 seconds
int reminderDurationMs = 10000; // default 10 seconds
int birthdayDurationMs = 15000; // default 15 seconds
int activeNotificationDurationMs = 5000;
bool mapsActive = false;

// Software Real-Time Clock variables
int rtcHour = 12;
int rtcMinute = 0;
int rtcSecond = 0;
String rtcDay = "Mon";
String rtcDate = "06 Jul";
unsigned long lastRtcMillis = 0;
bool is12HourFormat = false;

void parseAndSyncTime(String timeStr) {
  int firstColon = timeStr.indexOf(':');
  int secondColon = timeStr.lastIndexOf(':');
  if (firstColon > 0 && secondColon > firstColon) {
    rtcHour = timeStr.substring(0, firstColon).toInt();
    rtcMinute = timeStr.substring(firstColon + 1, secondColon).toInt();
    
    int commaIdx = timeStr.indexOf(',', secondColon);
    if (commaIdx > 0) {
      rtcSecond = timeStr.substring(secondColon + 1, commaIdx).toInt();
      int secondCommaIdx = timeStr.indexOf(',', commaIdx + 1);
      if (secondCommaIdx > 0) {
        rtcDay = timeStr.substring(commaIdx + 1, secondCommaIdx);
        rtcDate = timeStr.substring(secondCommaIdx + 1);
      }
    } else {
      rtcSecond = timeStr.substring(secondColon + 1).toInt();
    }
    lastRtcMillis = millis(); // Align RTC base to right now
  }
}

// Periodic expression cycling — all 7 available face expressions
const Expression cycleExpressions[] = {
  EXPR_IDLE,
  EXPR_HAPPY,
  EXPR_SAD,
  EXPR_ANGRY,
  EXPR_SURPRISED,
  EXPR_SLEEPING,
  EXPR_WINK
};
const int numCycleExpressions = 7;
int currentCycleIdx = 0;
unsigned long lastExpressionCycleTime = 0;

// Timing helper for BLE status notifications
unsigned long lastStatusUpdateTime = 0;
const unsigned long STATUS_UPDATE_INTERVAL = 1000; // 1 second

// Global BLE Write Event Handlers (declared extern in bluetooth.h)
void handleBLEExpressionWithLabel(Expression expr, String label) {
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on BLE action
  if (isAsleep && expr != EXPR_SLEEPING) {
    isAsleep = false;
    audio.playSound(SOUND_CHIRP);
  }
  
  if (label.length() > 0) {
    label.toUpperCase();
    // Search ALL_GIFS_TABLE for a match
    int foundIdx = -1;
    for (int i = 0; i < ALL_GIFS_COUNT; i++) {
      char nameBuf[32];
      strcpy_P(nameBuf, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[i].name));
      if (label.equals(nameBuf)) {
        foundIdx = i;
        break;
      }
    }
    if (foundIdx != -1) {
      face.setGifIndex(foundIdx);
      expr = EXPR_ALL_GIF;
    }
  }
  
  face.setExpression(expr);
  if (label.length() > 0) {
    face.setStateLabel(label);
  } else {
    // Fallback to default labels
    switch (expr) {
      case EXPR_IDLE: face.setStateLabel("IDLE"); break;
      case EXPR_HAPPY: face.setStateLabel("HAPPY"); break;
      case EXPR_SAD: face.setStateLabel("SAD"); break;
      case EXPR_ANGRY: face.setStateLabel("ANGRY"); break;
      case EXPR_SURPRISED: face.setStateLabel("SURPRISE"); break;
      case EXPR_SLEEPING: face.setStateLabel("SLEEP"); break;
      case EXPR_WINK: face.setStateLabel("WINK"); break;
      default: face.setStateLabel("IDLE"); break;
    }
  }
  
  // Play appropriate reaction sound effect automatically
  switch (expr) {
    case EXPR_HAPPY:
      audio.playSound(SOUND_POWERUP);
      break;
    case EXPR_SAD:
      audio.playSound(SOUND_POWERDOWN);
      break;
    case EXPR_ANGRY:
      audio.playSound(SOUND_JUMP); // Play angry cartoon jump sound
      break;
    case EXPR_SURPRISED:
      audio.playSound(SOUND_CHIRP);
      break;
    case EXPR_SLEEPING:
      // isAsleep = true; // Sleep mode disabled
      audio.playSound(SOUND_POWERDOWN);
      break;
    default:
      break;
  }
}

void handleBLEExpression(Expression expr) {
  handleBLEExpressionWithLabel(expr, "");
}

void handleBLEAudio(SoundEffect sound) {
  lastInteractionTime = millis();
  audio.playSound(sound);
}

void handleBLEText(String text) {
  handleRobotCommand(text);
}

void handleRobotCommand(String text) {
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on interaction
  
  String ackId = "";
  if (text.startsWith("ACK_ID:")) {
    int sep = text.indexOf('|');
    if (sep > 0) {
      ackId = text.substring(7, sep);
      text = text.substring(sep + 1);
    }
  }

  if (ackId.length() > 0) {
    ble.sendLog("ACK:" + ackId);
  }

  if (text == "WAKE") {
    isAsleep = false;
    face.setExpression(EXPR_IDLE);
    audio.playSound(SOUND_CHIRP);
    Serial.println("Robot woke up from remote command!");
  } else if (text == "SLEEP") {
    // isAsleep = true; // Sleep mode disabled
    face.setExpression(EXPR_SLEEPING);
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("Robot went to sleep from remote command!");
  } else if (text == "RESET") {
    // Factory reset: clear NVS and reboot
    audio.playSound(SOUND_GAMEOVER);
    delay(800);
    preferences.begin("luna", false);
    preferences.clear();
    preferences.end();
    Serial.println("OK:FactoryReset");
    ESP.restart();
  } else if (text.startsWith("EXPR:")) {
    String payload = text.substring(5);
    int comma = payload.indexOf(',');
    if (comma > 0) {
      int exprVal = payload.substring(0, comma).toInt();
      String label = payload.substring(comma + 1);
      handleBLEExpressionWithLabel((Expression)exprVal, label);
    } else {
      int exprVal = payload.toInt();
      handleBLEExpressionWithLabel((Expression)exprVal, "");
    }
    Serial.println("OK:ExprUpdated");
  } else if (text.startsWith("AUDIO:")) {
    int soundVal = text.substring(6).toInt();
    handleBLEAudio((SoundEffect)soundVal);
    Serial.println("OK:AudioPlayed");
  } else if (text.startsWith("TIME:")) {
    parseAndSyncTime(text.substring(5));
    Serial.println("OK:TimeSynced");

  } else if (text.startsWith("12HR:")) {
    is12HourFormat = (text.substring(5).toInt() == 1);
    preferences.begin("luna", false);
    preferences.putBool("is12H", is12HourFormat);
    preferences.end();
    Serial.println("OK:12HourUpdated");
  } else if (text.startsWith("SET:")) {
    applySettings(text.substring(4));
    Serial.println("OK:SettingsSaved");
  } else if (text.startsWith("WIFI:")) {
    // Command format: WIFI:ssid,pass
    String payload = text.substring(5);
    int comma = payload.indexOf(',');
    if (comma > 0) {
      String ssid = payload.substring(0, comma);
      String pass = payload.substring(comma + 1);
      ssid.trim();
      pass.trim();
      
      // Save credentials persistently in NVS Preferences
      preferences.begin("luna", false);
      preferences.putString("wifi_ssid", ssid);
      preferences.putString("wifi_pass", pass);
      preferences.end();
      
      Serial.println("Wi-Fi SSID/Pass saved. Starting network connection...");
      network.startWifi(ssid.c_str(), pass.c_str());
    }
  } else if (text.startsWith("PAIR:")) {
    // Command format: PAIR:mac,relation
    String payload = text.substring(5);
    int comma = payload.indexOf(',');
    if (comma > 0) {
      String companionMac = payload.substring(0, comma);
      String relationType = payload.substring(comma + 1);
      
      preferences.begin("luna", false);
      preferences.putString("comp_mac", companionMac);
      preferences.putString("rel_type", relationType);
      preferences.end();
      
      audio.playSound(SOUND_POWERUP);
      Serial.println("Companion paired: MAC=" + companionMac + ", Relation=" + relationType);
    }
  } else if (text.startsWith("RELATION:")) {
    // Command format: RELATION:type (friends, couple)
    String relationType = text.substring(9);
    preferences.begin("luna", false);
    preferences.putString("rel_type", relationType);
    preferences.end();
    
    // Play romantic sound for couple, friendly chime for friends
    if (relationType == "couple") {
      audio.playSound(SOUND_POWERUP);
    } else {
      audio.playSound(SOUND_COIN);
    }
    Serial.println("Relationship status updated: " + relationType);
  } else if (text.startsWith("CAL:")) {
    // Command format: CAL:type,time,title
    String payload = text.substring(4);
    int firstComma = payload.indexOf(',');
    int secondComma = payload.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma) {
      String type = payload.substring(0, firstComma);
      String time = payload.substring(firstComma + 1, secondComma);
      String title = payload.substring(secondComma + 1);
      
      Serial.println("Calendar Event: type=" + type + ", time=" + time + ", title=" + title);
      
      // Save in calendar events list
      face.addCalendarEvent(type, time, title);
      
      String notificationText = "";
      if (type == "birthday") {
        notificationText = "Birthday: " + title;
        activeNotificationDurationMs = birthdayDurationMs;
      } else {
        notificationText = "Meeting @ " + time + ": " + title;
        activeNotificationDurationMs = reminderDurationMs;
      }
      isReminderRinging = true;
      lastReminderSoundTime = millis();
      face.setNotificationText(notificationText, rtcHour, rtcMinute);
      audio.playSound(SOUND_POWERUP); // play alert sound
    }
  } else if (text.startsWith("ALARM:")) {
    String state = text.substring(6);
    if (state == "START") {
      isAlarmRinging = true;
      face.setExpression(EXPR_CLOCK);
      face.setStateLabel("ALARM!");
      Serial.println("Alarm triggered via BLE/Wi-Fi.");
    } else {
      isAlarmRinging = false;
      face.setExpression(EXPR_IDLE);
      face.setStateLabel("IDLE");
      Serial.println("Alarm stopped/dismissed.");
    }
  } else if (text.startsWith("MAP:")) {
    // Command format: MAP:direction,distance,description OR MAP:EXIT
    String payload = text.substring(4);
    payload.trim();
    if (payload == "EXIT") {
      mapsActive = false;
      face.setExpression(EXPR_IDLE);
      lastExpressionCycleTime = millis() - activeNotificationDurationMs;
      Serial.println("Maps Navigation Exited.");
      return;
    }
    int firstComma = payload.indexOf(',');
    String direction = "";
    String distance = "";
    String description = "";

    if (firstComma < 0) {
      direction = payload;
    } else {
      direction = payload.substring(0, firstComma);
      String rest = payload.substring(firstComma + 1);
      int secondComma = rest.indexOf(',');
      if (secondComma < 0) {
        distance = rest;
      } else {
        distance = rest.substring(0, secondComma);
        description = rest.substring(secondComma + 1);
      }
    }
    direction.trim();
    distance.trim();
    description.trim();
    // IMPORTANT: toUpperCase() modifies in-place on Arduino but we must reassign
    direction.toUpperCase(); // modifies in-place
    String dirUpper = direction; // ensure we use the modified value
    
    Serial.println("[MAP] direction='" + dirUpper + "' distance='" + distance + "' desc='" + description + "'");
    
    bool shouldBeep = (!mapsActive) || (dirUpper != face.getMapDirection());
    
    mapsActive = true;
    face.setMapNavigation(dirUpper, distance, description);
    
    if (shouldBeep) {
      audio.playSound(SOUND_CHIRP);
    }
    
    activeNotificationDurationMs = 20000; // 20 seconds visibility for turn navigation
  } else if (text == "CALL:START") {
    audio.micStreaming = true;
    audio.audioMode = LunaAudio::AUDIO_MODE_STREAM;
    face.setStateLabel("CALL");
    Serial.println("VoIP Call started");
  } else if (text == "CALL:STOP") {
    audio.micStreaming = false;
    audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
    face.setStateLabel("IDLE");
    Serial.println("VoIP Call stopped");
  } else if (text == "LOOPBACK:START") {
    audio.directLoopback = true;
    audio.micStreaming = false;     // BLE streaming off; direct i2s_write handles output
    audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH; // Keep TX task silent
    hardwareLoopbackActive = true;
    face.setStateLabel("TEST");
    audio.playSound(SOUND_POWERUP);
    Serial.println("Loopback test started (direct i2s path)");
  } else if (text == "LOOPBACK:STOP") {
    audio.directLoopback = false;
    audio.micStreaming = false;
    audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
    hardwareLoopbackActive = false;
    face.setStateLabel("IDLE");
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("Loopback test stopped");
  } else if (text == "MUSIC:START") {
    audio.micStreaming = false;
    audio.startMusicStream();
    face.setStateLabel("MUSIC");
    Serial.println("Music mode started");
  } else if (text == "MUSIC:STOP") {
    audio.stopMusicStream();
    face.setStateLabel("IDLE");
    Serial.println("Music mode stopped");
  } else if (text.startsWith("VOL:")) {
    int vol = text.substring(4).toInt();
    vol = constrain(vol, 0, 100);
    audio.setVolume(vol);
    Serial.print("Volume set to: ");
    Serial.println(vol);
  } else if (text.startsWith("BASS:")) {
    int bass = text.substring(5).toInt();
    bass = constrain(bass, 0, 10);
    audio.setBassBoost(bass);
    Serial.print("Bass boost set to: ");
    Serial.println(bass);
  } else if (text.startsWith("AUDIO_MODE:")) {
    String mode = text.substring(11);
    if (mode == "STREAM") {
      audio.audioMode = LunaAudio::AUDIO_MODE_STREAM;
    } else {
      audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
    }
    Serial.println("Audio mode set to: " + mode);
  } else if (text.startsWith("NOTIF:")) {
    // Command format: NOTIF:Title|Body
    String payload = text.substring(6);
    int sep = payload.indexOf('|');
    if (sep > 0) {
      String notifTitle = payload.substring(0, sep);
      String notifBody = payload.substring(sep + 1);
      notifTitle.trim();
      notifBody.trim();
      face.setDetailedNotification(notifTitle, notifBody, rtcHour, rtcMinute);
    } else {
      face.setNotificationText(payload, rtcHour, rtcMinute);
    }
    audio.playSound(SOUND_CHIRP);
    activeNotificationDurationMs = notificationDurationMs;
  } else {
    // Normal text message notification
    face.setNotificationText(text, rtcHour, rtcMinute);
    audio.playSound(SOUND_CHIRP); // alert user
    activeNotificationDurationMs = notificationDurationMs;
  }
}

void applySettings(String payload) {
  // Robust CSV parsing — split by commas into an array
  // Expected format: ble,speed,defaultGif,gifIntro,touchSingle,touchDouble,touchLong,negative,introSpeed,introSoundSpeed,notifDur,remDur,birthDur,clkStyle,oledBright
  String parts[15];
  int partCount = 0;
  int startIdx = 0;
  for (int i = 0; i <= payload.length() && partCount < 15; i++) {
    if (i == (int)payload.length() || payload[i] == ',') {
      String part = payload.substring(startIdx, i);
      part.trim();
      parts[partCount++] = part;
      startIdx = i + 1;
    }
  }

  if (partCount < 2) return; // Need at least ble,speed

  bleActive   = true; // Always ON
  gifSpeed    = parts[1].toInt();
  if (gifSpeed < 20)  gifSpeed = 20;
  if (gifSpeed > 500) gifSpeed = 500;

  if (partCount > 2) defaultGif  = parts[2].toInt();
  if (partCount > 3) gifIntro    = parts[3].toInt();
  if (partCount > 4) touchSingle = parts[4].toInt();
  if (partCount > 5) touchDouble = parts[5].toInt();
  if (partCount > 6) touchLong   = parts[6].toInt();
  if (partCount > 7) negativeDisplay = (parts[7].toInt() == 1);
  if (partCount > 8) {
    gifIntroSpeed = parts[8].toInt();
    if (gifIntroSpeed < 20)  gifIntroSpeed = 20;
    if (gifIntroSpeed > 500) gifIntroSpeed = 500;
  }
  if (partCount > 9) {
    introSoundSpeed = parts[9].toInt();
    if (introSoundSpeed < 20)  introSoundSpeed = 20;
    if (introSoundSpeed > 500) introSoundSpeed = 500;
  }
  if (partCount > 10) {
    notificationDurationMs = parts[10].toInt() * 1000;
    if (notificationDurationMs < 1000) notificationDurationMs = 1000;
  }
  if (partCount > 11) {
    reminderDurationMs = parts[11].toInt() * 1000;
    if (reminderDurationMs < 1000) reminderDurationMs = 1000;
  }
  if (partCount > 12) {
    birthdayDurationMs = parts[12].toInt() * 1000;
    if (birthdayDurationMs < 1000) birthdayDurationMs = 1000;
  }
  if (partCount > 13) {
    clockStyle = parts[13].toInt();
  }
  if (partCount > 14) {
    oledBrightness = parts[14].toInt();
  }

  // Apply settings immediately
  face.setFrameDelay(gifSpeed);

  // defaultGif: 99 = Cycle Mode
  //             0-8 = base Expression enum
  //             values >= 100 = specific GIF index (index = value - 100) in ALL_GIFS_TABLE
  if (defaultGif == 99) {
    isCycleMode = true;
    cycleExpression();
  } else {
    isCycleMode = false;
    if (defaultGif >= 100) {
      int gifIdx = defaultGif - 100;
      if (gifIdx >= 0 && gifIdx < ALL_GIFS_COUNT) {
        face.setGifIndex(gifIdx);
        face.setExpression(EXPR_ALL_GIF);
      }
    } else {
      face.setDefaultExpression((Expression)defaultGif);
      face.setExpression((Expression)defaultGif);
    }
  }

  ble.setBLEActive(bleActive);
  tft.invertDisplay(negativeDisplay);
  
  #ifdef TFT_BL
  analogWriteFrequency(TFT_BL, 24000); // 24 kHz high-frequency PWM
  if (oledBrightness == 1) analogWrite(TFT_BL, 30);
  else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
  else analogWrite(TFT_BL, 255);
  #endif
  
  Serial.print("NegativeDisplay set to: ");
  Serial.println(negativeDisplay ? "ON" : "OFF");
  Serial.print("ClockStyle set to: ");
  Serial.println(clockStyle);
  Serial.print("OledBrightness set to: ");
  Serial.println(oledBrightness);

  // Save all settings to NVS flash
  preferences.begin("luna", false);
  preferences.putBool("ble", bleActive);
  preferences.putInt("speed", gifSpeed);
  preferences.putInt("defGif", defaultGif);
  preferences.putInt("intGif", gifIntro);
  preferences.putInt("tchSing", touchSingle);
  preferences.putInt("tchDoub", touchDouble);
  preferences.putInt("tchLong", touchLong);
  preferences.putBool("neg", negativeDisplay);
  preferences.putInt("clkStyle", clockStyle);
  preferences.putInt("oledBright", oledBrightness);
  preferences.putInt("intSpeed", gifIntroSpeed);
  preferences.putInt("sndSpeed", introSoundSpeed);
  preferences.putInt("notifDur", notificationDurationMs);
  preferences.putInt("remDur", reminderDurationMs);
  preferences.putInt("birthDur", birthdayDurationMs);
  preferences.end();

  audio.playSound(SOUND_POWERUP);
}

void setup() {
  Serial.begin(115200);
  delay(100); // Faster boot!
  
  // Initialize device pins safely after core initialization
  audio.begin();
  interaction.begin();

  // Initialize push buttons with pull-ups
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  Serial.print(negativeDisplay ? "Ms. Luna Robot Booting Up... Version: " : "Mr. Luna Robot Booting Up... Version: ");
  Serial.println(FIRMWARE_VERSION);  // Load persistence settings from NVS Preferences
  preferences.begin("luna", false);
  bleActive = true; // Always ON
  gifSpeed = preferences.getInt("speed", 100);
  defaultGif = preferences.getInt("defGif", 99);
  gifIntro = preferences.getInt("intGif", 1);
  touchSingle = preferences.getInt("tchSing", 2);  // default: skip animation
  touchDouble = preferences.getInt("tchDoub", 0);
  touchLong = preferences.getInt("tchLong", 0);
  negativeDisplay = preferences.getBool("neg", false);
  clockStyle = preferences.getInt("clkStyle", 0);
  oledBrightness = preferences.getInt("oledBright", 2);
  gifIntroSpeed = preferences.getInt("intSpeed", 100);
  introSoundSpeed = preferences.getInt("sndSpeed", 100);
  is12HourFormat = preferences.getBool("is12H", false);
  notificationDurationMs = preferences.getInt("notifDur", 5000);
  reminderDurationMs = preferences.getInt("remDur", 10000);
  birthdayDurationMs = preferences.getInt("birthDur", 15000);
  activeNotificationDurationMs = notificationDurationMs;
  preferences.end();

  // Set the GIF speed delay and default expression
  face.setFrameDelay(gifSpeed);
  if (defaultGif == 99) {
    isCycleMode = true;
  } else {
    isCycleMode = false;
    if (defaultGif >= 100) {
      int gifIdx = defaultGif - 100;
      if (gifIdx >= 0 && gifIdx < ALL_GIFS_COUNT) {
        face.setGifIndex(gifIdx);
        face.setExpression(EXPR_ALL_GIF);
      }
    } else {
      face.setDefaultExpression((Expression)defaultGif);
      face.setExpression((Expression)defaultGif);
    }
  }

  // Initialize I2C Communication
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(800000); // 800kHz high-speed I2C for compatibility
  
  // Initialize TFT_eSPI Display and Sprite Buffer
  tft.init();
  tft.setRotation(0); // Normal rotation
  tft.fillScreen(TFT_BLACK);
  
  display.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  display.fillSprite(TFT_BLACK);
  
  // Make bootloading logo alone always invert
  tft.invertDisplay(!negativeDisplay);

  // Apply saved brightness setting
  #ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  analogWriteFrequency(TFT_BL, 24000); // 24 kHz high-frequency PWM
  if (oledBrightness == 1) analogWrite(TFT_BL, 30);
  else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
  else analogWrite(TFT_BL, 255);
  #endif

  // Display initial loading face / logo
  display.fillSprite(TFT_BLACK);
  display.pushImage((SCREEN_WIDTH - LOGO_WIDTH)/2, (SCREEN_HEIGHT - LOGO_HEIGHT)/2, LOGO_WIDTH, LOGO_HEIGHT, image_logo_pixels);
  display.pushSprite(0, 0);
  
  // Start Bluetooth BLE Server (always on)
  bleActive = true;
  ble.init();
  Serial.println(negativeDisplay ? "BLE Server Started as 'Ms. Luna Robot'." : "BLE Server Started as 'Mr. Luna Robot'.");

  // Play startup sound immediately so it plays while loading the logo
  audio.playSound(SOUND_STARTUP, introSoundSpeed);


  // Show logo for 3 seconds while playing the startup sound and ignoring/clearing touches
  unsigned long bootStart = millis();
  while (millis() - bootStart < 3000) {
    audio.update();
    interaction.update(); // read to clear/ignore early boot noise
    delay(1);
  }

  // Restore saved invert setting for standard operation
  tft.invertDisplay(negativeDisplay);
  
  // Set intro speed
  face.setFrameDelay(gifIntroSpeed);
  inIntroPhase = true;

  if (gifIntro >= 100) {
    int gifIdx = gifIntro - 100;
    if (gifIdx >= 0 && gifIdx < ALL_GIFS_COUNT) {
      face.setGifIndex(gifIdx);
      face.setExpression(EXPR_ALL_GIF);
    }
  } else {
    face.setExpression((Expression)gifIntro);
  }

  lastInteractionTime = millis();
  lastRtcMillis = millis();
  lastExpressionCycleTime = millis();

  // Dump all loaded GIF names to serial for debugging
  Serial.println("====== GIF TABLE DUMP ======");
  Serial.print("Total GIFs loaded: ");
  Serial.println(ALL_GIFS_COUNT);
  for (int i = 0; i < ALL_GIFS_COUNT; i++) {
    char gifName[32];
    strcpy_P(gifName, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[i].name));
    uint32_t cnt = (uint32_t)pgm_read_dword(&ALL_GIFS_TABLE[i].count);
    Serial.print("GIF[");
    Serial.print(i);
    Serial.print("] ");
    Serial.print(gifName);
    Serial.print(" (");
    Serial.print(cnt);
    Serial.println(" frames)");
  }
  Serial.println("=============================");
  network.init();
}

// Global index for all-gifs cycling — advances through all 63 entries
int allGifCycleIdx = 0;

void cycleExpression() {
  allGifCycleIdx = random(0, ALL_GIFS_COUNT);
  face.setGifIndex(allGifCycleIdx);
  face.setExpression(EXPR_ALL_GIF);
  // Read GIF name from PROGMEM — update stateLabel so BLE status mirrors hardware
  char gifName[32];
  strcpy_P(gifName, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[allGifCycleIdx].name));
  face.setStateLabel(String(gifName));   // <-- critical: keeps app simulator in sync
  uint32_t cnt = (uint32_t)pgm_read_dword(&ALL_GIFS_TABLE[allGifCycleIdx].count);
  Serial.print("PLAYING RANDOM:GIF[");
  Serial.print(allGifCycleIdx);
  Serial.print("/");
  Serial.print(ALL_GIFS_COUNT - 1);
  Serial.print("] ");
  Serial.print(gifName);
  Serial.print(" (");
  Serial.print(cnt);
  Serial.println(" frames)");
}

void executeTouchAction(int actionType, TouchEvent eventType) {
  if (actionType == 0) {
    // Default reaction
    if (eventType == TOUCH_TAP) {
      if (random(0, 2) == 0) {
        face.setExpression(EXPR_WINK);
        audio.playSound(SOUND_CHIRP);
      } else {
        face.setExpression(EXPR_SURPRISED);
        audio.playSound(SOUND_JUMP);
      }
    } else if (eventType == TOUCH_DOUBLE_TAP) {
      face.setExpression(EXPR_HAPPY);
      audio.playSound(SOUND_COIN);
    } else if (eventType == TOUCH_LONG_PRESS) {
      // isAsleep = true; // Sleep mode disabled
      face.setExpression(EXPR_SLEEPING);
      audio.playSound(SOUND_POWERDOWN);
      Serial.println(negativeDisplay ? "Ms. Luna entered Sleep Mode (animation only, stays awake)!" : "Mr. Luna entered Sleep Mode (animation only, stays awake)!");
    }
  } else {
    // Custom actions
    if (actionType == 1) {
      face.setExpression(EXPR_CLOCK);
      audio.playSound(SOUND_CHIRP);
      Serial.println("Triggered Full Screen Clock");
    } else if (actionType == 2) {
      cycleExpression();
      audio.playSound(SOUND_COIN);
      Serial.println("Skipped to next animation");
    } else if (actionType == 3) {
      // BLE is always ON, do not toggle
      bleActive = true;
      audio.playSound(SOUND_CHIRP);
      Serial.println("BLE Toggle touch action ignored (BLE is always ON)");
    } else if (actionType >= 20) {
      // Specific GIF index: actionType = 20 + gifIndex
      int gifIdx = actionType - 20;
      if (gifIdx >= 0 && gifIdx < ALL_GIFS_COUNT) {
        face.setGifIndex(gifIdx);
        face.setExpression(EXPR_ALL_GIF);
        allGifCycleIdx = gifIdx; // keep cycle state in sync
        audio.playSound(SOUND_COIN);
        char gifName[32];
        strcpy_P(gifName, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[gifIdx].name));
        Serial.print("Touch triggered specific GIF #");
        Serial.print(gifIdx);
        Serial.print(": ");
        Serial.println(gifName);
      }
    } else if (actionType >= 10 && actionType <= 16) {
      // Legacy base expression (0-6): actionType = 10 + exprId
      Expression target = (Expression)(actionType - 10);
      face.setExpression(target);
      switch (target) {
        case EXPR_HAPPY: audio.playSound(SOUND_POWERUP); break;
        case EXPR_SAD: audio.playSound(SOUND_POWERDOWN); break;
        case EXPR_ANGRY: audio.playSound(SOUND_GAMEOVER); break;
        case EXPR_SURPRISED: audio.playSound(SOUND_JUMP); break;
        case EXPR_SLEEPING: audio.playSound(SOUND_POWERDOWN); break;
        case EXPR_WINK: audio.playSound(SOUND_CHIRP); break;
        default: audio.playSound(SOUND_CHIRP); break;
      }
      Serial.print("Triggered base expression: ");
      Serial.println(actionType - 10);
    }
  }
}
void handleButton1Press() {
  unsigned long now = millis();
  lastInteractionTime = now; // reset inactivity clock
  
  if (currentScreen != SCREEN_SETTINGS) {
    // Go directly to settings screen
    currentScreen = SCREEN_SETTINGS;
    menuOption = 0;
    optionSelected = false;
    audio.playSound(SOUND_POWERUP);
    Serial.println("Button 1: Switched to settings screen.");
  } else {
    // We are on settings screen
    if (!optionSelected) {
      // Navigate options (0 to 7)
      menuOption = (menuOption + 1) % 8;
      audio.playSound(SOUND_CHIRP);
      Serial.printf("Button 1: Navigated to menu option %d\n", menuOption);
    } else {
      // Option is selected: cycle values!
      if (menuOption == 0) { // BLE on/off toggle (BLE always on)
        bleActive = true;
        audio.playSound(SOUND_CHIRP);
      } else if (menuOption == 1) { // GIF speed control
        gifSpeed += 20;
        if (gifSpeed > 300) {
          gifSpeed = 20;
        }
        face.setFrameDelay(gifSpeed);
        audio.playSound(SOUND_CHIRP);
      } else if (menuOption == 2) { // Clock Style
        clockStyle = (clockStyle + 1) % 5;
        audio.playSound(SOUND_CHIRP);
      } else if (menuOption == 3) { // Invert/Negative display
        negativeDisplay = !negativeDisplay;
        tft.invertDisplay(negativeDisplay);
        audio.playSound(SOUND_CHIRP);
      } else if (menuOption == 4) { // Brightness
        oledBrightness = (oledBrightness % 3) + 1;
        #ifdef TFT_BL
        if (oledBrightness == 1) analogWrite(TFT_BL, 30);
        else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
        else analogWrite(TFT_BL, 255);
        #endif
        audio.playSound(SOUND_CHIRP);
      } else if (menuOption == 5) { // Loopback Test
        hardwareLoopbackActive = !hardwareLoopbackActive;
        audio.directLoopback = hardwareLoopbackActive;
        audio.micStreaming = false;
        audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
        if (hardwareLoopbackActive) {
          face.setStateLabel("TEST");
          audio.playSound(SOUND_POWERUP);
        } else {
          face.setStateLabel("IDLE");
          audio.playSound(SOUND_POWERDOWN);
        }
      }
      Serial.printf("Button 1: Cycled menu option %d value\n", menuOption);
    }
  }
}

void handleButton2Press() {
  unsigned long now = millis();
  lastInteractionTime = now; // reset inactivity clock
  
  if (currentScreen == SCREEN_SETTINGS) {
    if (!optionSelected) {
      if (menuOption == 6) { // SAVE
        preferences.begin("luna", false);
        preferences.putBool("ble", bleActive);
        preferences.putInt("speed", gifSpeed);
        preferences.putInt("defGif", defaultGif);
        preferences.putInt("intGif", gifIntro);
        preferences.putInt("tchSing", touchSingle);
        preferences.putInt("tchDoub", touchDouble);
        preferences.putInt("tchLong", touchLong);
        preferences.putBool("neg", negativeDisplay);
        preferences.putInt("clkStyle", clockStyle);
        preferences.putInt("oledBright", oledBrightness);
        preferences.end();
        
        #ifdef TFT_BL
        if (oledBrightness == 1) analogWrite(TFT_BL, 30);
        else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
        else analogWrite(TFT_BL, 255);
        #endif
        
        audio.playSound(SOUND_POWERUP);
        optionSelected = false; // deselect
        Serial.println("Button 2: Settings saved.");
      } else if (menuOption == 7) { // EXIT
        hardwareLoopbackActive = false;
        audio.micStreaming = false;
        audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
        audio.prebuffering = true;
        face.setStateLabel("IDLE");
        currentScreen = SCREEN_FACE;
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("Button 2: Exited settings.");
      } else {
        // Select option (0 to 5)
        optionSelected = true;
        audio.playSound(SOUND_COIN);
        Serial.printf("Button 2: Selected option %d\n", menuOption);
      }
    } else {
      // Confirms/deselects option
      optionSelected = false;
      audio.playSound(SOUND_COIN);
      Serial.printf("Button 2: Confirmed option %d\n", menuOption);
    }
  }
}

void loop() {
  unsigned long now = millis();

  // 0. Debounced Push Button Reads
  static bool lastBtn1State = HIGH;
  static bool lastBtn2State = HIGH;
  static unsigned long lastBtn1DebounceTime = 0;
  static unsigned long lastBtn2DebounceTime = 0;
  const unsigned long DEBOUNCE_DELAY = 50;

  bool currentBtn1State = digitalRead(BUTTON1_PIN);
  bool currentBtn2State = digitalRead(BUTTON2_PIN);

  if (currentBtn1State != lastBtn1State) {
    lastBtn1DebounceTime = now;
  }
  if ((now - lastBtn1DebounceTime) > DEBOUNCE_DELAY) {
    static bool btn1Processed = false;
    if (currentBtn1State == LOW) {
      if (!btn1Processed) {
        handleButton1Press();
        btn1Processed = true;
      }
    } else {
      btn1Processed = false;
    }
  }
  lastBtn1State = currentBtn1State;

  if (currentBtn2State != lastBtn2State) {
    lastBtn2DebounceTime = now;
  }
  if ((now - lastBtn2DebounceTime) > DEBOUNCE_DELAY) {
    static bool btn2Processed = false;
    if (currentBtn2State == LOW) {
      if (!btn2Processed) {
        handleButton2Press();
        btn2Processed = true;
      }
    } else {
      btn2Processed = false;
    }
  }
  lastBtn2State = currentBtn2State;

  // 1. Maintain BLE stack status and connection advertisement
  ble.handleConnectionState();

  // 2. Refresh non-blocking audio synthesizer
  audio.update();

  // 2.1. If BLE mic streaming is active, route mic audio to BLE (loopback uses direct i2s path, not this)
  if (audio.micStreaming && !audio.directLoopback) {
    uint8_t micBuf[256];
    size_t micSize = 0;
    while (audio.getRxItem(micBuf, &micSize)) {
      if (micSize > 0) {
        ble.sendAudioStream(micBuf, micSize);
      }
    }
  }

  // 2.7. Poll companion network stack
  network.update();

  // 2.5. Process incoming USB Serial settings commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "RESET") {
      audio.playSound(SOUND_GAMEOVER);
      delay(800);
      preferences.begin("luna", false);
      preferences.clear();
      preferences.end();
      Serial.println("OK:FactoryReset");
      ESP.restart();
    } else if (cmd.startsWith("SET:")) {
      applySettings(cmd.substring(4));
      Serial.println("OK:SettingsSaved");
    } else if (cmd.startsWith("TIME:")) {
      parseAndSyncTime(cmd.substring(5));
      Serial.println("OK:TimeSynced");

    } else if (cmd == "GET") {
      Serial.println("SETTINGS:" + String(bleActive ? "1" : "0") + "," + String(gifSpeed) + "," + String(defaultGif) + "," + String(gifIntro) + "," + String(touchSingle) + "," + String(touchDouble) + "," + String(touchLong) + "," + String(negativeDisplay ? "1" : "0"));
    } else if (cmd == "LIST") {
      // Re-print all GIF entries for debugging
      Serial.println("====== GIF TABLE DUMP ======");
      Serial.print("Total GIFs loaded: ");
      Serial.println(ALL_GIFS_COUNT);
      for (int i = 0; i < ALL_GIFS_COUNT; i++) {
        char gifName[32];
        strcpy_P(gifName, (char*)pgm_read_ptr(&ALL_GIFS_TABLE[i].name));
        uint32_t cnt = (uint32_t)pgm_read_dword(&ALL_GIFS_TABLE[i].count);
        Serial.print("GIF[");
        Serial.print(i);
        Serial.print("] ");
        Serial.print(gifName);
        Serial.print(" (");
        Serial.print(cnt);
        Serial.println(" frames)");
      }
      Serial.println("=============================");
    } else {
      // Fallback: forward generic commands (e.g. MAP:, NOTIF:, EXPR:, AUDIO:) to the robot command handler
      handleRobotCommand(cmd);
    }
  }

  TouchEvent touchEvent = interaction.update();
  if (touchEvent != TOUCH_NONE) {
    lastInteractionTime = now; // reset inactivity clock
    lastExpressionCycleTime = now; // reset expression cycle timer

    if (face.isPopupActive()) {
      face.setPopupDismiss();
      audio.playSound(SOUND_CHIRP);
      Serial.println("Notification popup dismissed by touch.");
    } else if (isAlarmRinging || isReminderRinging) {
      isAlarmRinging = false;
      isReminderRinging = false;
      audio.playSound(SOUND_COIN); // play coin sound to confirm dismissal
      if (isCycleMode) {
        cycleExpression();
      } else {
        if (defaultGif >= 100) {
          face.setGifIndex(defaultGif - 100);
          face.setExpression(EXPR_ALL_GIF);
        } else {
          face.setExpression((Expression)defaultGif);
        }
      }
      face.setStateLabel("IDLE");
      ble.sendLog("ALARM:DISMISS");
      Serial.println("Alarm/Reminder dismissed by hardware touch button.");
    } else if (isAsleep && currentScreen != SCREEN_SETTINGS) {
      // Any touch wakes the robot up
      isAsleep = false;
      currentScreen = SCREEN_FACE;
      face.setExpression(EXPR_IDLE);
      audio.playSound(SOUND_CHIRP);
      Serial.println(negativeDisplay ? "Ms. Luna Woke Up!" : "Mr. Luna Woke Up!");
    } else {
      if (currentScreen == SCREEN_SETTINGS) {
        // Settings Menu Touch Logic
        if (optionSelected) {
          // Adjusting an option value
          if (touchEvent == TOUCH_TAP) {
            if (menuOption == 0) { // BLE on/off toggle
              // BLE is always ON, do not toggle
              bleActive = true;
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 1) { // GIF speed control
              gifSpeed += 20;
              if (gifSpeed > 300) {
                gifSpeed = 20;
              }
              face.setFrameDelay(gifSpeed);
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 2) { // Clock Style
              clockStyle = (clockStyle + 1) % 5;
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 3) { // Invert/Negative display
              negativeDisplay = !negativeDisplay;
              tft.invertDisplay(negativeDisplay);
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 4) { // Brightness
              oledBrightness = (oledBrightness % 3) + 1;
              #ifdef TFT_BL
              if (oledBrightness == 1) analogWrite(TFT_BL, 30);
              else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
              else analogWrite(TFT_BL, 255);
              #endif
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 5) { // Loopback Test
              hardwareLoopbackActive = !hardwareLoopbackActive;
              audio.directLoopback = hardwareLoopbackActive;
              audio.micStreaming = false;  // BLE streaming off during loopback
              audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH; // TX task stays silent
              if (hardwareLoopbackActive) {
                face.setStateLabel("TEST");
                audio.playSound(SOUND_POWERUP);
              } else {
                face.setStateLabel("IDLE");
                audio.playSound(SOUND_POWERDOWN);
              }
            }
          } else if (touchEvent == TOUCH_LONG_PRESS) {
            // Long press deselecting option
            optionSelected = false;
            audio.playSound(SOUND_COIN);
          }
        } else {
          // Navigating the menu options
          if (touchEvent == TOUCH_TAP) {
            // Single tap cycles options
            menuOption = (menuOption + 1) % 8;
            audio.playSound(SOUND_CHIRP);
          } else if (touchEvent == TOUCH_LONG_PRESS) {
            // Long press selects options
            if (menuOption == 6) { // SAVE
              preferences.begin("luna", false);
              preferences.putBool("ble", bleActive);
              preferences.putInt("speed", gifSpeed);
              preferences.putInt("defGif", defaultGif);
              preferences.putInt("intGif", gifIntro);
              preferences.putInt("tchSing", touchSingle);
              preferences.putInt("tchDoub", touchDouble);
              preferences.putInt("tchLong", touchLong);
              preferences.putBool("neg", negativeDisplay);
              preferences.putInt("clkStyle", clockStyle);
              preferences.putInt("oledBright", oledBrightness);
              preferences.end();
              
              // Apply settings immediately
              #ifdef TFT_BL
              if (oledBrightness == 1) analogWrite(TFT_BL, 30);
              else if (oledBrightness == 2) analogWrite(TFT_BL, 128);
              else analogWrite(TFT_BL, 255);
              #endif
              
              audio.playSound(SOUND_POWERUP);
              optionSelected = false; // deselect
            } else if (menuOption == 7) { // EXIT
              hardwareLoopbackActive = false;
              audio.micStreaming = false;
              audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
              audio.prebuffering = true;
              face.setStateLabel("IDLE");
              currentScreen = SCREEN_FACE;
              audio.playSound(SOUND_POWERDOWN);
            } else {
              // Option select (0 to 5)
              optionSelected = true;
              audio.playSound(SOUND_COIN);
            }
          } else if (touchEvent == TOUCH_DOUBLE_TAP) {
            // Double tap cycles screen within UI modes
            if (currentScreen >= SCREEN_CLOCK && currentScreen <= SCREEN_SETTINGS) {
              currentScreen = (SmartwatchScreen)((currentScreen + 1) % 4);
              if (currentScreen == SCREEN_SETTINGS) {
                menuOption = 0;
                optionSelected = false;
              } else {
                hardwareLoopbackActive = false;
                audio.micStreaming = false;
                audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
                audio.prebuffering = true;
                face.setStateLabel("IDLE");
              }
            }
            audio.playSound(SOUND_COIN);
            Serial.print("Switched screen to: ");
            Serial.println(currentScreen);
          } else if (touchEvent == TOUCH_TRIPLE_TAP) {
            currentScreen = SCREEN_FACE;
            audio.playSound(SOUND_STARTUP);
          }
        }
      } else {
        // Normal state controls (Smartwatch OS style navigation)
        switch (touchEvent) {
          case TOUCH_TAP:
            if (currentScreen == SCREEN_CLOCK) {
              clockStyle = (clockStyle + 1) % 5; // Cycle clock style
              audio.playSound(SOUND_CHIRP);
            } else if (currentScreen == SCREEN_NOTIFICATIONS) {
              face.cycleNotificationView();
              audio.playSound(SOUND_CHIRP);
            } else if (currentScreen == SCREEN_CALENDAR) {
              face.cycleCalendarView();
              audio.playSound(SOUND_CHIRP);
            } else if (currentScreen == SCREEN_FACE) {
              cycleExpression();
              audio.playSound(SOUND_CHIRP);
            } else {
              touchCount++;
              executeTouchAction(touchSingle, TOUCH_TAP);
            }
            break;

          case TOUCH_DOUBLE_TAP:
            if (currentScreen == SCREEN_FACE) {
              currentScreen = SCREEN_CLOCK;
              audio.playSound(SOUND_POWERUP);
              Serial.println("Double tap: Switched to Clock screen.");
            } else if (currentScreen >= SCREEN_CLOCK && currentScreen <= SCREEN_SETTINGS) {
              currentScreen = (SmartwatchScreen)((currentScreen + 1) % 4);
              if (currentScreen == SCREEN_SETTINGS) {
                menuOption = 0;
                optionSelected = false;
              }
              audio.playSound(SOUND_COIN);
              Serial.print("Switched screen to: ");
              Serial.println(currentScreen);
            }
            break;

          case TOUCH_TRIPLE_TAP:
            if (currentScreen == SCREEN_FACE) {
              currentScreen = SCREEN_CLOCK;
              audio.playSound(SOUND_POWERUP);
              Serial.println("Entered smartwatch UI mode.");
            } else {
              hardwareLoopbackActive = false;
              audio.micStreaming = false;
              audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
              audio.prebuffering = true;
              face.setStateLabel("IDLE");
              currentScreen = SCREEN_FACE;
              audio.playSound(SOUND_STARTUP);
              Serial.println("Exited UI, returned to Mochi expressions.");
            }
            break;

          case TOUCH_LONG_PRESS:
            if (currentScreen == SCREEN_CLOCK) {
              // BLE is always ON, do not toggle
              audio.playSound(SOUND_CHIRP);
            } else if (currentScreen == SCREEN_NOTIFICATIONS) {
              face.clearNotifications();
              audio.playSound(SOUND_GAMEOVER);
            } else if (currentScreen == SCREEN_CALENDAR) {
              face.toggleCalendarMode();
              audio.playSound(SOUND_COIN);
            } else if (currentScreen == SCREEN_FACE) {
              isAsleep = !isAsleep;
              face.setExpression(isAsleep ? EXPR_SLEEPING : EXPR_IDLE);
              audio.playSound(isAsleep ? SOUND_POWERDOWN : SOUND_CHIRP);
            } else {
              executeTouchAction(touchLong, TOUCH_LONG_PRESS);
            }
            break;

          default:
            break;
        }
      }
    }
  }

  // 3.5. Update Software Real-Time Clock (drift-free)
  if (now - lastRtcMillis >= 1000) {
    lastRtcMillis = now; // Use actual now to prevent drift accumulation
    rtcSecond++;
    if (rtcSecond >= 60) {
      rtcSecond = 0;
      rtcMinute++;
      if (rtcMinute >= 60) {
        rtcMinute = 0;
        rtcHour++;
        if (rtcHour >= 24) {
          rtcHour = 0;
        }
      }
    }
  }

  // 3.6. Expression cycling and transitions
  if (inIntroPhase) {
    currentScreen = SCREEN_FACE; // Force face screen on boot for intro animation
    if (face.isGifFinished()) {
      face.clearGifFinished();
      inIntroPhase = false;
      face.setFrameDelay(gifSpeed);
      currentScreen = SCREEN_FACE; // Switch to face after boot!
      if (isCycleMode) {
        cycleExpression();
      } else {
        if (defaultGif >= 100) {
          face.setGifIndex(defaultGif - 100);
          face.setExpression(EXPR_ALL_GIF);
        } else {
          face.setExpression((Expression)defaultGif);
        }
      }
      lastExpressionCycleTime = now;
    }
  } else {
    // Regular operation expression cycling (only when on SCREEN_FACE screen)
    if (currentScreen == SCREEN_FACE && !isAsleep) {
      if (face.getExpression() == EXPR_ALL_GIF) {
        if (face.isGifFinished()) {
          face.clearGifFinished();
          if (isCycleMode) {
            // Only switch to a different random GIF if at least 8 seconds has elapsed since last cycle!
            if (now - lastExpressionCycleTime >= 8000) {
              cycleExpression();
              lastExpressionCycleTime = now;
            }
          }
        }
      } else {
        // Return to random emoji cycling/default expression after notification duration
        if (!isReminderRinging && (now - lastExpressionCycleTime >= (unsigned long)activeNotificationDurationMs)) {
          if (isCycleMode) {
            cycleExpression();
          } else {
            if (defaultGif >= 100) {
              face.setGifIndex(defaultGif - 100);
              face.setExpression(EXPR_ALL_GIF);
            } else {
              face.setExpression((Expression)defaultGif);
            }
          }
          lastExpressionCycleTime = now;
        }
      }
    }
  }

  // Periodic alarm ringing sound (1s chirp)
  if (isAlarmRinging) {
    if (now - lastAlarmSoundTime >= 1000) {
      lastAlarmSoundTime = now;
      audio.playSound(SOUND_CHIRP);
    }
  }

  // Periodic reminder ringing sound (2s powerup chirp)
  if (isReminderRinging) {
    if (now - lastReminderSoundTime >= 2000) {
      lastReminderSoundTime = now;
      audio.playSound(SOUND_POWERUP);
    }
  }

  // 4. Inactivity Timer: Auto-return to Face screen after 15 seconds of no interaction in UI modes
  if (currentScreen != SCREEN_FACE && !inIntroPhase && !isAlarmRinging && !isReminderRinging && !mapsActive) {
    if (now - lastInteractionTime >= 15000) {
      currentScreen = SCREEN_FACE;
      lastExpressionCycleTime = now;
      Serial.println("Inactivity timeout: Returning to GIF expressions screen.");
    }
  }

  // 5. Periodic status updates to BLE client
  if (bleActive && ble.isConnected() && (now - lastStatusUpdateTime > STATUS_UPDATE_INTERVAL)) {
    lastStatusUpdateTime = now;
    
    unsigned long uptimeSec = now / 1000;
    float mockBatteryVolts = 3.82f;
    ble.updateStatus(uptimeSec, touchCount, mockBatteryVolts, face.getExpression(), face.getStateLabel());
  }

  // Update GIF frame states on every loop iteration
  face.update();

  // Draw the display at ~30fps rate
  static int lastDrawnSecond = -1;
  static unsigned long lastDisplayDrawTime = 0;
  if (now - lastDisplayDrawTime >= 33) {
    lastDisplayDrawTime = now;
    lastDrawnSecond = rtcSecond;
    face.setConnectivityStatus(ble.isConnected(), network.isWifiConnected());
    face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);
  }
}
