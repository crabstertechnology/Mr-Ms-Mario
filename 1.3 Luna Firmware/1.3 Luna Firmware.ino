#include <WiFi.h>
#include <Network.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>
#include <SPIFFS.h>

#include "config.h"
#include "expressions.h"
#include "audio.h"
#include "bluetooth.h"
#include "interaction.h"
#include "games.h"
#include "qr_card.h"

#define BUTTON1_PIN BTN_EXPR_PIN
#define BUTTON2_PIN BTN_SETTINGS_PIN

// Forward declaration for network callbacks
void handleRobotCommand(String cmd);

#include "luna_network.h"


// Hardware Interface Objects
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
LunaCanvas16 display(SCREEN_WIDTH, SCREEN_HEIGHT);
LunaFace face(tft, display);
LunaAudio audio;
volatile int micAmplitude = 0;
LunaBLE ble;
LunaInteraction interaction;   // dual-button handler (BTN_EXPR_PIN + BTN_SETTINGS_PIN)
LunaNetwork network;

// NVS Settings Persistence
Preferences preferences;
bool bleActive = true;
int gifSpeed = 169;
int gifIntroSpeed = 169;
int introSoundSpeed = 80;
int defaultGif = 99;      // default GIF expression (0-6, or 99 for Cycle Mode)
bool isCycleMode = true;
int gifIntro = 1;        // intro GIF expression (0-6)
int touchSingle = 2;     // action for single tap: 0=default, 1=clock, 2=skip_anim, 3=ble_toggle, 10-16=specific expr
int touchDouble = 0;     // action for double tap
int touchLong = 0;       // action for long press
bool negativeDisplay = false; // SSD1306 display color inversion
bool silentMode = false;
int clockStyle = 0; // clock style selector (0 to 3)
int oledBrightness = 2; // screen brightness (1: Low, 2: Med, 3: High)
bool settingsActive = false;
bool notificationsActive = false;
bool notificationSelected = false;
int menuOption = 0; // 0: BLE, 1: GIF Speed, 2: Clock Style, 3: Invert, 4: Brightness, 5: Save, 6: Exit
volatile bool hardwareLoopbackActive = false;
bool optionSelected = false;
SmartwatchScreen currentScreen = SCREEN_FACE;

bool gamesActive = false;
bool gamePlaying = false;
int gameMenuOption = 0;
int gameSelected = 0;
LunaGames games;
LunaQR qrCard;   // Digital Business Card QR manager

// System State Variables
unsigned int touchCount = 0;
float batteryVolts = 3.82f;
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
bool isRelationCommEnabled = true;
String companionMac = "";
String relType = "";
String robotVariant = "ms_luna";

struct NVSEventItem {
  char id[20];     // Unique ID (e.g. "1726123456789" or "ev_0")
  char type[12];   // "meeting", "reminder", "alarm", "birthday"
  char date[12];   // "12 Sep" (or "*" for daily)
  char time[6];    // "HH:MM"
  char title[32];  // event title
  bool active;
};
static const int MAX_NVS_EVENTS = 10;
NVSEventItem nvsEvents[MAX_NVS_EVENTS];
int nvsEventCount = 0;
int lastTriggeredAlarmMinute = -1;
unsigned long alarmRingStartTime = 0;
String activeRingingId = "";
String activeRingingType = "";
String activeRingingTitle = "";
String activeRingingTime = "";

void saveAllEventsToNVS() {
  preferences.begin("luna", false);
  preferences.putInt("cal_cnt", nvsEventCount);
  char key[12];
  char val[96];
  for (int i = 0; i < nvsEventCount; i++) {
    snprintf(key, sizeof(key), "cal_%d", i);
    snprintf(val, sizeof(val), "%s|%s|%s|%s|%s|%d",
      nvsEvents[i].id,
      nvsEvents[i].type,
      nvsEvents[i].date,
      nvsEvents[i].time,
      nvsEvents[i].title,
      nvsEvents[i].active ? 1 : 0
    );
    preferences.putString(key, val);
  }
  // Clear any dangling old keys beyond nvsEventCount
  for (int i = nvsEventCount; i < MAX_NVS_EVENTS; i++) {
    snprintf(key, sizeof(key), "cal_%d", i);
    if (preferences.isKey(key)) {
      preferences.remove(key);
    }
  }
  preferences.end();
}

void loadCalendarEventsFromNVS() {
  char key[12];
  preferences.begin("luna", false);
  robotVariant = preferences.getString("robot_var", "ms_luna");
  nvsEventCount = preferences.getInt("cal_cnt", 0);
  if (nvsEventCount > MAX_NVS_EVENTS) nvsEventCount = MAX_NVS_EVENTS;

  face.clearCalendarEvents();

  for (int i = 0; i < nvsEventCount; i++) {
    snprintf(key, sizeof(key), "cal_%d", i);
    String s = preferences.getString(key, "");
    if (s.length() > 0) {
      int seps[6];
      int sepCount = 0;
      int lastPos = -1;
      while ((lastPos = s.indexOf('|', lastPos + 1)) >= 0 && sepCount < 6) {
        seps[sepCount++] = lastPos;
      }

      if (sepCount >= 5) {
        // Format: id|type|date|time|title|active
        String idStr    = s.substring(0, seps[0]);
        String typeStr  = s.substring(seps[0] + 1, seps[1]);
        String dateStr  = s.substring(seps[1] + 1, seps[2]);
        String timeStr  = s.substring(seps[2] + 1, seps[3]);
        String titleStr = s.substring(seps[3] + 1, seps[4]);
        int actVal      = s.substring(seps[4] + 1).toInt();

        strncpy(nvsEvents[i].id,    idStr.c_str(),    sizeof(nvsEvents[i].id) - 1);    nvsEvents[i].id[sizeof(nvsEvents[i].id)-1] = 0;
        strncpy(nvsEvents[i].type,  typeStr.c_str(),  sizeof(nvsEvents[i].type) - 1);  nvsEvents[i].type[sizeof(nvsEvents[i].type)-1] = 0;
        strncpy(nvsEvents[i].date,  dateStr.c_str(),  sizeof(nvsEvents[i].date) - 1);  nvsEvents[i].date[sizeof(nvsEvents[i].date)-1] = 0;
        strncpy(nvsEvents[i].time,  timeStr.c_str(),  sizeof(nvsEvents[i].time) - 1);  nvsEvents[i].time[sizeof(nvsEvents[i].time)-1] = 0;
        strncpy(nvsEvents[i].title, titleStr.c_str(), sizeof(nvsEvents[i].title) - 1); nvsEvents[i].title[sizeof(nvsEvents[i].title)-1] = 0;
        nvsEvents[i].active = (actVal != 0);

        face.addCalendarEvent(nvsEvents[i].id, nvsEvents[i].type, nvsEvents[i].date, nvsEvents[i].time, nvsEvents[i].title);
      } else if (sepCount >= 2) {
        // Legacy format: type|time|title
        String typeStr  = s.substring(0, seps[0]);
        String timeStr  = s.substring(seps[0] + 1, seps[1]);
        String titleStr = s.substring(seps[1] + 1);
        char genId[16];
        snprintf(genId, sizeof(genId), "ev_%d", i);

        strncpy(nvsEvents[i].id,    genId,            sizeof(nvsEvents[i].id) - 1);
        strncpy(nvsEvents[i].type,  typeStr.c_str(),  sizeof(nvsEvents[i].type) - 1);  nvsEvents[i].type[sizeof(nvsEvents[i].type)-1] = 0;
        strncpy(nvsEvents[i].date,  "*",              sizeof(nvsEvents[i].date) - 1);
        strncpy(nvsEvents[i].time,  timeStr.c_str(),  sizeof(nvsEvents[i].time) - 1);  nvsEvents[i].time[sizeof(nvsEvents[i].time)-1] = 0;
        strncpy(nvsEvents[i].title, titleStr.c_str(), sizeof(nvsEvents[i].title) - 1); nvsEvents[i].title[sizeof(nvsEvents[i].title)-1] = 0;
        nvsEvents[i].active = true;

        face.addCalendarEvent(nvsEvents[i].id, nvsEvents[i].type, nvsEvents[i].date, nvsEvents[i].time, nvsEvents[i].title);
      }
    }
  }
  preferences.end();
}

void addOrUpdateHardwareEvent(const String& id, const String& type, const String& date, const String& time, const String& title) {
  int foundIdx = -1;
  for (int i = 0; i < nvsEventCount; i++) {
    if (id.length() > 0 && strcmp(nvsEvents[i].id, id.c_str()) == 0) {
      foundIdx = i;
      break;
    }
  }

  if (foundIdx != -1) {
    // Update existing event
    strncpy(nvsEvents[foundIdx].type,  type.c_str(),  sizeof(nvsEvents[foundIdx].type) - 1);  nvsEvents[foundIdx].type[sizeof(nvsEvents[foundIdx].type)-1] = 0;
    strncpy(nvsEvents[foundIdx].date,  date.c_str(),  sizeof(nvsEvents[foundIdx].date) - 1);  nvsEvents[foundIdx].date[sizeof(nvsEvents[foundIdx].date)-1] = 0;
    strncpy(nvsEvents[foundIdx].time,  time.c_str(),  sizeof(nvsEvents[foundIdx].time) - 1);  nvsEvents[foundIdx].time[sizeof(nvsEvents[foundIdx].time)-1] = 0;
    strncpy(nvsEvents[foundIdx].title, title.c_str(), sizeof(nvsEvents[foundIdx].title) - 1); nvsEvents[foundIdx].title[sizeof(nvsEvents[foundIdx].title)-1] = 0;
    nvsEvents[foundIdx].active = true;
  } else {
    // Insert new event at front
    if (nvsEventCount >= MAX_NVS_EVENTS) {
      nvsEventCount = MAX_NVS_EVENTS - 1;
    }
    for (int i = nvsEventCount; i > 0; i--) {
      nvsEvents[i] = nvsEvents[i - 1];
    }
    String realId = id;
    if (realId.length() == 0) realId = String(millis());
    strncpy(nvsEvents[0].id,    realId.c_str(), sizeof(nvsEvents[0].id) - 1);    nvsEvents[0].id[sizeof(nvsEvents[0].id)-1] = 0;
    strncpy(nvsEvents[0].type,  type.c_str(),   sizeof(nvsEvents[0].type) - 1);  nvsEvents[0].type[sizeof(nvsEvents[0].type)-1] = 0;
    strncpy(nvsEvents[0].date,  date.c_str(),   sizeof(nvsEvents[0].date) - 1);  nvsEvents[0].date[sizeof(nvsEvents[0].date)-1] = 0;
    strncpy(nvsEvents[0].time,  time.c_str(),   sizeof(nvsEvents[0].time) - 1);  nvsEvents[0].time[sizeof(nvsEvents[0].time)-1] = 0;
    strncpy(nvsEvents[0].title, title.c_str(),  sizeof(nvsEvents[0].title) - 1); nvsEvents[0].title[sizeof(nvsEvents[0].title)-1] = 0;
    nvsEvents[0].active = true;
    nvsEventCount++;
  }

  saveAllEventsToNVS();
  face.addCalendarEvent(id, type, date, time, title);
  Serial.printf("[NVS] Event saved: '%s' [%s] @ %s (%s)\n", title.c_str(), type.c_str(), time.c_str(), date.c_str());
}

void saveCalendarEventToNVS(const String& type, const String& time, const String& title) {
  addOrUpdateHardwareEvent(String(millis()), type, "*", time, title);
}

bool deleteHardwareEvent(const String& id) {
  int foundIdx = -1;
  for (int i = 0; i < nvsEventCount; i++) {
    if (strcmp(nvsEvents[i].id, id.c_str()) == 0 ||
        (strlen(nvsEvents[i].id) == 0 && strcmp(nvsEvents[i].title, id.c_str()) == 0)) {
      foundIdx = i;
      break;
    }
  }

  if (foundIdx != -1) {
    for (int i = foundIdx; i < nvsEventCount - 1; i++) {
      nvsEvents[i] = nvsEvents[i + 1];
    }
    nvsEvents[nvsEventCount - 1].active = false;
    nvsEventCount--;
    saveAllEventsToNVS();
    face.removeCalendarEvent(id);
    Serial.printf("[NVS] Deleted event %s permanently. Remaining: %d\n", id.c_str(), nvsEventCount);
    return true;
  }
  return false;
}

void clearAllHardwareEvents() {
  nvsEventCount = 0;
  saveAllEventsToNVS();
  face.clearCalendarEvents();
  Serial.println(F("[NVS] All events cleared permanently."));
}

void sendAllEventsToBLE() {
  if (!ble.isConnected()) return;
  ble.sendLog("EVT_START:" + String(nvsEventCount));
  delay(10);
  for (int i = 0; i < nvsEventCount; i++) {
    if (nvsEvents[i].active) {
      String payload = "EVT:" + String(nvsEvents[i].id) + "|" +
                                String(nvsEvents[i].type) + "|" +
                                String(nvsEvents[i].date) + "|" +
                                String(nvsEvents[i].time) + "|" +
                                String(nvsEvents[i].title);
      ble.sendLog(payload);
      delay(15);
    }
  }
  ble.sendLog("EVT_END");
}

void dismissAlarmRinging() {
  if (isAlarmRinging || isReminderRinging || face.isAlarmRingingActive()) {
    isAlarmRinging = false;
    isReminderRinging = false;
    face.setAlarmRinging(false);
    audio.playSound(SOUND_COIN);
    Serial.println(F("[Alarm] Ringing dismissed."));
    if (ble.isConnected()) {
      ble.sendLog("ALARM_DISMISSED:" + activeRingingId);
    }
    activeRingingId = "";
  }
}

// Software Real-Time Clock variables
int rtcHour = 12;
int rtcMinute = 0;
int rtcSecond = 0;
String rtcDay = "Mon";
String rtcDate = "12 Sep";
unsigned long lastRtcMillis = 0;
bool is12HourFormat = false;

void checkHardwareScheduledAlarms() {
  if (rtcSecond == 0 && rtcMinute != lastTriggeredAlarmMinute) {
    char currentHHMM[6];
    snprintf(currentHHMM, sizeof(currentHHMM), "%02d:%02d", rtcHour, rtcMinute);

    String normRtcDate = rtcDate;
    normRtcDate.trim();

    for (int i = 0; i < nvsEventCount; i++) {
      if (!nvsEvents[i].active) continue;

      // 1. Check exact time match
      if (strcmp(nvsEvents[i].time, currentHHMM) != 0) continue;

      // 2. Check calendar date match
      String evDate = String(nvsEvents[i].date);
      evDate.trim();
      bool dateMatches = false;
      if (evDate.length() == 0 || evDate == "*" || evDate.equalsIgnoreCase("daily")) {
        dateMatches = true; // Daily recurring alarm
      } else {
        // Compare with RTC date (e.g. "12 Sep")
        if (evDate.equalsIgnoreCase(normRtcDate)) {
          dateMatches = true;
        } else if (normRtcDate.length() > 0 && evDate.indexOf(normRtcDate) >= 0) {
          dateMatches = true;
        } else if (normRtcDate.length() > 0 && normRtcDate.indexOf(evDate) >= 0) {
          dateMatches = true;
        }
      }

      if (dateMatches) {
        lastTriggeredAlarmMinute = rtcMinute;
        isReminderRinging = true;
        isAlarmRinging = true;
        alarmRingStartTime = millis();
        lastReminderSoundTime = millis();
        lastAlarmSoundTime = millis();

        activeRingingId    = String(nvsEvents[i].id);
        activeRingingType  = String(nvsEvents[i].type);
        activeRingingTitle = String(nvsEvents[i].title);
        activeRingingTime  = String(nvsEvents[i].time);

        // Wake screen if asleep
        isAsleep = false;

        // Show visual ringing overlay on screen
        face.setAlarmRinging(true, activeRingingType, activeRingingTitle, activeRingingTime);

        // Immediate ringing sound
        if (strcmp(nvsEvents[i].type, "birthday") == 0 || strcmp(nvsEvents[i].type, "alarm") == 0) {
          audio.playSound(SOUND_POWERUP);
        } else {
          audio.playSound(SOUND_CHIRP);
        }
        Serial.println("Scheduled Hardware Alarm Ringing for: " + activeRingingTitle + " @ " + activeRingingTime + " (" + activeRingingType + ")");
        break;
      }
    }
  }
}

bool isCompanionPaired() {
  return (companionMac.length() > 0 && companionMac != "none" && relType.length() > 0 && relType != "none");
}

bool isRelationshipActive() {
  return isRelationCommEnabled && isCompanionPaired();
}

// Relationship Action Mappings
int relTapExpr = 1; // Default Happy
int relTapSound = 2; // Default Coin
int relDoubleExpr = 6; // Default Wink
int relDoubleSound = 6; // Default Jump
int relTripleExpr = 4; // Default Surprised
int relTripleSound = 8;
int relLongExpr = 5; // Default Sleeping
int relLongSound = 4; // Default Powerdown


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
  if (mapsActive) return;
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on BLE action
  if (isAsleep && expr != EXPR_SLEEPING) {
    isAsleep = false;
    audio.playSound(SOUND_CHIRP);
  }
  
  if (expr == EXPR_CLOCK) {
    currentScreen = SCREEN_CLOCK;
    face.setExpression(EXPR_CLOCK);
    face.setStateLabel("CLOCK");
    audio.playSound(SOUND_CHIRP);
    Serial.println("Triggered Full Screen Clock via BLE command");
    return;
  }
  
  if (label.length() > 0) {
    label.toUpperCase();
    int foundIdx = -1;
    for (int i = 0; i < SPRITE_AI13_ANIMATION_COUNT; i++) {
      if (label.equals(getSpriteAi13AnimName(i))) {
        foundIdx = i;
        break;
      }
    }
    if (foundIdx != -1) {
      face.setGifIndex(foundIdx);
      expr = (Expression)foundIdx;
    }
  }
  
  face.setExpression(expr);
  if (label.length() > 0) {
    face.setStateLabel(label);
  } else {
    face.setStateLabel(getExpressionName((int)expr));
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
  if (mapsActive) return;
  lastInteractionTime = millis();
  audio.playSound(sound);
}

void handleBLEText(String text) {
  handleRobotCommand(text);
}

void notifyScreenAndExprSync() {
  if (!ble.isConnected()) return;
  
  String screenName = "FACE";
  switch (currentScreen) {
    case SCREEN_FACE: screenName = "FACE"; break;
    case SCREEN_CARD: screenName = "CARD"; break;
    case SCREEN_CLOCK: screenName = "CLOCK"; break;
    case SCREEN_NOTIFICATIONS: screenName = "NOTIF"; break;
    case SCREEN_CALENDAR: screenName = "CALENDAR"; break;
    case SCREEN_GAMES: screenName = "GAMES"; break;
    case SCREEN_SETTINGS: screenName = "SETTINGS"; break;
    case SCREEN_MAPS: screenName = "MAPS"; break;
    default: screenName = "FACE"; break;
  }
  
  int animIndex = (int)face.getExpression();
  String label = face.getStateLabel();
  ble.sendLog("EXPR_SYNC:" + String(animIndex) + "|" + label);
  ble.sendLog("SCREEN_SYNC:" + screenName);
  
  unsigned long uptimeSec = millis() / 1000;
  ble.updateStatus(uptimeSec, touchCount, batteryVolts, (Expression)animIndex, label);
}

void handleRobotCommand(String text) {
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on interaction
  
  if (mapsActive && !text.startsWith("MAP") && !text.startsWith("SCREEN:") && !text.startsWith("CALL:") && !text.startsWith("TIME:")) {
    Serial.println("[BLE] Ignored command because MAPS is active");
    return;
  }
  
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
      companionMac = payload.substring(0, comma);
      relType = payload.substring(comma + 1);
      companionMac.trim();
      relType.trim();
      
      if (companionMac == "none" || relType == "none" || companionMac.length() == 0 || relType.length() == 0) {
        companionMac = "";
        relType = "none";
      }
      
      preferences.begin("luna", false);
      preferences.putString("comp_mac", companionMac);
      preferences.putString("rel_type", relType);
      preferences.end();
      
      audio.playSound(SOUND_POWERUP);
      Serial.println("Companion paired: MAC=" + companionMac + ", Relation=" + relType);
    }
  } else if (text == "UNPAIR" || text.startsWith("UNPAIR")) {
    companionMac = "";
    relType = "none";
    preferences.begin("luna", false);
    preferences.putString("comp_mac", "");
    preferences.putString("rel_type", "none");
    preferences.end();
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("Companion unpaired.");
  } else if (text.startsWith("RELATION:")) {
    // Command format: RELATION:type (friends, couple, none)
    relType = text.substring(9);
    relType.trim();
    if (relType == "none" || relType.length() == 0) {
      companionMac = "";
      relType = "none";
    }
    preferences.begin("luna", false);
    preferences.putString("comp_mac", companionMac);
    preferences.putString("rel_type", relType);
    preferences.end();
    
    // Play romantic sound for couple, friendly chime for friends
    if (relType == "couple") {
      audio.playSound(SOUND_POWERUP);
    } else {
      audio.playSound(SOUND_COIN);
    }
    Serial.println("Relationship status updated: " + relType);
  } else if (text.startsWith("MODEL:")) {
    String newVariant = text.substring(6);
    newVariant.trim();
    if (newVariant != robotVariant) {
      robotVariant = newVariant;
      preferences.begin("luna", false);
      preferences.putString("robot_var", robotVariant);
      preferences.end();
      Serial.println("OK:ModelVariantUpdated:" + robotVariant);
      delay(500);
      ESP.restart();
    } else {
      Serial.println("OK:ModelVariantAlreadyMatching:" + robotVariant);
    }
  } else if (text == "EVT_GET" || text == "CAL_GET") {
    sendAllEventsToBLE();
  } else if (text.startsWith("EVT_ADD:")) {
    // Command format: EVT_ADD:id,type,date,time,title
    String payload = text.substring(8);
    int c1 = payload.indexOf(',');
    int c2 = payload.indexOf(',', c1 + 1);
    int c3 = payload.indexOf(',', c2 + 1);
    int c4 = payload.indexOf(',', c3 + 1);
    if (c1 > 0 && c2 > c1 && c3 > c2 && c4 > c3) {
      String id    = payload.substring(0, c1);
      String type  = payload.substring(c1 + 1, c2);
      String date  = payload.substring(c2 + 1, c3);
      String time  = payload.substring(c3 + 1, c4);
      String title = payload.substring(c4 + 1);

      addOrUpdateHardwareEvent(id, type, date, time, title);
      if (ble.isConnected()) {
        ble.sendLog("EVT_ADD_OK:" + id);
      }
    }
  } else if (text.startsWith("EVT_DEL:") || text.startsWith("CAL_DEL:")) {
    int colon = text.indexOf(':');
    String id = text.substring(colon + 1);
    id.trim();
    bool ok = deleteHardwareEvent(id);
    if (ble.isConnected()) {
      ble.sendLog(ok ? ("EVT_DEL_OK:" + id) : ("EVT_DEL_FAIL:" + id));
    }
  } else if (text == "CAL_CLEAR" || text == "EVT_CLEAR") {
    clearAllHardwareEvents();
    if (ble.isConnected()) {
      ble.sendLog("EVT_CLEAR_OK");
    }
  } else if (text.startsWith("CAL:")) {
    // Legacy Command format: CAL:type,time,title
    String payload = text.substring(4);
    int firstComma = payload.indexOf(',');
    int secondComma = payload.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma) {
      String type = payload.substring(0, firstComma);
      String time = payload.substring(firstComma + 1, secondComma);
      String title = payload.substring(secondComma + 1);

      Serial.println("Calendar Event: type=" + type + ", time=" + time + ", title=" + title);
      addOrUpdateHardwareEvent(String(millis()), type, "*", time, title);
    }
  } else if (text.startsWith("ALARM:") || text == "EVT_DISMISS") {
    if (text == "EVT_DISMISS") {
      dismissAlarmRinging();
    } else {
      String state = text.substring(6);
      if (state == "START") {
        isAlarmRinging = true;
        alarmRingStartTime = millis();
        face.setAlarmRinging(true, "alarm", "ALARM RINGING", String(rtcHour) + ":" + String(rtcMinute));
        face.setExpression(EXPR_CLOCK);
        face.setStateLabel("ALARM!");
        Serial.println("Alarm triggered via BLE/Wi-Fi.");
      } else {
        dismissAlarmRinging();
        face.setExpression(EXPR_IDLE);
        face.setStateLabel("IDLE");
        Serial.println("Alarm stopped/dismissed.");
      }
    }
  } else if (text.startsWith("SCREEN:")) {
    String arg = text.substring(7);
    arg.toUpperCase();
    arg.trim();
    int sVal = -1;
    if (arg == "CLOCK") sVal = SCREEN_CLOCK;
    else if (arg == "NOTIF" || arg == "NOTIFICATIONS") sVal = SCREEN_NOTIFICATIONS;
    else if (arg == "CALENDAR" || arg == "CAL") sVal = SCREEN_CALENDAR;
    else if (arg == "MAP" || arg == "MAPS") sVal = SCREEN_MAPS;
    else if (arg == "GAMES" || arg == "ARCADE") sVal = SCREEN_GAMES;
    else if (arg == "FACE" || arg == "EYES") sVal = SCREEN_FACE;
    else if (arg == "CARD") sVal = SCREEN_CARD;
    else if (arg == "SETTINGS") sVal = SCREEN_SETTINGS;
    else sVal = arg.toInt();

    if (sVal >= 0 && sVal < SCREEN_MAX) {
      currentScreen = (SmartwatchScreen)sVal;
      settingsActive = false;
      gamesActive = false;
      gamePlaying = false;
      notificationsActive = false;
      notifyScreenAndExprSync();
      Serial.printf("OK:ScreenSwitched:%d\n", sVal);
    }
  } else if (text.startsWith("MAP:")) {
    // Command format: MAP:direction,turnDist,road,totalTime,totalDist,eta OR MAP:EXIT
    String payload = text.substring(4);
    payload.trim();
    if (payload == "EXIT") {
      mapsActive = false;
      currentScreen = SCREEN_CLOCK;
      face.setExpression(EXPR_IDLE);
      lastExpressionCycleTime = millis() - activeNotificationDurationMs;
      notifyScreenAndExprSync();
      Serial.println("Maps Navigation Exited.");
      return;
    }
    String parts[6];
    int partIdx = 0;
    int start = 0;
    for (int i = 0; i <= payload.length() && partIdx < 6; i++) {
      if (i == payload.length() || payload.charAt(i) == ',') {
        parts[partIdx++] = payload.substring(start, i);
        start = i + 1;
      }
    }

    String direction = (partIdx >= 1) ? parts[0] : "";
    String turnDist  = (partIdx >= 2) ? parts[1] : "";
    String road      = (partIdx >= 3) ? parts[2] : "";
    String totalTime = (partIdx >= 4) ? parts[3] : "";
    String totalDist = (partIdx >= 5) ? parts[4] : "";
    String eta       = (partIdx >= 6) ? parts[5] : "";

    direction.trim();
    turnDist.trim();
    road.trim();
    totalTime.trim();
    totalDist.trim();
    eta.trim();
    direction.toUpperCase();

    Serial.printf("[MAP] dir='%s' turnDist='%s' road='%s' time='%s' dist='%s' eta='%s'\n",
                  direction.c_str(), turnDist.c_str(), road.c_str(), totalTime.c_str(), totalDist.c_str(), eta.c_str());

    bool shouldBeep = (!mapsActive) || (direction != face.getMapDirection());

    mapsActive = true;
    currentScreen = SCREEN_MAPS;
    isAsleep = false;
    lastInteractionTime = millis();
    face.setMapTelemetry(direction, turnDist, road, totalTime, totalDist, eta);
    notifyScreenAndExprSync();

    if (shouldBeep) {
      audio.playSound(SOUND_CHIRP);
    }

    activeNotificationDurationMs = 20000;
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
  } else if (text.startsWith("REL_COMM:")) {
    int val = text.substring(9).toInt();
    isRelationCommEnabled = (val == 1);
    if (!isRelationCommEnabled) {
      face.headerText = "";
    }
    preferences.begin("luna", false);
    preferences.putBool("relComm", isRelationCommEnabled);
    preferences.end();
    Serial.print("Relationship communication set to: ");
    Serial.println(isRelationCommEnabled ? "ON" : "OFF");
    Serial.println("OK:RelationCommUpdated");
  } else if (text.startsWith("SET_REL_MAP:")) {
    String payload = text.substring(12);
    int sep1 = payload.indexOf('|');
    if (sep1 > 0) {
      int sep2 = payload.indexOf('|', sep1 + 1);
      if (sep2 > 0) {
        int tapType = payload.substring(0, sep1).toInt();
        int expr = payload.substring(sep1 + 1, sep2).toInt();
        int sound = payload.substring(sep2 + 1).toInt();
        
        preferences.begin("luna", false);
        if (tapType == 1) {
          relTapExpr = expr; relTapSound = sound;
          preferences.putInt("rTapEx", expr); preferences.putInt("rTapSd", sound);
        } else if (tapType == 2) {
          relDoubleExpr = expr; relDoubleSound = sound;
          preferences.putInt("rDobEx", expr); preferences.putInt("rDobSd", sound);
        } else if (tapType == 3) {
          relTripleExpr = expr; relTripleSound = sound;
          preferences.putInt("rTriEx", expr); preferences.putInt("rTriSd", sound);
        } else if (tapType == 4) {
          relLongExpr = expr; relLongSound = sound;
          preferences.putInt("rLonEx", expr); preferences.putInt("rLonSd", sound);
        }
        preferences.end();
        Serial.print("Relationship map updated: TapType=");
        Serial.print(tapType);
        Serial.print(" Expr=");
        Serial.print(expr);
        Serial.print(" Sound=");
        Serial.println(sound);
        Serial.println("OK:RelationMapUpdated");
      }
    }
  } else if (text.startsWith("NOTIF_EXPR:")) {
    if (!isRelationshipActive()) {
      Serial.println("Warning: NOTIF_EXPR ignored because relation communication is disabled or companion is not paired.");
      return;
    }
    // Command format: NOTIF_EXPR:Title|Body|ExprId
    String payload = text.substring(11);
    int sep1 = payload.indexOf('|');
    if (sep1 > 0) {
      int sep2 = payload.indexOf('|', sep1 + 1);
      if (sep2 > 0) {
        String title = payload.substring(0, sep1);
        String body = payload.substring(sep1 + 1, sep2);
        int exprVal = payload.substring(sep2 + 1).toInt();
        title.trim();
        body.trim();
        
        face.headerText = title + " - " + body;
        if (face.headerText.length() > 20) {
          face.headerText = face.headerText.substring(0, 17) + "...";
        }
        face.setExpression((Expression)exprVal);
        activeNotificationDurationMs = notificationDurationMs;
        lastExpressionCycleTime = millis();
        audio.playSound(SOUND_CHIRP);
      }
    }
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
  } else if (text.startsWith("WP_START:")) {
    int expectedSize = text.substring(9).toInt();
    File f = SPIFFS.open("/wallpaper.bin", "w");
    if (f) {
      f.close();
      ble.sendLog("WP_START:OK");
      Serial.print("Wallpaper write started, expected size: ");
      Serial.println(expectedSize);
    } else {
      ble.sendLog("WP_START:FAIL");
      Serial.println("Failed to start wallpaper write");
    }
  } else if (text.startsWith("WP_CHUNK:")) {
    String hexData = text.substring(9);
    File f = SPIFFS.open("/wallpaper.bin", "a");
    if (f) {
      size_t hexLen = hexData.length();
      uint8_t* tempBuf = (uint8_t*)malloc(hexLen / 2);
      if (tempBuf) {
        for (size_t i = 0; i < hexLen; i += 2) {
          char high = hexData[i];
          char low = hexData[i + 1];
          uint8_t val = 0;
          if (high >= '0' && high <= '9') val += (high - '0') << 4;
          else if (high >= 'a' && high <= 'f') val += (high - 'a' + 10) << 4;
          else if (high >= 'A' && high <= 'F') val += (high - 'A' + 10) << 4;
          
          if (low >= '0' && low <= '9') val += (low - '0');
          else if (low >= 'a' && low <= 'f') val += (low - 'a' + 10);
          else if (low >= 'A' && low <= 'F') val += (low - 'A' + 10);
          
          tempBuf[i / 2] = val;
        }
        f.write(tempBuf, hexLen / 2);
        free(tempBuf);
        f.close();
        ble.sendLog("WP_CHUNK:OK");
      } else {
        f.close();
        ble.sendLog("WP_CHUNK:FAIL");
      }
    } else {
      ble.sendLog("WP_CHUNK:FAIL");
    }
  } else if (text == "WP_END") {
    File f = SPIFFS.open("/wallpaper.bin", "r");
    if (f) {
      size_t finalSize = f.size();
      f.close();
      ble.sendLog("WP_END:OK");
      Serial.print("Wallpaper transmission finished, size: ");
      Serial.println(finalSize);
    } else {
      ble.sendLog("WP_END:FAIL");
    }
  } else if (text == "WP_CLEAR") {
    SPIFFS.remove("/wallpaper.bin");
    ble.sendLog("WP_CLEAR:OK");
    Serial.println("Wallpaper removed");
  } else if (text == "QRCARD:CLEAR") {
    qrCard.clearCard();
    if (currentScreen == SCREEN_CARD) {
      currentScreen = SCREEN_FACE;
    }
    ble.sendLog("QR_CLEARED:OK");
    Serial.println("[QR] Business card cleared from NVS.");
  } else if (text.startsWith("QRCARD:")) {
    // Digital Business Card sync: "QRCARD:<url>"
    String url = text.substring(7);
    url.trim();
    if (url.length() > 0 && url.length() <= 255) {
      bool saved = qrCard.saveUrl(url);
      if (saved) {
        currentScreen = SCREEN_CARD; // Jump to card screen to display updated QR code
        lastInteractionTime = millis();
        ble.sendLog("QR_SAVED:OK");  // App listens for this to confirm success
        audio.playSound(SOUND_POWERUP);
        Serial.println("[QR] Business card URL saved and displayed: " + url);
      } else {
        ble.sendLog("QR_SAVED:FAIL");
        Serial.println("[QR] Failed to save business card URL!");
      }
    } else {
      ble.sendLog("QR_SAVED:INVALID");
      Serial.println("[QR] Invalid or empty URL received.");
    }
  } else {
    // Normal text message notification
    face.setNotificationText(text, rtcHour, rtcMinute);
    audio.playSound(SOUND_CHIRP); // alert user
    activeNotificationDurationMs = notificationDurationMs;
  }
}

void drawRGBBitmapScaled(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h, int16_t targetW, int16_t targetH) {
  for (int16_t ty = 0; ty < targetH; ty++) {
    int16_t sy = (ty * h) / targetH;
    int32_t rowOffset = (int32_t)sy * w;
    for (int16_t tx = 0; tx < targetW; tx++) {
      int16_t sx = (tx * w) / targetW;
      uint16_t color = pgm_read_word(&bitmap[rowOffset + sx]);
      tft.drawPixel(x + tx, y + ty, color);
    }
  }
}

void applySettings(String payload) {
  // Robust CSV parsing — split by commas into an array
  // Expected format: ble,speed,defaultGif,gifIntro,touchSingle,touchDouble,touchLong,negative,introSpeed,introSoundSpeed,notifDur,remDur,birthDur,clkStyle,oledBright,silentMode
  String parts[16];
  int partCount = 0;
  int startIdx = 0;
  for (int i = 0; i <= payload.length() && partCount < 16; i++) {
    if (i == (int)payload.length() || payload[i] == ',') {
      String part = payload.substring(startIdx, i);
      part.trim();
      parts[partCount++] = part;
      startIdx = i + 1;
    }
  }

  if (partCount < 2) return; // Need at least ble,speed

  bleActive   = true; // Always ON
  gifSpeed    = 169;

  if (partCount > 2) defaultGif  = parts[2].toInt();
  if (partCount > 3) gifIntro    = parts[3].toInt();
  if (partCount > 4) touchSingle = parts[4].toInt();
  if (partCount > 5) touchDouble = parts[5].toInt();
  if (partCount > 6) touchLong   = parts[6].toInt();
  if (partCount > 7) negativeDisplay = (parts[7].toInt() == 1);
  if (partCount > 8) {
    gifIntroSpeed = 169;
  }
  if (partCount > 9) {
    introSoundSpeed = 80;
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
  if (partCount > 15) {
    silentMode = (parts[15].toInt() == 1);
    audio.silentMode = silentMode;
  }

  // Apply settings immediately
  face.setFrameDelay(gifSpeed);

  // defaultGif: 99 = Cycle Mode
  //             0-11 = Sprite AI animation index
  if (defaultGif == 99) {
    isCycleMode = true;
    cycleExpression();
  } else {
    isCycleMode = false;
    int animIdx = defaultGif;
    if (animIdx >= 100) animIdx -= 100;
    if (animIdx < 0 || animIdx >= SPRITE_AI13_ANIMATION_COUNT) animIdx = 0;
    face.setGifIndex(animIdx);
    face.setDefaultExpression((Expression)animIdx);
    face.setExpression((Expression)animIdx);
    face.setStateLabel(String(getSpriteAi13AnimName(animIdx)));
  }

  ble.setBLEActive(bleActive);
  tft.invertDisplay(true);
  
  #ifdef TFT_BLK
  analogWriteFrequency(TFT_BLK, 24000); // 24 kHz high-frequency PWM
  if (oledBrightness == 1) analogWrite(TFT_BLK, 30);
  else if (oledBrightness == 2) analogWrite(TFT_BLK, 128);
  else analogWrite(TFT_BLK, 255);
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
  preferences.putBool("silent", silentMode);
  preferences.end();

  audio.playSound(SOUND_POWERUP);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  display.allocate();
  
  // 1. Load persistence settings from NVS Preferences first
  preferences.begin("luna", false);
  bleActive = true; // Always ON
  gifSpeed = 169;
  defaultGif = preferences.getInt("defGif", 99);
  gifIntro = preferences.getInt("intGif", 1);
  touchSingle = preferences.getInt("tchSing", 2);  // default: skip animation
  touchDouble = preferences.getInt("tchDoub", 0);
  touchLong = preferences.getInt("tchLong", 0);
  robotVariant = preferences.getString("robot_var", "ms_luna");
  negativeDisplay = preferences.getBool("neg", false);
  clockStyle = preferences.getInt("clkStyle", 0);
  oledBrightness = preferences.getInt("oledBright", 2);
  silentMode = preferences.getBool("silent", false);
  audio.silentMode = silentMode;
  gifIntroSpeed = 169;
  introSoundSpeed = 80;
  is12HourFormat = preferences.getBool("is12H", false);
  notificationDurationMs = preferences.getInt("notifDur", 5000);
  reminderDurationMs = preferences.getInt("remDur", 10000);
  birthdayDurationMs = preferences.getInt("birthDur", 15000);
  activeNotificationDurationMs = notificationDurationMs;
  isRelationCommEnabled = preferences.getBool("relComm", true);
  companionMac = preferences.getString("comp_mac", "");
  relType = preferences.getString("rel_type", "");
  relTapExpr = preferences.getInt("rTapEx", 1);
  relTapSound = preferences.getInt("rTapSd", 2);
  relDoubleExpr = preferences.getInt("rDobEx", 6);
  relDoubleSound = preferences.getInt("rDobSd", 6);
  relTripleExpr = preferences.getInt("rTriEx", 4);
  relTripleSound = preferences.getInt("rTriSd", 8);
  relLongExpr = preferences.getInt("rLonEx", 5);
  relLongSound = preferences.getInt("rLonSd", 4);
  preferences.end();

  loadCalendarEventsFromNVS();

  // 2. Start Bluetooth BLE Server first when heap memory is maximum and unfragmented
  bleActive = true;
  ble.init();
  Serial.println(negativeDisplay ? "BLE Server Started as 'Ms. Luna Robot'." : "BLE Server Started as 'Mr. Luna Robot'.");

  // 3. Initialize audio and other hardware pins
  audio.begin();
  interaction.begin();   // sets up both buttons with INPUT_PULLUP (active-low)
  pinMode(BATTERY_PIN, INPUT); // Initialize battery monitoring pin
  // Initial battery read (uses BATTERY_CALIBRATION_MULTIPLIER to account for divider ratio and impedance loading)
  batteryVolts = (analogReadMilliVolts(BATTERY_PIN) * BATTERY_CALIBRATION_MULTIPLIER) / 1000.0f;

  Serial.print(negativeDisplay ? "Ms. Luna Robot Booting Up... Version: " : "Mr. Luna Robot Booting Up... Version: ");
  Serial.println(FIRMWARE_VERSION);

  // Set the GIF speed delay and default expression
  face.setFrameDelay(gifSpeed);
  if (defaultGif == 99) {
    isCycleMode = true;
  } else {
    isCycleMode = false;
    int animIdx = defaultGif;
    if (animIdx >= 100) animIdx -= 100;
    if (animIdx < 0 || animIdx >= SPRITE_AI13_ANIMATION_COUNT) animIdx = 0;
    face.setGifIndex(animIdx);
    face.setDefaultExpression((Expression)animIdx);
    face.setExpression((Expression)animIdx);
  }

  // Perform Hardware Reset
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_BLK, OUTPUT);
  
  digitalWrite(TFT_RST, HIGH);
  delay(50);
  digitalWrite(TFT_RST, LOW);
  delay(100);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  // Initialize Hardware SPI on custom pins
  SPI.begin(TFT_SCL, -1, TFT_SDA, -1);
  
  // Initialize ST7789 Display in SPI MODE 3
  tft.init(240, 240, SPI_MODE3);
  tft.setSPISpeed(20000000UL); // 20 MHz SPI speed
  tft.setRotation(2);          // Rotate right to make it vertical!
  
  tft.invertDisplay(true);   // Standard color representation for IPS screen during logo — gives white background
  tft.fillScreen(ST77XX_WHITE);
  
  display.fillScreen(ST77XX_WHITE);
  
  // Apply saved brightness setting
  if (oledBrightness == 1) analogWrite(TFT_BLK, 30);
  else if (oledBrightness == 2) analogWrite(TFT_BLK, 128);
  else analogWrite(TFT_BLK, 255);

  // Display startup logo on white background
  int logoSize = (SCREEN_WIDTH < SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT;
  int logoX = (SCREEN_WIDTH - logoSize) / 2;
  int logoY = (SCREEN_HEIGHT - logoSize) / 2;
  drawRGBBitmapScaled(logoX, logoY, image_logo_pixels, 240, 240, logoSize, logoSize);

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
  tft.invertDisplay(true);
  
  // Set intro speed
  face.setFrameDelay(gifIntroSpeed);
  inIntroPhase = true;

  int introIdx = gifIntro;
  if (introIdx >= 100) introIdx -= 100;
  if (introIdx < 0 || introIdx >= SPRITE_AI13_ANIMATION_COUNT) introIdx = 0;
  face.setGifIndex(introIdx);
  face.setExpression((Expression)introIdx);

  lastInteractionTime = millis();
  lastRtcMillis = millis();
  lastExpressionCycleTime = millis();

  // Dump all loaded Sprite AI animation names to serial for debugging
  Serial.println("====== SPRITE AI ANIMATION DUMP ======");
  Serial.printf("Total Sprite Animations: %d (4 frames each)\n", SPRITE_AI13_ANIMATION_COUNT);
  for (int i = 0; i < SPRITE_AI13_ANIMATION_COUNT; i++) {
    Serial.printf("ANIM[%d] %s (4 frames)\n", i, getSpriteAi13AnimName(i));
  }
  Serial.println("======================================");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
  games.begin();
  qrCard.begin();  // Load persisted business card URL from NVS
  network.init();
}

// Global index for sprite-ai cycling — advances through all 12 animations
int spriteAnimCycleIdx = 0;

void cycleExpression() {
  spriteAnimCycleIdx = (spriteAnimCycleIdx + 1) % SPRITE_AI13_ANIMATION_COUNT;
  face.setGifIndex(spriteAnimCycleIdx);
  face.setExpression((Expression)spriteAnimCycleIdx);
  const char* animName = getSpriteAi13AnimName(spriteAnimCycleIdx);
  face.setStateLabel(String(animName));
  Serial.printf("PLAYING ANIM[%d/%d]: %s (4 frames)\n",
    spriteAnimCycleIdx, SPRITE_AI13_ANIMATION_COUNT - 1, animName);
}

String getExpressionName(int expr) {
  if (expr >= 100) expr -= 100;
  if (expr >= 0 && expr < SPRITE_AI13_ANIMATION_COUNT) {
    return String(getSpriteAi13AnimName(expr));
  }
  return "IDLE";
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
      currentScreen = SCREEN_CLOCK;
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
      // Specific Sprite AI anim index: actionType = 20 + animIdx
      int animIdx = actionType - 20;
      if (animIdx >= 0 && animIdx < SPRITE_AI13_ANIMATION_COUNT) {
        face.setGifIndex(animIdx);
        face.setExpression((Expression)animIdx);
        spriteAnimCycleIdx = animIdx;
        audio.playSound(SOUND_COIN);
        Serial.printf("Touch triggered specific Anim #%d: %s\n", animIdx, getSpriteAi13AnimName(animIdx));
      }
    } else if (actionType >= 10 && actionType < 10 + SPRITE_AI13_ANIMATION_COUNT) {
      Expression target = (Expression)(actionType - 10);
      face.setExpression(target);
      audio.playSound(SOUND_CHIRP);
      Serial.print("Triggered expression: ");
      Serial.println(actionType - 10);
    }
  }
}
// =============================================================================
// Helper function to adjust Settings options (Direction: +1 for Up/Increment, -1 for Down/Decrement)
// =============================================================================
void adjustOption(int option, int direction) {
  switch (option) {
    case 0: // BLE
      bleActive = true;
      audio.playSound(SOUND_CHIRP);
      break;
    case 1: // GIF Speed
      gifSpeed = 169;
      face.setFrameDelay(169);
      audio.playSound(SOUND_CHIRP);
      break;
    case 2: // Clock Style
      if (direction > 0) {
        clockStyle = (clockStyle + 1) % 3;
      } else {
        clockStyle = (clockStyle - 1 + 3) % 3;
      }
      audio.playSound(SOUND_CHIRP);
      break;
    case 3: // Invert Display
      negativeDisplay = !negativeDisplay;
      tft.invertDisplay(true);
      audio.playSound(SOUND_CHIRP);
      break;
    case 4: // Brightness
      if (direction > 0) {
        oledBrightness = (oledBrightness % 3) + 1;
      } else {
        oledBrightness--;
        if (oledBrightness < 1) oledBrightness = 3;
      }
      if (oledBrightness == 1) analogWrite(TFT_BLK, 30);
      else if (oledBrightness == 2) analogWrite(TFT_BLK, 128);
      else analogWrite(TFT_BLK, 255);
      audio.playSound(SOUND_CHIRP);
      break;
    case 5: // Silent / Buzzer Mode
      silentMode = !silentMode;
      audio.silentMode = silentMode;
      if (!silentMode) {
        audio.playSound(SOUND_CHIRP);
      }
      break;
    case 6: // Save settings
      {
        preferences.begin("luna", false);
        preferences.putBool("ble",       bleActive);
        preferences.putInt("speed",      gifSpeed);
        preferences.putInt("clkStyle",   clockStyle);
        preferences.putBool("neg",       negativeDisplay);
        preferences.putInt("oledBright", oledBrightness);
        preferences.putBool("silent",    silentMode);
        preferences.end();
        audio.playSound(SOUND_POWERUP);
        Serial.println("[BTN] Settings SAVED");
        
        // Notify app about changes!
        String silentValStr = silentMode ? "1" : "0";
        String negValStr = negativeDisplay ? "1" : "0";
        ble.sendLog("SET_SYNC:" + String(clockStyle) + "," + String(oledBrightness) + "," + negValStr + "," + silentValStr);

        optionSelected = false; // deselect
      }
      break;
    case 7: // Exit settings
      optionSelected = false;
      settingsActive = false;
      currentScreen  = SCREEN_FACE;
      audio.playSound(SOUND_POWERDOWN);
      Serial.println("[BTN] Exited Settings");
      break;
  }
}

// =============================================================================
// Button 1 handlers
// =============================================================================
void handleBtn1Single() {
  lastInteractionTime = millis();
  if (mapsActive) return;

  // Button 1 always exits the QR card screen immediately
  if (currentScreen == SCREEN_CARD) {
    currentScreen = SCREEN_FACE;
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("[BTN1] Exited QR Card screen");
    return;
  }

  if (currentScreen == SCREEN_SETTINGS) {
    if (!settingsActive) {
      settingsActive = true;
      menuOption = 0;
      optionSelected = false;
      audio.playSound(SOUND_POWERUP);
      Serial.println("[BTN1] Settings screen ACTIVATED");
    } else {
      if (!optionSelected) {
        menuOption = (menuOption + 1) % 8;
        audio.playSound(SOUND_CHIRP);
        Serial.printf("[BTN1] Settings Option down -> %d\n", menuOption);
      } else {
        adjustOption(menuOption, -1);
      }
    }
    return;
  }

  if (currentScreen == SCREEN_GAMES) {
    if (!gamesActive) {
      gamesActive = true;
      gameMenuOption = 0;
      gamePlaying = false;
      audio.playSound(SOUND_POWERUP);
      Serial.println("[BTN1] Games screen ACTIVATED");
    } else {
      if (!gamePlaying) {
        // Red button (Button 1) cycles games menu down
        gameMenuOption = (gameMenuOption + 1) % 8;
        audio.playSound(SOUND_CHIRP);
        Serial.printf("[BTN1] Games Menu DOWN -> option %d\n", gameMenuOption);
      }
    }
    return;
  }

  // Button 1 single click on other screens:
  if (currentScreen == SCREEN_FACE) {
    // Next expression/animation
    cycleExpression();
    audio.playSound(SOUND_COIN);
    Serial.println("[BTN1] Next expression");
  } else if (currentScreen == SCREEN_CLOCK) {
    // Cycles clock styles
    clockStyle = (clockStyle + 1) % 3;
    audio.playSound(SOUND_CHIRP);
    Serial.println("[BTN1] Cycled clock style");
  } else if (currentScreen == SCREEN_NOTIFICATIONS) {
    if (!notificationsActive) {
      if (face.getNotificationCount() > 0) {
        notificationsActive = true;
        face.setCurrentNotifViewIdx(0);
        notificationSelected = false;
        audio.playSound(SOUND_POWERUP);
        Serial.println("[BTN1] Notifications screen ACTIVATED");
      }
    } else {
      if (!notificationSelected) {
        notificationSelected = true;
        audio.playSound(SOUND_POWERUP);
        Serial.printf("[BTN1] Opened notification %d\n", face.getCurrentNotifViewIdx());
      }
    }
    return;
  } else if (currentScreen == SCREEN_CALENDAR) {
    // Cycles calendar events/view
    face.cycleCalendarView();
    audio.playSound(SOUND_CHIRP);
    Serial.println("[BTN1] Cycled calendar");
  }
}

void handleBtn1Double() {
  lastInteractionTime = millis();
  if (mapsActive) return;
  
  if (currentScreen == SCREEN_GAMES) {
    if (gamePlaying) {
      Serial.println("[BTN1 DBL] Ignored double-click exit because game is playing");
      return;
    }
    gamesActive = false;
    gamePlaying = false;
    currentScreen = SCREEN_FACE;
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("[BTN1 DBL] Switched from Games to FACE screen");
    return;
  }

  // Clear any settings menu activation states
  settingsActive = false;
  optionSelected = false;
  notificationsActive = false;
  notificationSelected = false;

  currentScreen = SCREEN_CLOCK;
  audio.playSound(SOUND_POWERUP);
  Serial.println("[BTN1 DBL] Switched directly to CLOCK screen");
}

void handleBtn1Long() {
  lastInteractionTime = millis();
  if (mapsActive) return;

  if (currentScreen == SCREEN_GAMES) {
    // Long press to go back is removed as requested by the user
    return;
  }

  if (currentScreen == SCREEN_NOTIFICATIONS) {
    if (notificationSelected) {
      notificationSelected = false;
      audio.playSound(SOUND_POWERDOWN);
      Serial.println("[BTN1 LONG] Exited notification detail view");
    } else if (notificationsActive) {
      notificationsActive = false;
      audio.playSound(SOUND_POWERDOWN);
      Serial.println("[BTN1 LONG] Deactivated notifications screen");
    } else {
      currentScreen = SCREEN_FACE;
      audio.playSound(SOUND_STARTUP);
      Serial.println("[BTN1 LONG] Exited Notifications to FACE");
    }
    return;
  }

  if (currentScreen != SCREEN_FACE) {
    // Return to face screen
    settingsActive = false;
    optionSelected = false;
    currentScreen = SCREEN_FACE;
    hardwareLoopbackActive = false;
    audio.micStreaming = false;
    audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
    audio.prebuffering = true;
    face.setStateLabel("IDLE");
    audio.playSound(SOUND_STARTUP);
    Serial.println("[BTN1 LONG] Return to FACE screen");
  }
}

// =============================================================================
// Button 2 handlers
// =============================================================================
void handleBtn2Single() {
  lastInteractionTime = millis();
  if (mapsActive) return;

  if (currentScreen == SCREEN_GAMES && gamesActive) {
    if (!gamePlaying) {
      // Yellow button (Button 2) selects/confirms the option in games menu
      if (gameMenuOption >= 0 && gameMenuOption < 7) {
        gameSelected = gameMenuOption + 1;
        if (gameSelected == 1) games.resetRacer();
        else if (gameSelected == 2) games.resetSpace();
        else if (gameSelected == 3) games.resetFlappy();
        else if (gameSelected == 4) games.resetCatcher();
        else if (gameSelected == 5) games.resetJump();
        else if (gameSelected == 6) games.resetStacker();
        else if (gameSelected == 7) games.resetMemory();
        gamePlaying = true;
        audio.playSound(SOUND_POWERUP);
        Serial.printf("[BTN2] Started Game %d\n", gameSelected);
      } else {
        gamesActive = false;
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[BTN2] Exited Games Menu");
      }
    } else {
      // Game is playing: Button 2 action
      if (games.canExitActiveGame()) {
        gamePlaying = false;
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[BTN2] Exited active game back to Arcade menu");
      }
    }
    return;
  }

  if (currentScreen == SCREEN_SETTINGS && settingsActive) {
    if (menuOption == 6) { // SAVE SETTINGS
      adjustOption(6, 1);
    } else if (menuOption == 7) { // EXIT MENU
      adjustOption(7, 1);
    } else {
      optionSelected = !optionSelected;
      audio.playSound(SOUND_CHIRP);
      Serial.printf("[BTN2] Option selection toggled: %s\n", optionSelected ? "Selected" : "Deselected");
    }
    return;
  }

  if (currentScreen == SCREEN_NOTIFICATIONS && notificationsActive) {
    if (!notificationSelected) {
      int nextIdx = (face.getCurrentNotifViewIdx() + 1) % face.getNotificationCount();
      face.setCurrentNotifViewIdx(nextIdx);
      audio.playSound(SOUND_CHIRP);
      Serial.printf("[BTN2] Notifications DOWN -> index %d\n", nextIdx);
    }
    return;
  }

  // Cycles screens: Face -> QR Card -> Clock -> Notifications -> Calendar -> Games -> Settings -> Face
  SmartwatchScreen nextScreen;
  if (currentScreen == SCREEN_FACE) {
    nextScreen = SCREEN_CARD;
  } else if (currentScreen == SCREEN_CARD) {
    nextScreen = SCREEN_CLOCK;
  } else if (currentScreen == SCREEN_CLOCK) {
    nextScreen = SCREEN_NOTIFICATIONS;
  } else if (currentScreen == SCREEN_NOTIFICATIONS) {
    nextScreen = SCREEN_CALENDAR;
  } else if (currentScreen == SCREEN_CALENDAR) {
    nextScreen = SCREEN_GAMES;
  } else if (currentScreen == SCREEN_GAMES) {
    nextScreen = SCREEN_SETTINGS;
  } else {
    nextScreen = SCREEN_FACE;
  }

  currentScreen = nextScreen;
  settingsActive = false; 
  optionSelected = false;
  notificationsActive = false;
  notificationSelected = false;

  hardwareLoopbackActive = false;
  audio.micStreaming = false;
  audio.audioMode = LunaAudio::AUDIO_MODE_SYNTH;
  audio.prebuffering = true;
  face.setStateLabel("IDLE");
  audio.playSound(SOUND_COIN);
  notifyScreenAndExprSync();
  Serial.printf("[BTN2] Cycled screen to %d\n", currentScreen);
}

void handleBtn2Long() {
  lastInteractionTime = millis();
  if (mapsActive) return;

  if (currentScreen == SCREEN_SETTINGS && settingsActive) {
    if (menuOption == 6) {
      adjustOption(6, 1);
    } else if (menuOption == 7) {
      adjustOption(7, 1);
    } else {
      optionSelected = !optionSelected;
      audio.playSound(SOUND_CHIRP);
    }
    return;
  }

  // Return to Face screen on long press
  currentScreen = SCREEN_FACE;
  settingsActive = false;
  optionSelected = false;
  gamesActive = false;
  gamePlaying = false;
  notificationsActive = false;
  notificationSelected = false;
  audio.playSound(SOUND_STARTUP);
  notifyScreenAndExprSync();
  Serial.println("[BTN2 LONG] Switched to FACE screen");
}

void updateStateLabel() {
  if (currentScreen == SCREEN_FACE) {
    Expression expr = face.getExpression();
    if ((int)expr >= 0 && (int)expr < SPRITE_AI13_ANIMATION_COUNT) {
      face.setStateLabel(String(getSpriteAi13AnimName((int)expr)));
    } else if (expr == EXPR_ALL_GIF) {
      face.setStateLabel(String(getSpriteAi13AnimName(face.getGifIndex())));
    } else {
      face.setStateLabel("IDLE");
    }
  } else if (currentScreen == SCREEN_GAMES) {
    if (gamePlaying) {
      const char* gameNames[] = {
        "LUNA RACER", "LUNA SPACE", "FLAPPY MOCHY", "COIN CATCHER",
        "MOCHY JUMP", "STACKER", "MEMORY MATRIX"
      };
      if (gameSelected >= 1 && gameSelected <= 7) {
        face.setStateLabel(gameNames[gameSelected - 1]);
      } else {
        face.setStateLabel("ARCADE");
      }
    } else if (gamesActive) {
      face.setStateLabel("ARCADE MENU");
    } else {
      face.setStateLabel("ARCADE");
    }
  } else if (currentScreen == SCREEN_CLOCK) {
    face.setStateLabel("CLOCK");
  } else if (currentScreen == SCREEN_NOTIFICATIONS) {
    face.setStateLabel("NOTIFS");
  } else if (currentScreen == SCREEN_CALENDAR) {
    face.setStateLabel("CALENDAR");
  } else if (currentScreen == SCREEN_MAPS) {
    face.setStateLabel("MAPS");
  } else if (currentScreen == SCREEN_CARD) {
    face.setStateLabel("QR CARD");
  } else if (currentScreen == SCREEN_SETTINGS) {
    face.setStateLabel("SETTINGS");
  } else {
    face.setStateLabel("IDLE");
  }
}

void loop() {
  unsigned long now = millis();

  // ── 0. Poll unified button handler (immediate click on release, zero gesture delay) ──
  ButtonEvent btnEvt = interaction.update();
  if (btnEvt != BTN_NONE) {
    if (isAlarmRinging || isReminderRinging || face.isAlarmRingingActive()) {
      dismissAlarmRinging();
    } else {
      switch (btnEvt) {
        case BTN1_SINGLE: handleBtn1Single(); break;
        case BTN1_DOUBLE: handleBtn1Double(); break;
        case BTN1_LONG:   handleBtn1Long();   break;
        case BTN2_SINGLE: handleBtn2Single(); break;
        case BTN2_LONG:   handleBtn2Long();   break;
        default: break;
      }
    }
  }

  // 1. Maintain BLE stack status and connection advertisement
  ble.handleConnectionState();

  // 1.5. Check offline hardware scheduled alarms & calendar events
  checkHardwareScheduledAlarms();

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
      // Re-print all Sprite AI animation entries for debugging
      Serial.println("====== SPRITE AI ANIMATION DUMP ======");
      Serial.printf("Total Sprite Animations: %d (4 frames each)\n", SPRITE_AI13_ANIMATION_COUNT);
      for (int i = 0; i < SPRITE_AI13_ANIMATION_COUNT; i++) {
        Serial.printf("ANIM[%d] %s (4 frames)\n", i, getSpriteAi13AnimName(i));
      }
      Serial.println("======================================");
    } else {
      // Fallback: forward generic commands (e.g. MAP:, NOTIF:, EXPR:, AUDIO:) to the robot command handler
      handleRobotCommand(cmd);
    }
  }

  // Note: All navigation is now handled by the physical button event dispatcher
  // at the top of loop() via interaction.update() + handleBtn1/2 functions.


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
        int animIdx = defaultGif;
        if (animIdx >= 100) animIdx -= 100;
        if (animIdx < 0 || animIdx >= SPRITE_AI13_ANIMATION_COUNT) animIdx = 0;
        face.setGifIndex(animIdx);
        face.setExpression((Expression)animIdx);
      }
      lastExpressionCycleTime = now;
    }
  } else {
    // Regular operation expression cycling (only when on SCREEN_FACE screen)
    if (currentScreen == SCREEN_FACE && !isAsleep) {
      if (isCycleMode) {
        // Switch to next sprite animation every 8 seconds in cycle mode
        if (now - lastExpressionCycleTime >= 8000) {
          cycleExpression();
          lastExpressionCycleTime = now;
        }
      } else {
        // Return to default expression after notification duration
        if (!isReminderRinging && (now - lastExpressionCycleTime >= (unsigned long)activeNotificationDurationMs)) {
          face.headerText = ""; // Clear header overlay
          int animIdx = defaultGif;
          if (animIdx >= 100) animIdx -= 100;
          if (animIdx < 0 || animIdx >= SPRITE_AI13_ANIMATION_COUNT) animIdx = 0;
          face.setGifIndex(animIdx);
          face.setExpression((Expression)animIdx);
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
  // Note: SCREEN_CARD is excluded – QR must stay visible until user explicitly dismisses it
  if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_CARD && !inIntroPhase && !isAlarmRinging && !isReminderRinging && !mapsActive && !gamePlaying) {
    if (now - lastInteractionTime >= 15000) {
      currentScreen = SCREEN_FACE;
      lastExpressionCycleTime = now;
      Serial.println("Inactivity timeout: Returning to GIF expressions screen.");
    }
  }

  // Periodic battery read (every 1 second)
  static unsigned long lastBatteryReadTime = 0;
  if (now - lastBatteryReadTime > 1000) {
    lastBatteryReadTime = now;
    // Multiply by BATTERY_CALIBRATION_MULTIPLIER to get calibrated battery voltage.
    float rawVolts = (analogReadMilliVolts(BATTERY_PIN) * BATTERY_CALIBRATION_MULTIPLIER) / 1000.0f;
    // Apply low-pass Exponential Moving Average filter to smooth fluctuations
    if (batteryVolts == 3.82f) {
      batteryVolts = rawVolts; // first read override
    } else {
      batteryVolts = 0.9f * batteryVolts + 0.1f * rawVolts;
    }
  }

  // 5. Periodic status updates to BLE client
  if (bleActive && ble.isConnected() && (now - lastStatusUpdateTime > STATUS_UPDATE_INTERVAL)) {
    lastStatusUpdateTime = now;
    
    unsigned long uptimeSec = now / 1000;
    updateStateLabel();
    ble.updateStatus(uptimeSec, touchCount, batteryVolts, face.getExpression(), face.getStateLabel());
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
