#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Preferences.h>

#include "config.h"
#include "rtc.h"
#include "expressions.h"
#include "audio.h"
#include "bluetooth.h"
#include "interaction.h"
#include "games.h"
#include "qr_card.h"
#include "imu.h"

// Forward declaration for BLE command handler
void handleRobotCommand(String cmd);

// Hardware Interface Objects
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 display(SCREEN_WIDTH, SCREEN_HEIGHT);
LunaFace face(tft, display);
LunaAudio audio;
LunaBLE ble;
LunaInteraction interaction;   // touch screen interface
LunaIMU imu;                   // QMI8658 Accelerometer & Gyroscope
LunaRTC rtcDevice;             // PCF85063 Hardware RTC

float readBatteryVolts() {
  uint32_t totalMv = 0;
  for (int i = 0; i < 10; i++) {
    totalMv += analogReadMilliVolts(BATTERY_PIN);
    delayMicroseconds(200);
  }
  float avgMv = totalMv / 10.0f;
  return (avgMv * BATTERY_CALIBRATION_MULTIPLIER) / 1000.0f;
}


bool virtualBtn1 = false;
bool virtualBtn2 = false;
bool calibrateRequest = false;

// NVS Settings Persistence
Preferences preferences;
bool bleActive = true;
int gifSpeed = 169;
int gifIntroSpeed = 169;
int introSoundSpeed = 80;
int defaultGif = 20;      // default GIF expression (20 = EXPR_ROBOT_EYE)
bool isCycleMode = false;
int gifIntro = 20;        // intro GIF expression (20 = EXPR_ROBOT_EYE)
// Gestures and touch inputs completely removed
bool negativeDisplay = false; // SSD1306 display color inversion
bool silentMode = false;
int clockStyle = 3; // clock style selector (0 to 3) - Default to 3 for Custom UI Designer
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

// Real-Time Clock variables
extern int rtcHour;
extern int rtcMinute;

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


// System State Variables
unsigned int touchCount = 0;
float batteryVolts = 3.82f;
unsigned long lastInteractionTime = 0;
unsigned long lastScreenTransitionTime = 0;
const unsigned long SLEEP_TIMEOUT = 45000; // 45 seconds of inactivity -> sleep
bool isAsleep = false;
bool inIntroPhase = false;
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
  char type[12];   // "alarm","birthday","reminder" etc
  char time[6];    // "HH:MM"
  char title[32];  // event title
  bool active;
};
NVSEventItem nvsEvents[5];
int nvsEventCount = 0;
int lastTriggeredAlarmMinute = -1;

void saveCalendarEventToNVS(const String& type, const String& time, const String& title) {
  for (int i = 4; i > 0; i--) {
    nvsEvents[i] = nvsEvents[i - 1];
  }
  strncpy(nvsEvents[0].type,  type.c_str(),  sizeof(nvsEvents[0].type)  - 1); nvsEvents[0].type[sizeof(nvsEvents[0].type)-1]   = 0;
  strncpy(nvsEvents[0].time,  time.c_str(),  sizeof(nvsEvents[0].time)  - 1); nvsEvents[0].time[sizeof(nvsEvents[0].time)-1]   = 0;
  strncpy(nvsEvents[0].title, title.c_str(), sizeof(nvsEvents[0].title) - 1); nvsEvents[0].title[sizeof(nvsEvents[0].title)-1] = 0;
  nvsEvents[0].active = true;
  if (nvsEventCount < 5) nvsEventCount++;

  char key[8];
  char val[56]; // type(12)+|+time(6)+|+title(32)+null
  preferences.begin("luna", false);
  preferences.putInt("cal_cnt", nvsEventCount);
  for (int i = 0; i < nvsEventCount; i++) {
    snprintf(key, sizeof(key), "cal_%d", i);
    snprintf(val, sizeof(val), "%s|%s|%s", nvsEvents[i].type, nvsEvents[i].time, nvsEvents[i].title);
    preferences.putString(key, val);
  }
  preferences.end();
}

void loadCalendarEventsFromNVS() {
  char key[8];
  char val[56];
  preferences.begin("luna", false);
  robotVariant = preferences.getString("robot_var", "ms_luna");
  nvsEventCount = preferences.getInt("cal_cnt", 0);
  if (nvsEventCount > 5) nvsEventCount = 5;
  for (int i = 0; i < nvsEventCount; i++) {
    snprintf(key, sizeof(key), "cal_%d", i);
    String s = preferences.getString(key, "");
    if (s.length() > 0) {
      int sep1 = s.indexOf('|');
      int sep2 = s.indexOf('|', sep1 + 1);
      if (sep1 > 0 && sep2 > sep1) {
        strncpy(nvsEvents[i].type,  s.substring(0, sep1).c_str(),        sizeof(nvsEvents[i].type)  - 1); nvsEvents[i].type[sizeof(nvsEvents[i].type)-1]   = 0;
        strncpy(nvsEvents[i].time,  s.substring(sep1+1, sep2).c_str(),   sizeof(nvsEvents[i].time)  - 1); nvsEvents[i].time[sizeof(nvsEvents[i].time)-1]   = 0;
        strncpy(nvsEvents[i].title, s.substring(sep2+1).c_str(),         sizeof(nvsEvents[i].title) - 1); nvsEvents[i].title[sizeof(nvsEvents[i].title)-1] = 0;
        nvsEvents[i].active = true;
        face.addCalendarEvent(nvsEvents[i].type, nvsEvents[i].time, nvsEvents[i].title);
      }
    }
  }
  preferences.end();
}

// Software Real-Time Clock variables
int rtcHour = 0;
int rtcMinute = 0;
int rtcSecond = 0;
String rtcDay = "Mon";
String rtcDate = "01 Jan";
unsigned long lastRtcMillis = 0;
bool is12HourFormat = false;
bool rtcEverSynced = false; // true once a valid TIME: sync has been received

void checkHardwareScheduledAlarms() {
  if (rtcSecond == 0 && rtcMinute != lastTriggeredAlarmMinute) {
    char currentHHMM[6];
    snprintf(currentHHMM, sizeof(currentHHMM), "%02d:%02d", rtcHour, rtcMinute);

    for (int i = 0; i < nvsEventCount; i++) {
      if (nvsEvents[i].active && strcmp(nvsEvents[i].time, currentHHMM) == 0) {
        lastTriggeredAlarmMinute = rtcMinute;
        isReminderRinging = true;
        lastReminderSoundTime = millis();
        char formattedType[12];
        strncpy(formattedType, nvsEvents[i].type, sizeof(formattedType) - 1);
        formattedType[sizeof(formattedType)-1] = 0;
        if (formattedType[0]) formattedType[0] = toupper(formattedType[0]);
        face.setDetailedNotification(formattedType, nvsEvents[i].title, rtcHour, rtcMinute);
        if (strcmp(nvsEvents[i].type, "birthday") == 0 || strcmp(nvsEvents[i].type, "alarm") == 0) {
          audio.playSound(SOUND_POWERUP);
        } else {
          audio.playSound(SOUND_COIN);
        }
        Serial.print(F("Alarm: ")); Serial.print(nvsEvents[i].title); Serial.print(F(" @ ")); Serial.println(currentHHMM);
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


void saveTimeToNVS(int h, int m, int s, const String& day, const String& date) {
  preferences.begin("luna", false);
  preferences.putBool("rtcSynced", true); // Mark that a real sync has happened
  preferences.putInt("rtcH",  h);
  preferences.putInt("rtcM",  m);
  preferences.putInt("rtcS",  s);
  preferences.putString("rtcDay",  day);
  preferences.putString("rtcDate", date);
  preferences.end();
}

void parseAndSyncTime(String timeStr) {
  int firstColon = timeStr.indexOf(':');
  int secondColon = timeStr.lastIndexOf(':');
  if (firstColon > 0 && secondColon > firstColon) {
    int h = timeStr.substring(0, firstColon).toInt();
    int m = timeStr.substring(firstColon + 1, secondColon).toInt();
    int s = 0;
    String day  = rtcDay;
    String date = rtcDate;

    int commaIdx = timeStr.indexOf(',', secondColon);
    if (commaIdx > 0) {
      s = timeStr.substring(secondColon + 1, commaIdx).toInt();
      int secondCommaIdx = timeStr.indexOf(',', commaIdx + 1);
      if (secondCommaIdx > 0) {
        day  = timeStr.substring(commaIdx + 1, secondCommaIdx);
        date = timeStr.substring(secondCommaIdx + 1);
        day.trim();
        date.trim();
      }
    } else {
      s = timeStr.substring(secondColon + 1).toInt();
    }

    // Basic sanity check before accepting
    if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
      Serial.println(F("[TIME] Invalid time values — sync rejected"));
      return;
    }

    rtcHour   = h;
    rtcMinute = m;
    rtcSecond = s;
    rtcDay    = day;
    rtcDate   = date;
    rtcEverSynced = true;
    face.timeSynced = true; // Unlock clock display

    lastRtcMillis = millis(); // Align software fallback base to right now

    // Write to hardware RTC (uses STOP bit protocol internally)
    rtcDevice.setTime(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate);

    // Persist to NVS so we survive reboots without BLE
    saveTimeToNVS(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate);

    Serial.printf("[TIME] Synced: %02d:%02d:%02d %s %s\n",
                  rtcHour, rtcMinute, rtcSecond, rtcDay.c_str(), rtcDate.c_str());
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
const unsigned long STATUS_UPDATE_INTERVAL = 3000; // 3 seconds (was 1s — 3x reduction in BLE TX duty cycle)

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
      case EXPR_ROBOT_EYE: face.setStateLabel("ROBOT_EYE"); break;
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
      audio.playSound(SOUND_POWERDOWN);
      break;
    case EXPR_SURPRISED:
      audio.playSound(SOUND_CHIRP);
      break;
    case EXPR_SLEEPING:
      isAsleep = true;
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

void handleRobotCommand(String text) {
  lastInteractionTime = millis();
  lastExpressionCycleTime = millis(); // Reset cycle timer on interaction
  
  if (mapsActive && !text.startsWith("MAP")) {
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
    isAsleep = true;
    face.setExpression(EXPR_SLEEPING);
    audio.playSound(SOUND_POWERDOWN);
    Serial.println("Robot went to sleep from remote command!");
  } else if (text == "ANGRY" || text == "ANIM:ANGRY" || text == "EXPR_ANGRY") {
    currentScreen = SCREEN_FACE;
    face.setExpression(EXPR_ANGRY);
    Serial.println("OK:AngryAnimationTriggered");
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
      
      // Save in RAM and NVS persistent Flash memory
      face.addCalendarEvent(type, time, title);
      saveCalendarEventToNVS(type, time, title);
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
  } else if (text.startsWith("SCREEN:")) {
    int sVal = text.substring(7).toInt();
    if (sVal >= 0 && sVal < SCREEN_MAX) {
      currentScreen = (SmartwatchScreen)sVal;
      settingsActive = false;
      gamesActive = false;
      gamePlaying = false;
      notificationsActive = false;
      Serial.printf("OK:ScreenSwitched:%d\n", sVal);
    }
  } else if (text.startsWith("MAP:")) {
    // Command format: MAP:direction,distance,description OR MAP:EXIT
    String payload = text.substring(4);
    payload.trim();
    if (payload == "EXIT") {
      mapsActive = false;
      currentScreen = SCREEN_FACE;
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
    currentScreen = SCREEN_MAPS;  // <-- CRITICAL: actually show the map screen
    lastInteractionTime = millis(); // reset inactivity timer so map stays visible
    face.setMapNavigation(dirUpper, distance, description);
    
    if (shouldBeep) {
      audio.playSound(SOUND_CHIRP);
    }
    
    activeNotificationDurationMs = 20000; // 20 seconds visibility for turn navigation
  } else if (text.startsWith("MAPLINE:")) {
    int firstColon = text.indexOf(':');
    int secondColon = text.indexOf(':', firstColon + 1);
    if (secondColon > firstColon) {
      int y = text.substring(firstColon + 1, secondColon).toInt();
      String hexStr = text.substring(secondColon + 1);
      int pxCount = hexStr.length() / 4;
      if (pxCount > 0 && pxCount <= 240) {
        static uint16_t lineBuf[240];
        for (int i = 0; i < pxCount; i++) {
          char hbuf[5] = { hexStr[i*4], hexStr[i*4+1], hexStr[i*4+2], hexStr[i*4+3], 0 };
          lineBuf[i] = (uint16_t)strtoul(hbuf, NULL, 16);
        }
        face.updateLiveMapLine(y, lineBuf, pxCount);
        mapsActive = true;
        currentScreen = SCREEN_MAPS;
        lastInteractionTime = millis();
      }
    }
  } else if (text.startsWith("MAPCHUNK:")) {
    int firstColon = text.indexOf(':');
    int secondColon = text.indexOf(':', firstColon + 1);
    if (secondColon > firstColon) {
      int offset = text.substring(firstColon + 1, secondColon).toInt();
      String hexStr = text.substring(secondColon + 1);
      int pxCount = hexStr.length() / 4;
      if (pxCount > 0) {
        uint16_t* chunkBuf = (uint16_t*)malloc(pxCount * sizeof(uint16_t));
        if (chunkBuf != nullptr) {
          for (int i = 0; i < pxCount; i++) {
            char hbuf[5] = { hexStr[i*4], hexStr[i*4+1], hexStr[i*4+2], hexStr[i*4+3], 0 };
            chunkBuf[i] = (uint16_t)strtoul(hbuf, NULL, 16);
          }
          face.updateLiveMapChunk(offset, chunkBuf, pxCount);
          free(chunkBuf);
          mapsActive = true;
          currentScreen = SCREEN_MAPS;
          lastInteractionTime = millis();
        }
      }
    }
  } else if (text == "MAPCLEAR") {
    face.clearLiveMap();
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
      tft.drawPixel(x + tx, y + ty + 20, color); // Apply 20px screen vertical offset
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
  // parts[4], parts[5], parts[6] represent touch settings which are now removed/ignored.
  if (partCount > 7) negativeDisplay = false; // Force White Theme (ignore app dark theme setting)
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
    // Disabled companion app clockStyle override to prevent reverting
    // clockStyle = parts[13].toInt();
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

  face.setDefaultExpression(EXPR_ROBOT_EYE);
  face.setExpression(EXPR_ROBOT_EYE);
  face.setStateLabel("ROBOT_EYE");

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
  // Gestures/touch settings no longer stored in Preferences
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
  // ── Power & Thermal Optimization ────────────────────────────────────────
  // 240MHz is overkill for a UI watch — 160MHz gives identical responsiveness
  // with ~25% lower power draw and significantly less heat.
  setCpuFrequencyMhz(160);

  Serial.begin(115200);
  delay(100);
  
  // 1. Load persistence settings from NVS Preferences first
  preferences.begin("luna", false);
  bleActive = true; // Always ON
  gifSpeed = 169;
  defaultGif = preferences.getInt("defGif", 20);
  gifIntro = preferences.getInt("intGif", 20);
  // Gestures/touch settings no longer loaded
  robotVariant = preferences.getString("robot_var", "ms_luna");
  negativeDisplay = false; // Always boot in White Theme (Light Mode)
  preferences.putBool("neg", false);
  clockStyle = 3; // Force Style 3 (Tactical HUD) on boot
  preferences.putInt("clkStyle", 3);
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
  interaction.begin();   // Initialize touch interface
  imu.begin();           // Initialize accelerometer & gyroscope
  analogSetPinAttenuation(BATTERY_PIN, ADC_11db);
  pinMode(BATTERY_PIN, INPUT); // Initialize battery monitoring pin
  pinMode(40, INPUT_PULLUP);   // Initialize Power button (Key2)
  pinMode(0, INPUT_PULLUP);    // Initialize Boot button (Key1)
  pinMode(41, OUTPUT);         // Hold power pin HIGH to keep power on when on battery
  digitalWrite(41, HIGH);

  // Initialize PCF85063 Hardware RTC
  // Wire is already started by interaction.begin() above (SDA=11, SCL=10)
  if (rtcDevice.begin()) {
    bool hwTimeValid = rtcDevice.readTime(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate);
    if (hwTimeValid) {
      Serial.printf("[RTC] Hardware RTC active & advanced time loaded: %02d:%02d:%02d %s %s\n",
                    rtcHour, rtcMinute, rtcSecond, rtcDay.c_str(), rtcDate.c_str());
    } else {
      Serial.println(F("[RTC] HW read failed — initialising from build time"));
      rtcHour   = BUILD_HOUR;
      rtcMinute = BUILD_MIN;
      rtcSecond = BUILD_SEC;
      rtcDevice.setTimeFull(BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);
      Serial.printf("[RTC] Initialised from compile-time: %02d:%02d:%02d\n", rtcHour, rtcMinute, rtcSecond);
    }
  } else {
    // RTC hardware not found — still try NVS, fallback to compile time
    Serial.println(F("[RTC] PCF85063 not detected — checking NVS"));
    preferences.begin("luna", true);
    int nvs_h    = preferences.getInt("rtcH",  -1);
    int nvs_m    = preferences.getInt("rtcM",  0);
    int nvs_s    = preferences.getInt("rtcS",  0);
    String nvs_day  = preferences.getString("rtcDay",  "Mon");
    String nvs_date = preferences.getString("rtcDate", "01 Jan");
    preferences.end();
    if (nvs_h >= 0 && nvs_h <= 23 && nvs_m >= 0 && nvs_m <= 59) {
      rtcHour   = nvs_h;
      rtcMinute = nvs_m;
      rtcSecond = nvs_s;
      rtcDay    = nvs_day;
      rtcDate   = nvs_date;
    } else {
      rtcHour   = BUILD_HOUR;
      rtcMinute = BUILD_MIN;
      rtcSecond = BUILD_SEC;
      Serial.printf("[RTC] Fallback to compile-time: %02d:%02d:%02d\n", rtcHour, rtcMinute, rtcSecond);
    }
  }

  // Restore timeSynced flag from NVS — if a TIME: sync was ever received,
  // the clock should keep showing real time across reboots.
  {
    preferences.begin("luna", true);
    bool nvsHadSync = preferences.getBool("rtcSynced", false);
    preferences.end();
    if (nvsHadSync) {
      rtcEverSynced   = true;
      face.timeSynced = true;
      Serial.println(F("[RTC] Previous TIME: sync found in NVS — clock enabled."));
    } else {
      Serial.println(F("[RTC] No previous sync in NVS — clock shows --:-- until app syncs."));
    }
  }

  // Initial battery read using 10-sample ADC averaging
  batteryVolts = readBatteryVolts();


  Serial.print(negativeDisplay ? "Ms. Luna Robot Booting Up... Version: " : "Mr. Luna Robot Booting Up... Version: ");
  Serial.println(FIRMWARE_VERSION);

  // Set default expression
  face.setFrameDelay(gifSpeed);
  face.setDefaultExpression(EXPR_ROBOT_EYE);
  face.setExpression(EXPR_ROBOT_EYE);

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
  
  // Initialize ST7789 Display in SPI MODE 3 (240x320 resolution controller mode)
  tft.init(240, 320, SPI_MODE3);
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
  face.setExpression(EXPR_ROBOT_EYE);

  lastInteractionTime = millis();
  lastRtcMillis = millis();
  lastExpressionCycleTime = millis();

  games.begin();
  qrCard.begin();  // Load persisted business card URL from NVS
}

// Global index for all-gifs cycling — advances through all 63 entries
int allGifCycleIdx = 0;

void cycleExpression() {
  face.setDefaultExpression(EXPR_ROBOT_EYE);
  face.setExpression(EXPR_ROBOT_EYE);
  face.setStateLabel("ROBOT_EYE");
  Serial.println("PLAYING ROBOT_EYE ANIMATION (Sprite AI)");
}

String getExpressionName(int expr) {
  if (expr >= 100) {
    return "ROBOT_EYE";
  }
  switch (expr) {
    case 0: return "IDLE";
    case 1: return "HAPPY";
    case 2: return "SAD";
    case 3: return "ANGRY";
    case 4: return "SURPRISED";
    case 5: return "SLEEPING";
    case 6: return "WINK";
    case 7: return "TEXT";
    case 8: return "CLOCK";
    case 9: return "MAP";
    case 20: return "ROBOT_EYE";
    default: return "HAPPY";
  }
}

// executeTouchAction removed
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
        clockStyle = (clockStyle + 1) % 5;
      } else {
        clockStyle = (clockStyle - 1 + 5) % 5;
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

  // On touchscreen firmware, we disable single-tap exit on the QR Card screen
  // to prevent accidental exits when showing/scanning the QR code or during fast swipes.
  if (currentScreen == SCREEN_CARD) {
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
      int lastY = interaction.getLastY();
      int canvasY = lastY - 20; // 20px screen offset calibration
      if (canvasY >= 62 && canvasY <= 190) {
        int scrollOffset = (menuOption >= 4) ? (menuOption - 3) : 0;
        int optIdx = (canvasY - 62) / 32 + scrollOffset;
        if (optIdx >= 0 && optIdx < 8) {
          if (menuOption == optIdx) {
            // Tapped already highlighted option
            if (menuOption == 6 || menuOption == 7) {
              // Save / Exit execute immediately without entering optionSelected state
              adjustOption(menuOption, 1);
            } else if (optionSelected) {
              // If already adjusting, adjust it
              adjustOption(menuOption, 1);
            } else {
              // Select it
              optionSelected = true;
              audio.playSound(SOUND_POWERUP);
              Serial.printf("[BTN1] Settings Option %d SELECTED\n", menuOption);
            }
          } else {
            // Highlight the new option
            menuOption = optIdx;
            optionSelected = false;
            audio.playSound(SOUND_CHIRP);
            Serial.printf("[BTN1] Settings Option highlighted -> %d\n", menuOption);
          }
        }
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
        int lastY = interaction.getLastY();
        int canvasY = lastY - 20; // 20px screen offset calibration
        if (canvasY >= 66 && canvasY <= 194) {
          int scrollOffset = (gameMenuOption >= 4) ? (gameMenuOption - 3) : 0;
          int optIdx = (canvasY - 66) / 32 + scrollOffset;
          if (optIdx >= 0 && optIdx < 8) {
            if (gameMenuOption == optIdx) {
              if (gameMenuOption == 7) {
                // Exit arcade menu
                gamesActive = false;
                gamePlaying = false;
                currentScreen = SCREEN_FACE;
                audio.playSound(SOUND_POWERDOWN);
                Serial.println("[BTN1] Exited Games to FACE screen");
              } else {
                // Selected game tapped again -> Start the game!
                gameSelected = gameMenuOption + 1;
                if (gameSelected == 1) games.resetRacer();
                else if (gameSelected == 2) games.resetSpace();
                else if (gameSelected == 3) games.resetFlappy();
                else if (gameSelected == 4) games.resetCatcher();
                else if (gameSelected == 5) games.resetJump();
                else if (gameSelected == 6) games.resetStacker();
                else if (gameSelected == 7) games.resetMemory();
                gamePlaying = true;
                audio.playSound(SOUND_STARTUP);
                Serial.printf("[BTN1] Started Game %d\n", gameSelected);
              }
            } else {
              // Highlight the new game
              gameMenuOption = optIdx;
              audio.playSound(SOUND_CHIRP);
              Serial.printf("[BTN1] Highlighted Game Option -> %d\n", gameMenuOption);
            }
          }
        }
      }
    }
    return;
  }

  if (currentScreen == SCREEN_FACE) {
    // Center tap on FACE screen -> cycle through all 3 Sprite AI animations
    face.getRobotEyeAnim().nextAnimation();
    audio.playSound(SOUND_CHIRP);
    Serial.printf("[BTN1] Cycled Sprite AI animation -> Anim #%d\n", face.getRobotEyeAnim().getAnimationIndex());
    return;
  } else if (currentScreen == SCREEN_CLOCK) {
    // Cycles clock styles
    clockStyle = (clockStyle + 1) % 5;
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
      if (notificationSelected) {
        // Detail View: tap deselects/exits detail view
        notificationSelected = false;
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[BTN1] Exited Notification Detail View");
      } else {
        int lastY = interaction.getLastY();
        int optIdx = (lastY - 58) / 33;
        int notifCount = face.getNotificationCount();
        if (optIdx >= 0 && optIdx < notifCount && optIdx < 5) {
          if (face.getCurrentNotifViewIdx() == optIdx) {
            // Tapped already highlighted notification -> Open it!
            notificationSelected = true;
            audio.playSound(SOUND_POWERUP);
            Serial.printf("[BTN1] Opened Notification %d\n", optIdx);
          } else {
            // Highlight the tapped notification
            face.setCurrentNotifViewIdx(optIdx);
            audio.playSound(SOUND_CHIRP);
            Serial.printf("[BTN1] Highlighted Notification -> %d\n", optIdx);
          }
        }
      }
    }
    return;
  } else if (currentScreen == SCREEN_CALENDAR) {
    // Cycles calendar events/view
    face.cycleCalendarView();
    audio.playSound(SOUND_CHIRP);
    Serial.println("[BTN1] Cycled calendar");
  } else if (currentScreen == SCREEN_LEVEL) {
    // Calibrate level sensor on single tap
    calibrateRequest = true;
    Serial.println("[BTN1] Triggered Level calibration");
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
    if (gamePlaying) {
      // Game is playing: check if we can exit
      if (games.canExitActiveGame()) {
        gamePlaying = false;
        audio.playSound(SOUND_POWERDOWN);
        Serial.println("[BTN2] Exited active game back to Arcade menu");
      }
      return;
    }
  }

  unsigned long transitionNow = millis();
  if (transitionNow - lastScreenTransitionTime < 350) {
    Serial.println("[BTN2] Ignored rapid single tap screen transition to prevent bounce");
    return;
  }
  lastScreenTransitionTime = transitionNow;

  // Screen cycle: Face <-> Card <-> Clock <-> Notifications <-> Calendar <-> Games <-> Settings <-> Level <-> Pomodoro <-> Face
  static const SmartwatchScreen CYCLE[] = {
    SCREEN_FACE, SCREEN_CARD, SCREEN_CLOCK, SCREEN_NOTIFICATIONS,
    SCREEN_CALENDAR, SCREEN_GAMES, SCREEN_SETTINGS, SCREEN_LEVEL, SCREEN_POMODORO
  };
  static const int CYCLE_LEN = 9;

  int idx = 0;
  for (int i = 0; i < CYCLE_LEN; i++) {
    if (CYCLE[i] == currentScreen) { idx = i; break; }
  }
  currentScreen = CYCLE[(idx + 1) % CYCLE_LEN];

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

  const char* names[] = {"FACE","CARD","CLOCK","NOTIF","CAL","GAMES","SETTINGS","LEVEL","POMO"};
  Serial.printf("[BTN2] >>> %s (screen %d)\n", names[(idx+1)%CYCLE_LEN], currentScreen);
}


void handleBtn2Double() {
  lastInteractionTime = millis();
  if (mapsActive) return;

  unsigned long transitionNow = millis();
  if (transitionNow - lastScreenTransitionTime < 350) {
    Serial.println("[BTN2 DBL] Ignored rapid double tap screen transition to prevent bounce");
    return;
  }
  lastScreenTransitionTime = transitionNow;

  static const SmartwatchScreen CYCLE[] = {
    SCREEN_FACE, SCREEN_CARD, SCREEN_CLOCK, SCREEN_NOTIFICATIONS,
    SCREEN_CALENDAR, SCREEN_GAMES, SCREEN_SETTINGS, SCREEN_LEVEL, SCREEN_POMODORO
  };
  static const int CYCLE_LEN = 9;

  int idx = 0;
  for (int i = 0; i < CYCLE_LEN; i++) {
    if (CYCLE[i] == currentScreen) { idx = i; break; }
  }
  currentScreen = CYCLE[(idx - 1 + CYCLE_LEN) % CYCLE_LEN];

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

  const char* names[] = {"FACE","CARD","CLOCK","NOTIF","CAL","GAMES","SETTINGS","LEVEL","POMO"};
    Serial.printf("[BTN2 DBL] <<< %s (screen %d)\n", names[(idx-1+CYCLE_LEN)%CYCLE_LEN], currentScreen);
}

void handleSwipeUp() {
  lastInteractionTime = millis();
  if (mapsActive || gamePlaying) return;

  if (currentScreen == SCREEN_SETTINGS && settingsActive) {
    if (optionSelected) {
      adjustOption(menuOption, 1);
    } else {
      menuOption = (menuOption + 1) % 8;
      audio.playSound(SOUND_CHIRP);
      Serial.printf("[Swipe Up] Settings Option highlighted -> %d\n", menuOption);
    }
  } else if (currentScreen == SCREEN_GAMES && gamesActive && !gamePlaying) {
    gameMenuOption = (gameMenuOption + 1) % 8;
    audio.playSound(SOUND_CHIRP);
    Serial.printf("[Swipe Up] Game Option highlighted -> %d\n", gameMenuOption);
  }
}

void handleSwipeDown() {
  lastInteractionTime = millis();
  if (mapsActive || gamePlaying) return;

  if (currentScreen == SCREEN_SETTINGS && settingsActive) {
    if (optionSelected) {
      adjustOption(menuOption, -1);
    } else {
      menuOption = (menuOption - 1 + 8) % 8;
      audio.playSound(SOUND_CHIRP);
      Serial.printf("[Swipe Down] Settings Option highlighted -> %d\n", menuOption);
    }
  } else if (currentScreen == SCREEN_GAMES && gamesActive && !gamePlaying) {
    gameMenuOption = (gameMenuOption - 1 + 8) % 8;
    audio.playSound(SOUND_CHIRP);
    Serial.printf("[Swipe Down] Game Option highlighted -> %d\n", gameMenuOption);
  }
}

void updateStateLabel() {
  if (currentScreen == SCREEN_FACE) {
    Expression expr = face.getExpression();
    switch (expr) {
      case EXPR_IDLE: face.setStateLabel("IDLE"); break;
      case EXPR_HAPPY: face.setStateLabel("HAPPY"); break;
      case EXPR_SAD: face.setStateLabel("SAD"); break;
      case EXPR_ANGRY: face.setStateLabel("ANGRY"); break;
      case EXPR_SURPRISED: face.setStateLabel("SURPRISE"); break;
      case EXPR_SLEEPING: face.setStateLabel("SLEEP"); break;
      case EXPR_WINK: face.setStateLabel("WINK"); break;
      case EXPR_ROBOT_EYE: face.setStateLabel("ROBOT_EYE"); break;
      default: face.setStateLabel("ROBOT_EYE"); break;
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

  // ── 0. Poll Physical Buttons for Sleep/Wake ──────────────────────────────
  static unsigned long powerBtnPressStart = 0;
  static bool powerBtnWasPressed = false;
  static unsigned long bootBtnPressStart = 0;
  static bool bootBtnWasPressed = false;

  bool powerPressed = (digitalRead(40) == LOW);
  bool bootPressed = (digitalRead(0) == LOW);

  // Handle Power button (GPIO 40)
  if (powerPressed) {
    if (!powerBtnWasPressed) {
      powerBtnWasPressed = true;
      powerBtnPressStart = now;
    } else {
      if (now - powerBtnPressStart >= 1500) {
        // Long press -> Enter Sleep
        if (!isAsleep) {
          isAsleep = true;
          audio.playSound(SOUND_POWERDOWN);
          Serial.println("[Power Button] Long press -> Entering Sleep Mode");
          while (digitalRead(40) == LOW) {
            delay(10);
            audio.update(); // Keep audio synthesizer playing
          }
          powerBtnWasPressed = false;
          powerBtnPressStart = 0;
        }
      }
    }
  } else {
    if (powerBtnWasPressed) {
      unsigned long pressDuration = now - powerBtnPressStart;
      powerBtnWasPressed = false;
      powerBtnPressStart = 0;
      
      if (pressDuration >= 50 && pressDuration < 1500) {
        if (isAsleep) {
          isAsleep = false;
          audio.playSound(SOUND_CHIRP);
          Serial.println("[Power Button] Short press -> Waking up from Sleep Mode");
        } else if (gamePlaying) {
          // Exit game to arcade menu
          gamePlaying = false;
          gamesActive = true;
          audio.playSound(SOUND_POWERDOWN);
          Serial.println("[Power Button] Short press in game -> Exited to arcade");
        }
      }
    }
  }

  // Handle Boot button (GPIO 0)
  if (bootPressed) {
    if (!bootBtnWasPressed) {
      bootBtnWasPressed = true;
      bootBtnPressStart = now;
    } else {
      if (now - bootBtnPressStart >= 1500) {
        // Long press -> Toggle silent mode
        silentMode = !silentMode;
        audio.silentMode = silentMode;
        
        // Save to NVS
        preferences.begin("luna", false);
        preferences.putBool("silent", silentMode);
        preferences.end();
        
        // Trigger non-intrusive transient silent overlay notification
        face.triggerSilentOverlay(silentMode);
        if (silentMode) {
          Serial.println("[Boot Button] Long press -> Silent Mode Enabled");
        } else {
          audio.playSound(SOUND_CHIRP);
          Serial.println("[Boot Button] Long press -> Silent Mode Disabled");
        }
        
        // Notify BLE app
        String silentValStr = silentMode ? "1" : "0";
        String negValStr = negativeDisplay ? "1" : "0";
        ble.sendLog("SET_SYNC:" + String(clockStyle) + "," + String(oledBrightness) + "," + negValStr + "," + silentValStr);
        
        // Wait for button release
        while (digitalRead(0) == LOW) {
          delay(10);
          audio.update(); // Keep audio synthesizer playing
        }
        bootBtnWasPressed = false;
        bootBtnPressStart = 0;
      }
    }
  } else {
    if (bootBtnWasPressed) {
      unsigned long pressDuration = now - bootBtnPressStart;
      bootBtnWasPressed = false;
      bootBtnPressStart = 0;
      
      if (pressDuration >= 50 && pressDuration < 1500) {
        if (gamePlaying && !isAsleep) {
          gamePlaying = false;
          gamesActive = true;
          audio.playSound(SOUND_POWERDOWN);
          Serial.println("[Boot Button] Short press in game -> Exited to arcade");
        }
      }
    }
  }

  // ── 0.5. High-Efficiency Sleep Path ─────────────────────────────────────
  if (isAsleep) {
    // Run minimal tasks for sleep mode
    ble.handleConnectionState();
    // Update Pomodoro timer background countdown
    updatePomodoroTimer();

    
    // Update Real-Time Clock from PCF85063 hardware with software tick fallback
    if (now - lastRtcMillis >= 1000) {
      lastRtcMillis = now;
      int prevS = rtcSecond;
      int prevM = rtcMinute;
      int prevH = rtcHour;
      bool hwOk = rtcDevice.readTime(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate);
      if (!hwOk || (hwOk && rtcHour == prevH && rtcMinute == prevM && rtcSecond == prevS)) {
        rtcSecond = prevS + 1;
        rtcMinute = prevM;
        rtcHour   = prevH;
        if (rtcSecond >= 60) {
          rtcSecond = 0;
          rtcMinute++;
          if (rtcMinute >= 60) {
            rtcMinute = 0;
            rtcHour++;
            if (rtcHour >= 24) rtcHour = 0;
          }
        }
      }
    }
    
    // Periodic battery read (every 10 seconds)
    static unsigned long lastBatteryReadTime = 0;
    if (now - lastBatteryReadTime > 10000) {
      lastBatteryReadTime = now;
      float rawVolts = readBatteryVolts();
      if (batteryVolts == 3.82f) {
        batteryVolts = rawVolts;
      } else {
        batteryVolts = 0.8f * batteryVolts + 0.2f * rawVolts;
      }
    }

    
    // Draw the display at ~30fps rate
    static unsigned long lastDisplayDrawTime = 0;
    if (now - lastDisplayDrawTime >= 33) {
      lastDisplayDrawTime = now;
      face.setConnectivityStatus(ble.isConnected(), false);
      face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);
    }
    
    vTaskDelay(1);
    return;
  }

  // ── 0.6. Poll unified touch handler (only when awake) ───────────────────
  ButtonEvent btnEvt = interaction.update();
  switch (btnEvt) {
    case BTN1_SINGLE: handleBtn1Single(); break;
    case BTN1_DOUBLE: handleBtn1Double(); break;
    case BTN1_LONG:   handleBtn1Long();   break;
    case BTN2_SINGLE:
    case BTN_SWIPE_LEFT:
      handleBtn2Single();
      break;
    case BTN2_DOUBLE:
    case BTN_SWIPE_RIGHT:
      handleBtn2Double();
      break;
    case BTN_SWIPE_UP:   handleSwipeUp();   break;
    case BTN_SWIPE_DOWN: handleSwipeDown(); break;
    default: break;
  }

  // 1. Maintain BLE stack status and connection advertisement
  ble.handleConnectionState();

  // 1.5. Check offline hardware scheduled alarms & calendar events
  checkHardwareScheduledAlarms();
  updatePomodoroTimer();


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
      Serial.println("SETTINGS:" + String(bleActive ? "1" : "0") + "," + String(gifSpeed) + "," + String(defaultGif) + "," + String(gifIntro) + ",0,0,0," + String(negativeDisplay ? "1" : "0"));
    } else if (cmd == "LIST") {
      // Re-print all GIF entries for debugging
      Serial.println("====== GIF TABLE DUMP ======");
      Serial.print("Total GIFs loaded: ");
      Serial.println("Sprite AI Robot Eye animation active.");
      Serial.println("=============================");
    } else {
      // Fallback: forward generic commands (e.g. MAP:, NOTIF:, EXPR:, AUDIO:) to the robot command handler
      handleRobotCommand(cmd);
    }
  }

  // Note: All navigation is now handled by the physical button event dispatcher
  // at the top of loop() via interaction.update() + handleBtn1/2 functions.


  // 3.5. Update Real-Time Clock from PCF85063 hardware with software tick fallback
  if (now - lastRtcMillis >= 1000) {
    lastRtcMillis = now;
    int prevS = rtcSecond;
    int prevM = rtcMinute;
    int prevH = rtcHour;
    bool hwOk = rtcDevice.readTime(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate);
    if (!hwOk || (hwOk && rtcHour == prevH && rtcMinute == prevM && rtcSecond == prevS)) {
      rtcSecond = prevS + 1;
      rtcMinute = prevM;
      rtcHour   = prevH;
      if (rtcSecond >= 60) {
        rtcSecond = 0;
        rtcMinute++;
        if (rtcMinute >= 60) {
          rtcMinute = 0;
          rtcHour++;
          if (rtcHour >= 24) rtcHour = 0;
        }
      }
    }
  }


  // 3.6. Expression cycling
  if (currentScreen == SCREEN_FACE && !isAsleep) {
    if (face.getExpression() != EXPR_ROBOT_EYE) {
      cycleExpression();
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

  // 4. Inactivity Timer: Auto-return to Face screen after 60 seconds of no interaction in UI modes
  // Note: SCREEN_CARD is excluded – QR must stay visible until user explicitly dismisses it
  // Inactivity timeout: 5 minutes of no interaction returns to Face screen
  if (currentScreen != SCREEN_FACE && currentScreen != SCREEN_CARD && !inIntroPhase && !isAlarmRinging && !isReminderRinging && !mapsActive && !gamePlaying) {
    if (now >= lastInteractionTime && now - lastInteractionTime >= 300000) {
      currentScreen = SCREEN_FACE;
      lastExpressionCycleTime = now;
      Serial.println("[Inactivity] 5min timeout: returning to Face screen.");
    }
  }

  // Periodic battery read (every 10 seconds)
  static unsigned long lastBatteryReadTime = 0;
  if (now - lastBatteryReadTime > 10000) {
    lastBatteryReadTime = now;
    float rawVolts = readBatteryVolts();
    if (batteryVolts == 3.82f) {
      batteryVolts = rawVolts;
    } else {
      batteryVolts = 0.8f * batteryVolts + 0.2f * rawVolts;
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
    face.setConnectivityStatus(ble.isConnected(), false);
    face.draw(rtcHour, rtcMinute, rtcSecond, rtcDay, rtcDate, clockStyle, is12HourFormat);
  }

  // Yield one FreeRTOS tick to WiFi/BLE background tasks.
  // Prevents the Arduino loop task from monopolising Core 1 at 100% and
  // causing the IC to heat up. Completely invisible to the user at 30fps.
  vTaskDelay(1);
}
