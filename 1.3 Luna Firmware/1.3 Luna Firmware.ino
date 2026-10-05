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
void handleBtn1Single();
void handleBtn1Double();
void handleBtn1Long();
void handleBtn2Single();
void handleBtn2Double();
void handleBtn2Long();
void transitionToNextVideoWithFade();

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

bool virtualBtn1 = false;
bool virtualBtn2 = false;

// Software Real-Time Clock variables
int rtcHour = 12;
int rtcMinute = 0;
int rtcSecond = 0;
String rtcDay = "Mon";
String rtcDate = "12 Sep";
unsigned long lastRtcMillis = 0;
bool is12HourFormat = false;

// Pomodoro Timer State Variables
int pomoMode = 0;             // 0: 25m Focus Work, 1: 5m Short Break, 2: 15m Long Break
int pomoState = 0;            // 0: Stopped, 1: Running, 2: Paused, 3: Completed
int pomoRemainingSec = 1500;  // Default 25 minutes = 1500s
int pomoTotalSec = 1500;
int pomoCompletedSessions = 0;
unsigned long pomoLastTickMillis = 0;

void resetPomodoroTimer() {
  pomoState = 0;
  if (pomoMode == 0) pomoTotalSec = 1500;      // 25 mins
  else if (pomoMode == 1) pomoTotalSec = 300;  // 5 mins
  else if (pomoMode == 2) pomoTotalSec = 900;  // 15 mins
  pomoRemainingSec = pomoTotalSec;
}

void updatePomodoroTimer() {
  if (pomoState == 1) { // Running
    unsigned long now = millis();
    if (now - pomoLastTickMillis >= 1000) {
      pomoLastTickMillis = now;
      if (pomoRemainingSec > 0) {
        pomoRemainingSec--;
      }
      if (pomoRemainingSec == 0) {
        pomoState = 3; // Completed
        if (pomoMode == 0) {
          pomoCompletedSessions++;
        }
        audio.playSound(SOUND_POWERUP);
        if (pomoMode == 0) {
          face.setDetailedNotification("POMODORO", "Focus session complete! Take a break.", rtcHour, rtcMinute);
        } else {
          face.setDetailedNotification("POMODORO", "Break is over! Ready to focus?", rtcHour, rtcMinute);
        }
      }
    }
  }
}

// NVS Settings Persistence
Preferences preferences;
bool bleActive = true;
int gifSpeed = 165;
int gifIntroSpeed = 165;
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
    face.timeSynced = true;
    lastRtcMillis = millis(); // Align RTC base to right now
  }
}

// ── Luna Mood & Living Life Engine ──────────────────────────────────────────
enum LunaMood {
  MOOD_HAPPY = 0,
  MOOD_PLAYFUL,
  MOOD_CURIOUS,
  MOOD_SLEEPY,
  MOOD_HUNGRY,
  MOOD_FED
};

// ── Luna Meal-time Schedule ─────────────────────────────────────────────────
enum MealSlot {
  MEAL_NONE = 0,
  MEAL_BREAKFAST = 1, // 08:00 - 10:00
  MEAL_LUNCH = 2,     // 12:30 - 14:30
  MEAL_DINNER = 3     // 19:30 - 21:30
};

MealSlot lastMealFed = MEAL_NONE;
String lastFedDate = "";

// ── Luna Pet XP, Level, Feeding & Age Persistence ───────────────────────────
uint32_t lunaXP = 0;
int      lunaLevel = 1;
uint32_t lunaFeedCount = 0;
uint32_t lunaAgeDays = 1;
uint32_t lunaBirthDayOfYear = 0;
uint32_t lunaBirthYear = 0;

// Owner Personalization & Greeting
String ownerName = "Sasi";
String ownerDOB = "";
bool isGreeting = false;
unsigned long greetingEndTime = 0;

String getLunaEvolutionStage(int lvl) {
  if (lvl < 5) return "Baby Luna";
  if (lvl < 10) return "Mochi Child";
  if (lvl < 20) return "Cyber Teen";
  return "Omega Luna";
}

int calculateLunaLevel(uint32_t xp) {
  int lvl = 1 + (int)(xp / 100);
  if (lvl < 1) lvl = 1;
  return lvl;
}

void loadLunaPetStats() {
  preferences.begin("luna", false);
  lunaXP = preferences.getUInt("pet_xp", 0);
  lunaFeedCount = preferences.getUInt("pet_feeds", 0);
  lunaLevel = calculateLunaLevel(lunaXP);
  lunaBirthDayOfYear = preferences.getUInt("pet_bday", 0);
  lunaBirthYear = preferences.getUInt("pet_byear", 0);
  lunaAgeDays = preferences.getUInt("pet_age", 1);
  if (lunaAgeDays == 0) lunaAgeDays = 1;
  ownerName = preferences.getString("ownerName", "Sasi");
  ownerDOB = preferences.getString("ownerDOB", "");
  preferences.end();
  Serial.printf("[PET] Loaded: XP=%u, Level=%d, Feeds=%u, Age=%u days, Stage=%s\n",
                lunaXP, lunaLevel, lunaFeedCount, lunaAgeDays, getLunaEvolutionStage(lunaLevel).c_str());
}

void saveLunaPetStats() {
  preferences.begin("luna", false);
  preferences.putUInt("pet_xp", lunaXP);
  preferences.putUInt("pet_feeds", lunaFeedCount);
  preferences.putUInt("pet_age", lunaAgeDays);
  if (lunaBirthYear > 0) preferences.putUInt("pet_byear", lunaBirthYear);
  if (lunaBirthDayOfYear > 0) preferences.putUInt("pet_bday", lunaBirthDayOfYear);
  preferences.end();
}

void sendLunaStatsToBLE() {
  if (!ble.isConnected()) return;
  String stage = getLunaEvolutionStage(lunaLevel);
  String statsMsg = "LUNA_STATS:" + String(lunaXP) + ":" + String(lunaLevel) + ":" +
                    String(lunaFeedCount) + ":" + String(lunaAgeDays) + ":" + stage;
  ble.sendLog(statsMsg);
}

void playAnimationSound(int animIndex) {
  switch (animIndex) {
    case 0:  audio.playSound(SOUND_CHIRP); break;         // Happy Smile
    case 1:  audio.playSound(SOUND_POWERDOWN); break;     // Angry Face
    case 2:  audio.playSound(SOUND_JUMP); break;          // Confused
    case 3:  audio.playSound(SOUND_COIN); break;          // Playful Wink
    case 4:  audio.playSound(SOUND_THEMECHANGE); break;    // Sparkle Eye
    case 5:  audio.playSound(SOUND_POWERDOWN); break;     // Sleepy Zzz
    case 6:  audio.playSound(SOUND_CHIRP); break;         // Curious
    case 7:  audio.playSound(SOUND_COIN); break;          // Giggle
    case 8:  audio.playSound(SOUND_JUMP); break;          // Excited
    case 9:  audio.playSound(SOUND_POWERDOWN); break;     // Extreme Angry
    case 10: audio.playSound(SOUND_ALERT_BEEP); break;     // Crying
    case 11: audio.playSound(SOUND_POWERUP); break;       // Cheery
    default: audio.playSound(SOUND_CHIRP); break;
  }
}

const char* getAnimationThought(int idx) {
  switch (idx) {
    case 0: return "Feeling Happy!";
    case 1: return "Hmph!";
    case 2: return "Thinking...";
    case 3: return "Wanna play?";
    case 4: return "So shiny!";
    case 5: return "Zzz... sleepy";
    case 6: return "What's that?";
    case 7: return "Hehehe!";
    case 8: return "Yay let's go!";
    case 9: return "Grrrrr!!";
    case 10: return "Sob sob...";
    case 11: return "Feeling great!";
    default: return "";
  }
}

LunaMood currentMood = MOOD_HAPPY;
bool isHungry = false;
unsigned long lastFedTime = 0;
unsigned long moodStartTime = 0;
unsigned long moodDurationMs = 8000;
unsigned long lastHungerWhimperTime = 0;

void feedLuna();
void updateLunaLife();

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
    for (int i = 0; i < TOTAL_ANIMATIONS; i++) {
      if (label.equalsIgnoreCase(getSpriteAiAnimationName(i))) {
        foundIdx = i;
        break;
      }
    }
    if (foundIdx != -1) {
      face.getRobotEyeAnim().setAnimationIndex(foundIdx);
      face.getRobotEyeAnim().reset();
      face.getRobotEyeAnim().play();
      face.setExpression(EXPR_ROBOT_EYE);
      face.setStateLabel(String(getSpriteAiAnimationName(foundIdx)));
      audio.playSound(getAnimationSound(foundIdx));
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
    case SCREEN_POMODORO: screenName = "POMODORO"; break;
    default: screenName = "FACE"; break;
  }
  
  int animIndex = (int)face.getExpression();
  String label = face.getStateLabel();
  ble.sendLog("EXPR_SYNC:" + String(animIndex) + "|" + label);
  ble.sendLog("SCREEN_SYNC:" + screenName);
  
  unsigned long uptimeSec = millis() / 1000;
  ble.updateStatus(uptimeSec, touchCount, batteryVolts, (Expression)animIndex, label);
}

void feedLuna() {
  if (!isHungry && !face.isHungry()) return; // Only feed if hungry!

  // Add +50 XP and increment feed count
  lunaFeedCount++;
  lunaXP += 50;
  lunaLevel = calculateLunaLevel(lunaXP);
  saveLunaPetStats();
  sendLunaStatsToBLE();

  // Start feeding animation: bubbles float up towards crying face
  face.startFeeding();
  face.setThoughtText("+50 XP! Eating...");
  audio.playSound(SOUND_CHIRP);
  Serial.printf("[LUNA] FEEDING: XP now %u (Lv %d). Food bubbles floating up...\n", lunaXP, lunaLevel);
}

void updateLunaLife() {
  unsigned long now = millis();

  // Only run life/mood engine on SCREEN_FACE when awake and not in intro or maps/games
  if (currentScreen != SCREEN_FACE || isAsleep || inIntroPhase || mapsActive || gamePlaying) return;

  // 0. Active feeding in progress: bubbles floating up
  if (face.isFeedingActive()) {
    if (now - face.getFeedingStartTime() >= 2500) {
      // Finished feeding!
      face.stopFeeding();
      isHungry = false;
      face.setHungry(false);
      lastFedTime = now;

      // Mark current meal slot as fed
      int minuteOfDay = rtcHour * 60 + rtcMinute;
      if (minuteOfDay >= 8 * 60 && minuteOfDay < 10 * 60) {
        lastMealFed = MEAL_BREAKFAST;
      } else if (minuteOfDay >= 12 * 60 + 30 && minuteOfDay < 14 * 60 + 30) {
        lastMealFed = MEAL_LUNCH;
      } else if (minuteOfDay >= 19 * 60 + 30 && minuteOfDay < 21 * 60 + 30) {
        lastMealFed = MEAL_DINNER;
      }

      currentMood = MOOD_HAPPY;
      moodStartTime = now;
      moodDurationMs = 15000;

      // Switch to Happy Smile (index 0) after feeding!
      face.setExpression((Expression)0);
      face.setGifIndex(0);
      face.setStateLabel(String(getSpriteAi13AnimName(0)));
      face.setThoughtText("Full & Happy!");
      audio.playSound(SOUND_POWERUP);
      Serial.println(F("[LUNA] FED! Finished eating, Luna is now Happy!"));
      notifyScreenAndExprSync();
      sendLunaStatsToBLE();
    }
    return;
  }

  // 1. Personalized Greeting check ("Hi <User Name>!")
  if (isGreeting) {
    if (now < greetingEndTime) {
      face.setThoughtText("Hi " + ownerName + "!");
    } else {
      isGreeting = false;
      int curIdx = face.getGifIndex();
      face.setThoughtText(getAnimationThought(curIdx));
    }
  }

  // 2. Check hunger timer (Morning, Afternoon, Night meal windows)
  if (!isHungry) {
    // Reset daily meal tracker when calendar date changes
    if (lastFedDate.length() > 0 && rtcDate.length() > 0 && rtcDate != lastFedDate) {
      lastFedDate = rtcDate;
      lastMealFed = MEAL_NONE;
      lunaAgeDays++;
      saveLunaPetStats();
    }
    if (lastFedDate.length() == 0 && rtcDate.length() > 0) {
      lastFedDate = rtcDate;
    }

    MealSlot currentMealSlot = MEAL_NONE;
    int minuteOfDay = rtcHour * 60 + rtcMinute;

    // Breakfast: 08:00 - 10:00 (480 - 600 mins)
    if (minuteOfDay >= 8 * 60 && minuteOfDay < 10 * 60) {
      currentMealSlot = MEAL_BREAKFAST;
    // Lunch: 12:30 - 14:30 (750 - 870 mins)
    } else if (minuteOfDay >= 12 * 60 + 30 && minuteOfDay < 14 * 60 + 30) {
      currentMealSlot = MEAL_LUNCH;
    // Dinner: 19:30 - 21:30 (1170 - 1290 mins)
    } else if (minuteOfDay >= 19 * 60 + 30 && minuteOfDay < 21 * 60 + 30) {
      currentMealSlot = MEAL_DINNER;
    }

    bool shouldTriggerHunger = false;
    if (currentMealSlot != MEAL_NONE && lastMealFed != currentMealSlot) {
      shouldTriggerHunger = true;
    } else if (currentMealSlot == MEAL_NONE && (now - lastFedTime >= 14400000UL)) {
      // Fallback: 4 hours without food if outside meal slots or clock not set
      shouldTriggerHunger = true;
    }

    if (shouldTriggerHunger) {
      // Luna feels hungry!
      isHungry = true;
      face.setHungry(true);
      currentMood = MOOD_HUNGRY;
      face.setExpression((Expression)10); // 10 is Crying
      face.setGifIndex(10);
      face.setStateLabel(String(getSpriteAi13AnimName(10)));
      face.setThoughtText("HUNGRY");
      lastHungerWhimperTime = now;
      audio.playSound(SOUND_ALERT_BEEP);
      Serial.println(F("[LUNA] Meal time hunger triggered! Crying for food (press BTN to feed)."));
      notifyScreenAndExprSync();
    }
  }

  // 3. While hungry, stay crying and whimper periodically
  if (isHungry) {
    face.setThoughtText("HUNGRY");
    if (now - lastHungerWhimperTime >= 12000) {
      lastHungerWhimperTime = now;
      audio.playSound(SOUND_CHIRP);
    }
    return; // Do not cycle away to happy expressions while hungry!
  }

  // 4. Living mood engine: organic expression transitions based on mood
  if (now - moodStartTime >= moodDurationMs) {
    moodStartTime = now;
    moodDurationMs = random(12000, 22000); // 12 to 22 seconds between shifts

    int nextAnim = 0;
    switch (currentMood) {
      case MOOD_HAPPY: {
        const int happyPool[] = {0, 4, 7, 11}; // Happy Smile, Sparkle Eye, Giggle, Cheery
        nextAnim = happyPool[random(0, 4)];
        int r = random(0, 100);
        if (r < 30) currentMood = MOOD_PLAYFUL;
        else if (r < 50) currentMood = MOOD_CURIOUS;
        break;
      }
      case MOOD_PLAYFUL: {
        const int playfulPool[] = {3, 7, 8, 0}; // Playful Wink, Giggle, Excited, Happy Smile
        nextAnim = playfulPool[random(0, 4)];
        int r = random(0, 100);
        if (r < 35) currentMood = MOOD_HAPPY;
        else if (r < 55) currentMood = MOOD_CURIOUS;
        break;
      }
      case MOOD_CURIOUS: {
        const int curiousPool[] = {6, 2, 4, 0}; // Curious, Confused, Sparkle Eye, Happy Smile
        nextAnim = curiousPool[random(0, 4)];
        int r = random(0, 100);
        if (r < 40) currentMood = MOOD_HAPPY;
        else if (r < 60) currentMood = MOOD_PLAYFUL;
        break;
      }
      case MOOD_SLEEPY: {
        const int sleepyPool[] = {5, 2, 0}; // Sleepy Zzz, Confused, Happy Smile
        nextAnim = sleepyPool[random(0, 3)];
        if (random(0, 100) < 50) currentMood = MOOD_HAPPY;
        break;
      }
      default:
        nextAnim = 0;
        break;
    }

    face.setExpression((Expression)nextAnim);
    face.setGifIndex(nextAnim);
    face.setStateLabel(String(getSpriteAi13AnimName(nextAnim)));
    if (!isGreeting) {
      face.setThoughtText(getAnimationThought(nextAnim));
    }
    playAnimationSound(nextAnim);
    notifyScreenAndExprSync();
  }
}

void handleRobotCommand(String text) {
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on interaction
  
  if (mapsActive && !text.startsWith("MAP") && !text.startsWith("SCREEN:") && !text.startsWith("CALL:") && !text.startsWith("TIME:") && !text.startsWith("FOCUS") && !text.startsWith("APPLIMIT")) {
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
  } else if (text == "TOUCH_SIM:TAP") {
    handleBtn1Single();
    notifyScreenAndExprSync();
  } else if (text == "TOUCH_SIM:DOUBLE") {
    handleBtn1Double();
    notifyScreenAndExprSync();
  } else if (text == "TOUCH_SIM:LONG") {
    handleBtn1Long();
    notifyScreenAndExprSync();
  } else if (text == "TOUCH_SIM:NEXT") {
    handleBtn2Single();
    notifyScreenAndExprSync();
  } else if (text == "TOUCH_SIM:PREV") {
    handleBtn2Double();
    notifyScreenAndExprSync();
  } else if (text == "ANGRY" || text == "ANIM:ANGRY" || text == "EXPR_ANGRY") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(1);
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Angry Face");
    face.setThoughtText("Hmph!");
    audio.playSound(SOUND_ANIM_ANGRY);
    notifyScreenAndExprSync();
    Serial.println("OK:AngryAnimationTriggered");
  } else if (text == "FEED" || text == "EAT" || text == "FEED:FISH") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(4); // Eat Fish
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Eat Fish");
    audio.playSound(SOUND_ANIM_EAT_FISH);
    feedLuna();
    Serial.println("OK:LunaFedFish");
  } else if (text == "FEED:MILK") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(5); // Drink Milk
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Drink Milk");
    audio.playSound(SOUND_ANIM_DRINK_MILK);
    feedLuna();
    Serial.println("OK:LunaFedMilk");
  } else if (text == "FEED:SALAD") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(6); // Eat Salad
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Eat Salad");
    audio.playSound(SOUND_ANIM_EAT_SALAD);
    feedLuna();
    Serial.println("OK:LunaFedSalad");
  } else if (text == "SICK") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(8); // Luna Sick
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Luna Sick");
    audio.playSound(SOUND_ANIM_SICK);
    notifyScreenAndExprSync();
  } else if (text == "CURE" || text == "RECOVER") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(9); // Recovered
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Recovered");
    audio.playSound(SOUND_ANIM_RECOVERED);
    notifyScreenAndExprSync();
  } else if (text == "THINK") {
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(13); // Luna Thinking
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Luna Thinking");
    audio.playSound(SOUND_ANIM_THINKING);
    notifyScreenAndExprSync();
  } else if (text == "GET_STATS" || text == "STATS") {
    sendLunaStatsToBLE();
    Serial.println("OK:StatsSent");
  } else if (text == "HUNGRY") {
    currentScreen = SCREEN_FACE;
    isHungry = true;
    face.setHungry(true);
    currentMood = MOOD_HUNGRY;
    face.getRobotEyeAnim().setAnimationIndex(3); // Getting Hungry
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Getting Hungry");
    face.setThoughtText("HUNGRY");
    audio.playSound(SOUND_ALERT_BEEP);
    notifyScreenAndExprSync();
    Serial.println("OK:LunaHungryTriggered");
  } else if (text.startsWith("SET_NAME:") || text.startsWith("USER:")) {
    ownerName = text.substring(text.indexOf(':') + 1);
    ownerName.trim();
    if (ownerName.length() == 0) ownerName = "Sasi";
    preferences.begin("luna", false);
    preferences.putString("ownerName", ownerName);
    preferences.end();
    isGreeting = true;
    greetingEndTime = millis() + 4000;
    face.setThoughtText("Hi " + ownerName + "!");
    Serial.printf("[USER] Updated owner name: %s\n", ownerName.c_str());
  } else if (text.startsWith("SET_DOB:") || text.startsWith("DOB:")) {
    ownerDOB = text.substring(text.indexOf(':') + 1);
    ownerDOB.trim();
    preferences.begin("luna", false);
    preferences.putString("ownerDOB", ownerDOB);
    preferences.end();
    Serial.printf("[USER] Updated owner DOB: %s\n", ownerDOB.c_str());
  } else if (text.startsWith("FOCUS_ALERT:") || text.startsWith("APPLIMIT:")) {
    String payload = text.substring(text.indexOf(':') + 1);
    String appName = "App";
    String duration = "";
    int colon = payload.indexOf(':');
    if (colon > 0) {
      appName = payload.substring(0, colon);
      duration = payload.substring(colon + 1);
    } else {
      appName = payload;
    }
    appName.trim();
    duration.trim();
    if (appName.length() == 0) appName = "Phone";

    isAsleep = false;
    lastInteractionTime = millis();
    lastExpressionCycleTime = millis();
    face.setPopupDismiss();
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(1); // Angry Face
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Angry Face");
    face.setThoughtText("Focus on work!");
    audio.playSound(SOUND_ALERT_BEEP);
    ble.sendLog("FOCUS_ALERT_TRIGGERED:" + appName);
    notifyScreenAndExprSync();
    Serial.printf("[FOCUS_ALERT] Triggered for app: %s (%s)\n", appName.c_str(), duration.c_str());
  } else if (text == "FOCUS_TEST") {
    isAsleep = false;
    lastInteractionTime = millis();
    lastExpressionCycleTime = millis();
    face.setPopupDismiss();
    currentScreen = SCREEN_FACE;
    face.getRobotEyeAnim().setAnimationIndex(1);
    face.getRobotEyeAnim().reset();
    face.getRobotEyeAnim().play();
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel("Angry Face");
    face.setThoughtText("Focus on work!");
    audio.playSound(SOUND_ALERT_BEEP);
    ble.sendLog("FOCUS_ALERT_TRIGGERED:Test");
    notifyScreenAndExprSync();
    Serial.println("[FOCUS_ALERT] Test triggered via BLE");
  } else if (text.startsWith("ANIM:") || text.startsWith("SPRITE:")) {
    int colon = text.indexOf(':');
    int idx = text.substring(colon + 1).toInt();
    if (idx >= 0 && idx < TOTAL_ANIMATIONS) {
      currentScreen = SCREEN_FACE;
      face.getRobotEyeAnim().setAnimationIndex(idx);
      face.getRobotEyeAnim().reset();
      face.getRobotEyeAnim().play();
      face.setExpression(EXPR_ROBOT_EYE);
      face.setStateLabel(String(getSpriteAiAnimationName(idx)));
      audio.playSound(getAnimationSound(idx));
      notifyScreenAndExprSync();
      Serial.printf("OK:AnimIndexSet:%d\n", idx);
    }
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
    else if (arg == "POMODORO" || arg == "POMO") sVal = SCREEN_POMODORO;
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
  } else if (text.startsWith("CALL:RING:") || text.startsWith("CALL:RING")) {
    String caller = "Incoming Call";
    if (text.startsWith("CALL:RING:")) {
      caller = text.substring(10);
      caller.trim();
      if (caller.length() == 0) caller = "Incoming Call";
    }
    isAsleep = false;
    lastInteractionTime = millis();
    face.setCallRinging(true, caller);
    audio.playSound(SOUND_CHIRP);
    Serial.printf("[CALL] Ringing alert for: %s\n", caller.c_str());
  } else if (text == "CALL:END" || text.startsWith("CALL:END")) {
    face.dismissIncomingCall();
    Serial.println("[CALL] Ended");
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
  if (w == targetW && h == targetH) {
    tft.drawRGBBitmap(x, y, bitmap, w, h);
    return;
  }
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
    if (animIdx < 0 || animIdx >= TOTAL_ANIMATIONS) animIdx = 0;
    face.setGifIndex(animIdx);
    face.setDefaultExpression(EXPR_ROBOT_EYE);
    face.setExpression(EXPR_ROBOT_EYE);
    face.setStateLabel(String(getSpriteAiAnimationName(animIdx)));
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
    if (animIdx < 0 || animIdx >= TOTAL_ANIMATIONS) animIdx = 0;
    face.setGifIndex(animIdx);
    face.setDefaultExpression(EXPR_ROBOT_EYE);
    face.setExpression(EXPR_ROBOT_EYE);
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
  tft.setSPISpeed(40000000UL); // 40 MHz SPI speed for ultra-smooth video & UI
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

  // Play cinematic startup chime
  audio.playSound(SOUND_BOOT_CHIME);

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
  if (introIdx < 0 || introIdx >= TOTAL_ANIMATIONS) introIdx = 0;
  face.setGifIndex(introIdx);
  face.setExpression(EXPR_ROBOT_EYE);

  lastInteractionTime = millis();
  lastRtcMillis = millis();
  lastExpressionCycleTime = millis();

  // Dump all loaded Video AI animation names to serial for debugging
  Serial.println("====== VIDEO AI ANIMATION DUMP ======");
  Serial.printf("Total Video Animations: %d\n", TOTAL_ANIMATIONS);
  for (int i = 0; i < TOTAL_ANIMATIONS; i++) {
    Serial.printf("ANIM[%d] %s (%d frames)\n", i, getSpriteAiAnimationName(i), anim_frame_counts[i]);
  }
  Serial.println("======================================");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
  games.begin();
  qrCard.begin();  // Load persisted business card URL from NVS
  network.init();
  loadLunaPetStats();
  resetPomodoroTimer();
}

// ── Smooth Fade Transition Between Video Animations (Cinematic Dip) ──────
void transitionToNextVideoWithFade() {
  if (isAsleep || mapsActive || gamePlaying) return;
  if (currentScreen != SCREEN_FACE) {
    currentScreen = SCREEN_FACE;
  }

  uint8_t targetDuty = 255;
  if (oledBrightness == 1) targetDuty = 40;
  else if (oledBrightness == 2) targetDuty = 140;
  else targetDuty = 255;

  // 1. Smooth Fade-Out (70ms) - Cinematic dip to black
  const unsigned long FADE_MS = 70;
  unsigned long t0 = millis();
  while (millis() - t0 < FADE_MS) {
    float t = (float)(millis() - t0) / (float)FADE_MS;
    if (t > 1.0f) t = 1.0f;
    float ease = 1.0f - (t * t * (3.0f - 2.0f * t)); // smooth cubic ease-out
    uint8_t d = (uint8_t)(ease * targetDuty);
    #ifdef TFT_BLK
    analogWrite(TFT_BLK, d);
    #endif
    audio.update();
    interaction.update();
    delay(2);
  }
  #ifdef TFT_BLK
  analogWrite(TFT_BLK, 0);
  #endif

  // 2. Advance to next video animation while screen is black
  face.getRobotEyeAnim().nextAnimation();
  int newIdx = face.getRobotEyeAnim().getAnimationIndex();
  const char* animName = getSpriteAiAnimationName(newIdx);
  face.setStateLabel(String(animName));
  face.setGifIndex(newIdx);
  face.setExpression(EXPR_ROBOT_EYE);

  // Render Frame 0 of the new animation into display buffer & TFT while black
  face.getRobotEyeAnim().reset();
  face.getRobotEyeAnim().play();
  face.clearGifFinished();
  face.update();
  face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);

  // 3. Play the matching animation sound effect while the screen fades back in
  audio.playSound(getAnimationSound(newIdx));

  // 4. Smooth Fade-In (70ms) - Restores full brightness seamlessly
  t0 = millis();
  while (millis() - t0 < FADE_MS) {
    float t = (float)(millis() - t0) / (float)FADE_MS;
    if (t > 1.0f) t = 1.0f;
    float ease = t * t * (3.0f - 2.0f * t); // smooth cubic ease-in
    uint8_t d = (uint8_t)(ease * targetDuty);
    #ifdef TFT_BLK
    analogWrite(TFT_BLK, d);
    #endif
    audio.update();
    interaction.update();
    delay(2);
  }
  #ifdef TFT_BLK
  analogWrite(TFT_BLK, targetDuty);
  #endif

  // 5. Synchronize animation playback timer immediately after fade:
  face.getRobotEyeAnim().reset();
  face.getRobotEyeAnim().play();

  Serial.printf("[VIDEO] Anim -> %s (%d frames)\n",
                animName, face.getRobotEyeAnim().getFrameCount());
  notifyScreenAndExprSync();
}

// Global index for video-ai cycling — advances through all 14 animations with fade
void cycleExpression() {
  transitionToNextVideoWithFade();
}

String getExpressionName(int expr) {
  if (expr >= 100) expr -= 100;
  if (expr >= 0 && expr < TOTAL_ANIMATIONS) {
    return String(getSpriteAiAnimationName(expr));
  }
  return "Luna Idle";
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
      Serial.println("Skipped to next animation");
    } else if (actionType == 3) {
      // BLE is always ON, do not toggle
      bleActive = true;
      audio.playSound(SOUND_CHIRP);
      Serial.println("BLE Toggle touch action ignored (BLE is always ON)");
    } else if (actionType >= 20) {
      // Specific Video AI anim index: actionType = 20 + animIdx
      int animIdx = actionType - 20;
      if (animIdx >= 0 && animIdx < TOTAL_ANIMATIONS) {
        face.setGifIndex(animIdx);
        face.setExpression(EXPR_ROBOT_EYE);
        audio.playSound(getAnimationSound(animIdx));
        Serial.printf("Triggered specific Anim #%d: %s\n", animIdx, getSpriteAiAnimationName(animIdx));
      }
    } else if (actionType >= 10 && actionType < 10 + TOTAL_ANIMATIONS) {
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
    if (isHungry || face.isHungry()) {
      feedLuna();
      return;
    }
    // Next expression/animation
    cycleExpression();
    Serial.println("[BTN1] Next expression");
  } else if (currentScreen == SCREEN_POMODORO) {
    if (pomoState == 0 || pomoState == 2) {
      pomoState = 1; // Start / Resume
      pomoLastTickMillis = millis();
      audio.playSound(SOUND_COIN);
      Serial.println("[BTN1] Pomodoro timer started/resumed");
    } else if (pomoState == 1) {
      pomoState = 2; // Pause
      audio.playSound(SOUND_CHIRP);
      Serial.println("[BTN1] Pomodoro timer paused");
    } else if (pomoState == 3) {
      resetPomodoroTimer();
      audio.playSound(SOUND_POWERUP);
      Serial.println("[BTN1] Pomodoro timer reset after completion");
    }
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

  if (currentScreen == SCREEN_POMODORO) {
    pomoMode = (pomoMode + 1) % 3;
    resetPomodoroTimer();
    audio.playSound(SOUND_CHIRP);
    Serial.println("[BTN1 DBL] Cycled Pomodoro mode");
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

  if (currentScreen == SCREEN_POMODORO) {
    resetPomodoroTimer();
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("[BTN1 LONG] Reset Pomodoro timer");
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

  // Screen cycle: Clock (Home) -> Notifications -> Calendar -> Maps -> Focus (Pomodoro) -> Games -> Settings -> Card -> Face
  static const SmartwatchScreen CYCLE[] = {
    SCREEN_CLOCK, SCREEN_NOTIFICATIONS, SCREEN_CALENDAR, SCREEN_MAPS, SCREEN_POMODORO,
    SCREEN_GAMES, SCREEN_SETTINGS, SCREEN_CARD, SCREEN_FACE
  };
  static const int CYCLE_LEN = 9;

  int idx = 0;
  for (int i = 0; i < CYCLE_LEN; i++) {
    if (CYCLE[i] == currentScreen) { idx = i; break; }
  }
  do {
    idx = (idx + 1) % CYCLE_LEN;
  } while (!mapsActive && CYCLE[idx] == SCREEN_MAPS);
  currentScreen = CYCLE[idx];

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

void handleBtn2Double() {
  lastInteractionTime = millis();
  if (currentScreen == SCREEN_MAPS || mapsActive) {
    mapsActive = false;
  }

  static const SmartwatchScreen CYCLE[] = {
    SCREEN_CLOCK, SCREEN_NOTIFICATIONS, SCREEN_CALENDAR, SCREEN_MAPS, SCREEN_POMODORO,
    SCREEN_GAMES, SCREEN_SETTINGS, SCREEN_CARD, SCREEN_FACE
  };
  static const int CYCLE_LEN = 9;

  int idx = 0;
  for (int i = 0; i < CYCLE_LEN; i++) {
    if (CYCLE[i] == currentScreen) { idx = i; break; }
  }
  do {
    idx = (idx - 1 + CYCLE_LEN) % CYCLE_LEN;
  } while (!mapsActive && CYCLE[idx] == SCREEN_MAPS);
  currentScreen = CYCLE[idx];

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
  Serial.printf("[BTN2 DBL] Cycled screen BACKWARDS to %d\n", currentScreen);
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
    face.setStateLabel(String(getSpriteAiAnimationName(face.getRobotEyeAnim().getAnimationIndex())));
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
  } else if (currentScreen == SCREEN_POMODORO) {
    face.setStateLabel("FOCUS");
  } else {
    face.setStateLabel("IDLE");
  }
}

void loop() {
  unsigned long now = millis();

  // ── 0. Poll unified button handler (immediate click on release, zero gesture delay) ──
  ButtonEvent btnEvt = interaction.update();
  if (btnEvt != BTN_NONE) {
    if (face.isCallRingingActive()) {
      if (btnEvt == BTN1_SINGLE) {
        face.muteIncomingCall();
        ble.sendLog("CALL_ACT:MUTE");
        audio.playSound(SOUND_CHIRP);
        Serial.println("[CALL] User pressed BTN1 (MUTE)");
      } else if (btnEvt == BTN2_SINGLE) {
        face.dismissIncomingCall();
        ble.sendLog("CALL_ACT:REJECT");
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[CALL] User pressed BTN2 (CUT)");
      }
    } else if (face.isQuickReplyActive()) {
      if (btnEvt == BTN1_SINGLE) {
        face.cycleQuickReplyPreset();
        audio.playSound(SOUND_CHIRP);
      } else if (btnEvt == BTN2_SINGLE) {
        int qIdx = face.getQuickReplySelectedIdx();
        String reply = face.getQuickReplyPreset(qIdx);
        ble.sendLog("REPLY:WA:" + reply);
        face.closeQuickReply();
        notificationSelected = false;
        audio.playSound(SOUND_CHIRP);
        Serial.printf("[WA] Quick reply sent: %s\n", reply.c_str());
      } else if (btnEvt == BTN1_LONG || btnEvt == BTN2_LONG) {
        face.closeQuickReply();
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[WA] Quick reply cancelled");
      }
    } else if (face.isPopupActive()) {
      // If WhatsApp popup is showing, BTN1 opens Quick Reply, BTN2 dismisses popup
      if (face.getPopupTitle().indexOf("WhatsApp") >= 0 || face.getPopupTitle().startsWith("WA:") ||
          face.getPopupBody().indexOf("WhatsApp") >= 0) {
        if (btnEvt == BTN1_SINGLE) {
          face.openQuickReply();
          face.setPopupDismiss();
          audio.playSound(SOUND_CHIRP);
        } else {
          face.setPopupDismiss();
          audio.playSound(SOUND_CHIRP);
        }
      } else {
        face.setPopupDismiss();
        audio.playSound(SOUND_CHIRP);
      }
    } else if (isAlarmRinging || isReminderRinging || face.isAlarmRingingActive()) {
      dismissAlarmRinging();
    } else {
      switch (btnEvt) {
        case BTN1_SINGLE: handleBtn1Single(); break;
        case BTN1_DOUBLE: handleBtn1Double(); break;
        case BTN1_LONG:   handleBtn1Long();   break;
        case BTN2_SINGLE: handleBtn2Single(); break;
        case BTN2_DOUBLE: handleBtn2Double(); break;
        case BTN2_LONG:   handleBtn2Long();   break;
        default: break;
      }
    }
  }

  // Periodic ring sound when incoming call is ringing (if not muted)
  static unsigned long lastCallRingBeep = 0;
  if (face.isCallRingingActive()) {
    if (!face.isCallMuted() && millis() - lastCallRingBeep >= 2000) {
      lastCallRingBeep = millis();
      audio.playSound(SOUND_CHIRP);
    }
  }

  // 1. Maintain BLE stack status and connection advertisement
  ble.handleConnectionState();

  // 1.5. Check offline hardware scheduled alarms & calendar events + Pomodoro timer
  static unsigned long lastAlarmCheckMs = 0;
  if (now - lastAlarmCheckMs >= 1000) {
    lastAlarmCheckMs = now;
    checkHardwareScheduledAlarms();
    updatePomodoroTimer();
  }

  // Update Luna living mood engine and feeding animation
  updateLunaLife();

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
      // Re-print all Video AI animation entries for debugging
      Serial.println("====== VIDEO AI ANIMATION DUMP ======");
      Serial.printf("Total Video Animations: %d\n", TOTAL_ANIMATIONS);
      for (int i = 0; i < TOTAL_ANIMATIONS; i++) {
        Serial.printf("ANIM[%d] %s (%d frames)\n", i, getSpriteAiAnimationName(i), anim_frame_counts[i]);
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
    if (face.getRobotEyeAnim().isCycleCompleted()) {
      inIntroPhase = false;
      face.setFrameDelay(gifSpeed);
      if (isCycleMode) {
        transitionToNextVideoWithFade();
      } else {
        int animIdx = defaultGif;
        if (animIdx >= 100) animIdx -= 100;
        if (animIdx < 0 || animIdx >= TOTAL_ANIMATIONS) animIdx = 0;
        face.setGifIndex(animIdx);
        face.setExpression(EXPR_ROBOT_EYE);
      }
      lastExpressionCycleTime = now;
    }
  } else {
    // Regular operation expression cycling (only when on SCREEN_FACE screen)
    if (currentScreen == SCREEN_FACE && !isAsleep) {
      if (isCycleMode) {
        // Auto-advance to next video with fade when the current animation finishes its single full cycle:
        if (face.getRobotEyeAnim().isCycleCompleted()) {
          transitionToNextVideoWithFade();
          lastExpressionCycleTime = now;
        }
      } else {
        // Return to default expression after notification duration
        if (!isReminderRinging && (now - lastExpressionCycleTime >= (unsigned long)activeNotificationDurationMs)) {
          face.headerText = ""; // Clear header overlay
          int animIdx = defaultGif;
          if (animIdx >= 100) animIdx -= 100;
          if (animIdx < 0 || animIdx >= TOTAL_ANIMATIONS) animIdx = 0;
          face.setGifIndex(animIdx);
          face.setExpression(EXPR_ROBOT_EYE);
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
  bool frameChanged = face.update();

  static int lastDrawnSecond = -1;
  static unsigned long lastDisplayDrawTime = 0;

  if (currentScreen == SCREEN_FACE) {
    // Ultra-smooth event-driven video rendering: draw immediately and only when a frame actually advances!
    if (frameChanged) {
      face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);
    }
  } else {
    // Other smartwatch screens (games, menus, clock) draw at standard ~30fps rate
    if (now - lastDisplayDrawTime >= 33) {
      lastDisplayDrawTime = now;
      lastDrawnSecond = rtcSecond;
      face.setConnectivityStatus(ble.isConnected(), network.isWifiConnected());
      face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);
    }
  }
}
