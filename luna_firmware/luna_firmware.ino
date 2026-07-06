#include <WiFi.h>
#include <Network.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
LunaFace face(display);
LunaAudio audio(BUZZER_PIN);
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
bool inSettingsMenu = false;
int menuOption = 0; // 0: BLE, 1: GIF Speed, 2: Save, 3: Exit
bool optionSelected = false;

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
      face.setNotificationText(notificationText);
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
  } else if (text.startsWith("NOTIF:")) {
    // Command format: NOTIF:Title|Body
    String payload = text.substring(6);
    int sep = payload.indexOf('|');
    if (sep > 0) {
      String notifTitle = payload.substring(0, sep);
      String notifBody = payload.substring(sep + 1);
      notifTitle.trim();
      notifBody.trim();
      face.setDetailedNotification(notifTitle, notifBody);
    } else {
      face.setNotificationText(payload);
    }
    audio.playSound(SOUND_CHIRP);
    activeNotificationDurationMs = notificationDurationMs;
  } else {
    // Normal text message notification
    face.setNotificationText(text);
    audio.playSound(SOUND_CHIRP); // alert user
    activeNotificationDurationMs = notificationDurationMs;
  }
}

void applySettings(String payload) {
  // Robust CSV parsing — split by commas into an array
  // Expected format: ble,speed,defaultGif,gifIntro,touchSingle,touchDouble,touchLong,negative,introSpeed,introSoundSpeed,notifDur,remDur,birthDur
  String parts[13];
  int partCount = 0;
  int startIdx = 0;
  for (int i = 0; i <= payload.length() && partCount < 13; i++) {
    if (i == (int)payload.length() || payload[i] == ',') {
      String part = payload.substring(startIdx, i);
      part.trim();
      parts[partCount++] = part;
      startIdx = i + 1;
    }
  }

  if (partCount < 2) return; // Need at least ble,speed

  bleActive   = (parts[0] == "1");
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
  display.invertDisplay(negativeDisplay);
  Serial.print("NegativeDisplay set to: ");
  Serial.println(negativeDisplay ? "ON" : "OFF");

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
  Serial.print(negativeDisplay ? "Ms. Luna Robot Booting Up... Version: " : "Mr. Luna Robot Booting Up... Version: ");
  Serial.println(FIRMWARE_VERSION);

  // Load persistence settings from NVS Preferences
  preferences.begin("luna", false);
  bleActive = preferences.getBool("ble", true);
  gifSpeed = preferences.getInt("speed", 100);
  defaultGif = preferences.getInt("defGif", 99);
  gifIntro = preferences.getInt("intGif", 1);
  touchSingle = preferences.getInt("tchSing", 2);  // default: skip animation
  touchDouble = preferences.getInt("tchDoub", 0);
  touchLong = preferences.getInt("tchLong", 0);
  negativeDisplay = preferences.getBool("neg", false);
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
  Wire.setClock(800000); // 800kHz high-speed I2C for smooth rendering
  
  // Initialize SSD1306 Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed. Check connections!"));
    for (;;); // Stop execution
  }
  // Apply saved invert setting immediately after display init
  display.invertDisplay(negativeDisplay);

  // Display initial loading face
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 25);
  display.print(negativeDisplay ? "Loading Ms. Luna..." : "Loading Mr. Luna...");
  display.display();
  
  // Start Bluetooth BLE Server if active
  if (bleActive) {
    ble.init();
    Serial.println(negativeDisplay ? "BLE Server Started as 'Ms. Luna Robot'." : "BLE Server Started as 'Mr. Luna Robot'.");
  } else {
    Serial.println("BLE Server disabled by startup settings.");
  }

  // Boot sequence animation & sound
  delay(100); // Faster boot!
  audio.playSound(SOUND_STARTUP, introSoundSpeed);
  
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
      bleActive = !bleActive;
      ble.setBLEActive(bleActive);
      if (bleActive) {
        audio.playSound(SOUND_POWERUP);
      } else {
        audio.playSound(SOUND_POWERDOWN);
      }
      Serial.print("Toggled BLE: ");
      Serial.println(bleActive ? "ON" : "OFF");
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

void loop() {
  unsigned long now = millis();

  // 1. Maintain BLE stack status and connection advertisement
  ble.handleConnectionState();

  // 2. Refresh non-blocking audio synthesizer
  audio.update();

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

    if (isAlarmRinging || isReminderRinging) {
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
    } else if (isAsleep && !inSettingsMenu) {
      // Any touch wakes the robot up
      isAsleep = false;
      face.setExpression(EXPR_IDLE);
      audio.playSound(SOUND_CHIRP);
      Serial.println(negativeDisplay ? "Ms. Luna Woke Up!" : "Mr. Luna Woke Up!");
    } else if (face.getExpression() == EXPR_CLOCK) {
      // Any touch exits clock mode
      face.setExpression(EXPR_IDLE);
      audio.playSound(SOUND_CHIRP);
      Serial.println("Exited clock mode");
    } else {
      if (inSettingsMenu) {
        // Settings Menu Touch Logic
        if (optionSelected) {
          // Adjusting an option value
          if (touchEvent == TOUCH_TAP) {
            if (menuOption == 0) { // BLE on/off toggle
              bleActive = !bleActive;
              ble.setBLEActive(bleActive);
              audio.playSound(SOUND_CHIRP);
            } else if (menuOption == 1) { // GIF speed control in numbers
              gifSpeed += 20;
              if (gifSpeed > 300) {
                gifSpeed = 20;
              }
              face.setFrameDelay(gifSpeed);
              audio.playSound(SOUND_CHIRP);
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
            menuOption = (menuOption + 1) % 4;
            audio.playSound(SOUND_CHIRP);
          } else if (touchEvent == TOUCH_LONG_PRESS) {
            // Long press selects options
            if (menuOption == 2) { // SAVE
              preferences.begin("luna", false);
              preferences.putBool("ble", bleActive);
              preferences.putInt("speed", gifSpeed);
              preferences.putInt("defGif", defaultGif);
              preferences.putInt("intGif", gifIntro);
              preferences.putInt("tchSing", touchSingle);
              preferences.putInt("tchDoub", touchDouble);
              preferences.putInt("tchLong", touchLong);
              preferences.putBool("neg", negativeDisplay);
              preferences.end();
              audio.playSound(SOUND_POWERUP);
              optionSelected = false; // deselect
            } else if (menuOption == 3) { // EXIT
              inSettingsMenu = false;
              audio.playSound(SOUND_POWERDOWN);
            } else {
              // BLE or Speed option select
              optionSelected = true;
              audio.playSound(SOUND_COIN);
            }
          }
        }
      } else {
        // Normal state controls
        switch (touchEvent) {
          case TOUCH_TAP:
            touchCount++;
            Serial.print("Touch count (Single Tap): ");
            Serial.println(touchCount);
            executeTouchAction(touchSingle, TOUCH_TAP);
            break;

          case TOUCH_DOUBLE_TAP:
            touchCount += 2;
            Serial.print("Touch count (Double Tap): ");
            Serial.println(touchCount);
            executeTouchAction(touchDouble, TOUCH_DOUBLE_TAP);
            break;

          case TOUCH_TRIPLE_TAP:
            // Open local settings menu
            inSettingsMenu = true;
            menuOption = 0;
            optionSelected = false;
            audio.playSound(SOUND_POWERUP);
            Serial.println("Local Settings Menu opened.");
            break;

          case TOUCH_LONG_PRESS:
            executeTouchAction(touchLong, TOUCH_LONG_PRESS);
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

  // 3.6. Expression cycling and transitions (play once for all-gifs cycle, timeout for static reactions)
  if (!inSettingsMenu && !isAsleep && face.getExpression() != EXPR_CLOCK) {
    if (inIntroPhase) {
      if (face.isGifFinished()) {
        face.clearGifFinished();
        inIntroPhase = false;
        face.setFrameDelay(gifSpeed);
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
      if (face.getExpression() == EXPR_ALL_GIF) {
        if (face.isGifFinished()) {
          face.clearGifFinished();
          if (mapsActive) {
            face.setExpression(EXPR_MAP);
            lastExpressionCycleTime = now;
          } else if (isCycleMode) {
            // Only switch to a different random GIF if at least 8 seconds has elapsed since last cycle!
            if (now - lastExpressionCycleTime >= 8000) {
              cycleExpression();
              lastExpressionCycleTime = now;
            }
          }
        }
      } else {
        // Return to random emoji cycling/default expression after notification duration
        if (face.getExpression() != EXPR_MAP && !isReminderRinging && (now - lastExpressionCycleTime >= (unsigned long)activeNotificationDurationMs)) {
          if (mapsActive) {
            face.setExpression(EXPR_MAP);
          } else if (isCycleMode) {
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

  // 4. Inactivity Timer (Timeout to Sleep) - DISABLED
  // (We do not automatically transition to EXPR_SLEEPING via inactivity timer)

  // 5. Periodic status updates to BLE client
  if (bleActive && ble.isConnected() && (now - lastStatusUpdateTime > STATUS_UPDATE_INTERVAL)) {
    lastStatusUpdateTime = now;
    
    unsigned long uptimeSec = now / 1000;
    float mockBatteryVolts = 3.82f;
    ble.updateStatus(uptimeSec, touchCount, mockBatteryVolts, face.getExpression(), face.getStateLabel());
  }

  // Update GIF frame states on every loop iteration for microsecond precision
  bool faceChanged = false;
  if (!inSettingsMenu) {
    faceChanged = face.update();
  }

  // Draw the display under these conditions:
  // 1. In settings menu (draw at 40fps rate / every 25ms)
  // 2. Face changed (frame advanced or text scrolled)
  // 3. Current clock second changed while on clock/map screens
  // 4. Fallback redraw every 500ms
  static int lastDrawnSecond = -1;
  bool timeUpdated = (rtcSecond != lastDrawnSecond);

  static unsigned long lastDisplayDrawTime = 0;
  bool forceRedraw = (now - lastDisplayDrawTime >= 500);

  if (inSettingsMenu) {
    // Redraw settings menu at ~40fps
    if (now - lastDisplayDrawTime >= 25) {
      lastDisplayDrawTime = now;
      face.drawSettingsMenu(menuOption, optionSelected, bleActive, gifSpeed);
    }
  } else if (faceChanged || ((face.getExpression() == EXPR_CLOCK || face.getExpression() == EXPR_MAP) && timeUpdated) || forceRedraw) {
    lastDisplayDrawTime = now;
    lastDrawnSecond = rtcSecond;
    face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, is12HourFormat);
  }
}
