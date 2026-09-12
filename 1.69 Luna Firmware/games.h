#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Preferences.h>
#include "config.h"
#include "audio.h"
#include "imu.h"

// External references
extern bool gamesActive;
extern bool gamePlaying;
extern int gameMenuOption;
extern int gameSelected;
extern float gamesScrollPx;

extern bool virtualBtn1;
extern bool virtualBtn2;
extern LunaIMU imu;

class LunaGames {
private:
  // High scores
  int racHighScore;
  int spcHighScore;
  int flapHighScore;
  int catHighScore;
  int jumpHighScore;
  int stkHighScore;
  int memHighScore;

  // --- Game 1: Luna Racer State ---
  float racPlayerX;
  float racPlayerY;
  float racSpeed;
  bool racNitroActive;
  float racNitroFuel;
  float racRoadScroll;
  int racScore;
  bool racGameOver;
  float racCarX[4];
  float racCarY[4];
  float racCarSpeed[4];
  int racCarColor[4];
  bool racCarActive[4];

  // --- Game 2: Luna Space State ---
  float spcPlayerX;
  float spcPlayerY;
  int spcScore;
  int spcHealth;
  bool spcGameOver;
  unsigned long spcLastShoot;
  unsigned long spcShieldTime;
  unsigned long spcSmartBombCooldown;
  float spcLaserX[6];
  float spcLaserY[6];
  bool spcLaserActive[6];
  float spcEnemyX[8];
  float spcEnemyY[8];
  bool spcEnemyActive[8];
  int spcEnemyHP[8];
  int spcEnemyType[8];
  unsigned long spcEnemyLastShoot[8];
  float spcEnemyLaserX[4];
  float spcEnemyLaserY[4];
  bool spcEnemyLaserActive[4];
  float spcPartX[15];
  float spcPartY[15];
  float spcPartVX[15];
  float spcPartVY[15];
  int spcPartLife[15];
  bool spcPartActive[15];
  float spcPowerX;
  float spcPowerY;
  int spcPowerType;
  bool spcPowerActive;
  bool spcSpreadActive;
  bool spcBossActive;
  int spcBossHP;
  float spcBossX;
  float spcBossY;
  int spcBossDir;
  unsigned long spcBossLastShoot;

  // --- Game 3: Flappy Mochy State ---
  float flapPlayerY;
  float flapPlayerVY;
  float flapPipeX;
  float flapPipeGapY;
  int flapScore;
  bool flapGameOver;

  // --- Game 4: Coin Catcher State ---
  float catPlayerX;
  float catCoinsX[3];
  float catCoinsY[3];
  bool catCoinsActive[3];
  bool catIsBomb[3];
  int catScore;
  int catLives;
  bool catGameOver;

  // --- Game 5: Mochy Jump State ---
  float jumpPlayerX;
  float jumpPlayerY;
  float jumpPlayerVY;
  float jumpPlatX[5];
  float jumpPlatY[5];
  int jumpScore;
  bool jumpGameOver;

  // --- Game 6: Stacker State ---
  float stkBlockX;
  float stkBlockW;
  float stkBlockY;
  float stkSpeed;
  int stkDir;
  float stkBaseX;
  int stkScore;
  bool stkGameOver;
  int stkStackHeight;
  float stkStackX[12];
  float stkStackW[12];

  // --- Game 7: Memory Matrix State ---
  int memSeq[16];
  int memSeqLen;
  int memSeqStep;
  int memPlayerStep;
  int memSelectedTile;
  int memState; // 0: showing sequence, 1: player input
  unsigned long memTimer;
  int memScore;
  bool memGameOver;

  // Input helpers
  bool lastBtn1;
  bool lastBtn2;

public:
  LunaGames() {
    racHighScore = 0;
    spcHighScore = 0;
    flapHighScore = 0;
    catHighScore = 0;
    jumpHighScore = 0;
    stkHighScore = 0;
    memHighScore = 0;
    lastBtn1 = false;
    lastBtn2 = false;
  }

#if SCREEN_WIDTH == 240
  static const int G_X = 0;
  static const int G_Y = 58;
  static const int G_W = 240;
  static const int G_H = 182;
  static const int G_B = 240;
  static const int G_R = 240;
#else
  static const int G_X = 0;
  static const int G_Y = 22;
  static const int G_W = 128;
  static const int G_H = 138;
  static const int G_B = 160;
  static const int G_R = 128;
#endif

  void begin() {
    Preferences prefs;
    prefs.begin("luna_scores", true);
    racHighScore = prefs.getInt("rac", 0);
    spcHighScore = prefs.getInt("spc", 0);
    flapHighScore = prefs.getInt("flap", 0);
    catHighScore = prefs.getInt("cat", 0);
    jumpHighScore = prefs.getInt("jump", 0);
    stkHighScore = prefs.getInt("stk", 0);
    memHighScore = prefs.getInt("mem", 0);
    prefs.end();
  }

  void saveHighScore(const char* key, int& highScore, int currentScore) {
    if (currentScore > highScore) {
      highScore = currentScore;
      Preferences prefs;
      prefs.begin("luna_scores", false);
      prefs.putInt(key, highScore);
      prefs.end();
    }
  }

  // Accelerometer & Gyroscope 6-Axis Motion Steering Engine
  // Returns normalized lateral deflection: < 0 for left, > 0 for right
  float getTiltSteer() {
    if (!imu.isInitialized()) return 0.0f;
    float ax = 0.0f, ay = 0.0f, az = 1.0f;
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    if (!imu.readMotion(ax, ay, az, gx, gy, gz)) return 0.0f;

    // Lateral tilt acceleration on portrait 240x280 (ST7789):
    // Tilting right makes ay negative, so -ay is positive.
    // Tilting left makes ay positive, so -ay is negative.
    float lateral = -ay;

    // Combine angular velocity (gyro roll rate in dps) for zero-latency flick/reaction
    lateral += (-gy * 0.0025f);

    const float deadzone = 0.06f;
    if (lateral > deadzone) {
      return constrain((lateral - deadzone) / 0.35f, 0.0f, 1.6f);
    } else if (lateral < -deadzone) {
      return -constrain((-lateral - deadzone) / 0.35f, 0.0f, 1.6f);
    }
    return 0.0f;
  }

  void resetRacer() {
    int carW = SCREEN_WIDTH == 240 ? 16 : 8;
    int carH = SCREEN_WIDTH == 240 ? 28 : 14;
    racPlayerX = G_X + G_W / 2 - carW / 2;
    racPlayerY = G_B - carH - 6;
    racSpeed = SCREEN_WIDTH == 240 ? 3.0f : 2.5f;
    racNitroActive = false;
    racNitroFuel = 100.0f;
    racRoadScroll = 0;
    racScore = 0;
    racGameOver = false;
    for (int i = 0; i < 4; i++) {
      racCarX[i] = G_X + 24 + random(0, 3) * (G_W - 48)/3 - carW / 2;
      racCarY[i] = G_Y - 30 - i * 60;
      racCarSpeed[i] = 1.2f + random(0, 20) / 10.0f;
      racCarColor[i] = i % 3;
      racCarActive[i] = true;
    }
  }

  void resetSpace() {
    spcPlayerX = G_X + G_W / 2;
    spcPlayerY = G_B - (SCREEN_WIDTH == 240 ? 20 : 12);
    spcScore = 0;
    spcHealth = 3;
    spcGameOver = false;
    spcLastShoot = 0;
    spcShieldTime = 0;
    spcSmartBombCooldown = 0;
    spcSpreadActive = false;
    for (int i = 0; i < 6; i++) spcLaserActive[i] = false;
    for (int i = 0; i < 8; i++) spcEnemyActive[i] = false;
    for (int i = 0; i < 4; i++) spcEnemyLaserActive[i] = false;
    for (int i = 0; i < 15; i++) spcPartActive[i] = false;
    spcPowerActive = false;
    spcBossActive = false;
    spcBossHP = 20;
    spcBossX = G_X + G_W / 2;
    spcBossY = G_Y + 15;
    spcBossDir = 1;
    spcBossLastShoot = 0;
  }

  void resetFlappy() {
    flapPlayerY = G_Y + G_H / 2;
    flapPlayerVY = 0;
    flapPipeX = G_R;
    int gapH = SCREEN_WIDTH == 240 ? 50 : 35;
    flapPipeGapY = G_Y + 20 + random(0, G_H - gapH - 40);
    flapScore = 0;
    flapGameOver = false;
  }

  void resetCatcher() {
    int basketW = SCREEN_WIDTH == 240 ? 24 : 16;
    catPlayerX = G_X + G_W / 2 - basketW / 2;
    catScore = 0;
    catLives = 3;
    catGameOver = false;
    for (int i = 0; i < 3; i++) {
      catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
      catCoinsY[i] = G_Y - 20 - i * 50;
      catCoinsActive[i] = true;
      catIsBomb[i] = (random(0, 10) < 3);
    }
  }

  void resetJump() {
    jumpPlayerX = G_X + G_W / 2;
    jumpPlayerY = G_B - 40;
    jumpPlayerVY = -5.0f;
    jumpScore = 0;
    jumpGameOver = false;
    for (int i = 0; i < 5; i++) {
      jumpPlatX[i] = G_X + 10 + random(0, G_W - 50);
      jumpPlatY[i] = G_B - 15 - i * 36;
    }
  }

  void resetStacker() {
    stkBlockW = SCREEN_WIDTH == 240 ? 60 : 40;
    stkBlockX = G_X + (G_W - stkBlockW) / 2;
    stkBlockY = G_B - (SCREEN_WIDTH == 240 ? 20 : 12);
    stkSpeed = SCREEN_WIDTH == 240 ? 3.5f : 2.0f;
    stkDir = 1;
    stkBaseX = stkBlockX;
    stkStackHeight = 0;
    stkScore = 0;
    stkGameOver = false;
    for (int i = 0; i < 12; i++) {
      stkStackX[i] = 0;
      stkStackW[i] = 0;
    }
  }

  void resetMemory() {
    memSeqLen = 1;
    memSeqStep = 0;
    memPlayerStep = 0;
    memSelectedTile = 4;
    memState = 0;
    memSeq[0] = random(0, 9);
    memTimer = millis();
    memScore = 0;
    memGameOver = false;
  }

  bool canExitActiveGame() {
    if (gameSelected == 1) return racGameOver;
    if (gameSelected == 2) return spcGameOver;
    if (gameSelected == 3) return flapGameOver;
    if (gameSelected == 4) return catGameOver;
    if (gameSelected == 5) return jumpGameOver;
    if (gameSelected == 6) return stkGameOver;
    if (gameSelected == 7) return memGameOver;
    return false;
  }

  void drawGameOverScreen(GFXcanvas16& display, int score, int highScore) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8;
    uint16_t themeBg  = TFT_BLACK;
    uint16_t themeText = TFT_WHITE;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);

    display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
    display.setTextColor(TFT_RED);
    display.setCursor((SCREEN_WIDTH - 9 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, SCREEN_WIDTH == 240 ? 65 : 36);
    display.print("GAME OVER");

    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "SC:%d HI:%d", score, highScore);
    int scoreW = strlen(scoreBuf) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - scoreW) / 2, SCREEN_WIDTH == 240 ? 115 : 72);
    display.print(scoreBuf);

    display.setTextColor(themeAccent);
    const char* pAgain = "TAP TO REPLAY";
    int pW = strlen(pAgain) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - pW) / 2, SCREEN_WIDTH == 240 ? 160 : 108);
    display.print(pAgain);

    const char* bMenu = "SWIPE TO EXIT";
    int bW = strlen(bMenu) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - bW) / 2, SCREEN_WIDTH == 240 ? 190 : 126);
    display.print(bMenu);
  }

  void drawMenu(GFXcanvas16& display) {
    extern String robotVariant;
    extern bool negativeDisplay;
    uint16_t themeAccent  = 0x001F; // Deep Royal Blue
    uint16_t themeBg      = 0xFFFF; // Crisp Pure White
    uint16_t themeText    = 0x1082; // Deep Graphite/Black
    uint16_t themeBorder  = 0xCE79; // Crisp light grey hairline
    uint16_t themeSubText = 0x632C; // Muted titanium slate

    // Clear display below status bar
    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // ── Header ──────────────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(20, 28);
    display.print("LUNA ARCADE");
    display.fillCircle(168, 35, 3, themeAccent);

    display.drawFastHLine(20, 46, SCREEN_WIDTH - 40, themeBorder);

    // ── Game Titles & Metadata ──────────────────────────────────────────────
    int opt = constrain(gameMenuOption, 0, 7);

    const char* titlesL1[] = {
      "LUNA", "LUNA", "FLAPPY", "COIN", "MOCHY", "STACKER", "MEMORY", "EXIT"
    };
    const char* titlesL2[] = {
      "RACER", "SPACE", "MOCHY", "CATCHER", "JUMP", "TOWER", "MATRIX", "ARCADE"
    };
    int highScores[] = {
      racHighScore, spcHighScore, flapHighScore, catHighScore, jumpHighScore, stkHighScore, memHighScore, 0
    };

    // Dominant 2-line title
    display.setTextSize(3);
    display.setTextColor(themeText);
    display.setCursor(20, 56);
    display.print(titlesL1[opt]);

    display.setTextColor(themeAccent);
    display.setCursor(20, 84);
    display.print(titlesL2[opt]);

    // Metadata tagline
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 114);
    if (opt < 7) {
      char scoreBuf[40];
      if (opt == 0 || opt == 1 || opt == 3 || opt == 4) {
        snprintf(scoreBuf, sizeof(scoreBuf), "HI:%d // GYRO TILT ACTIVE", highScores[opt]);
      } else {
        snprintf(scoreBuf, sizeof(scoreBuf), "SIMULATION // HI-SCORE: %d", highScores[opt]);
      }
      display.print(scoreBuf);
    } else {
      display.print("RETURN // WATCH INTERFACE");
    }

    // ── Futuristic Wireframe Silhouette Preview ($Y \in [128, 185]$) ────────
    int cx = 120;
    int cy = 154;

    switch (opt) {
      case 0: { // LUNA RACER: Aerodynamic wireframe speedster
        display.drawFastHLine(cx - 50, cy - 14, 25, 0xCE79);
        display.drawFastHLine(cx - 60, cy,      30, themeBorder);
        display.drawFastHLine(cx - 45, cy + 14, 20, 0xCE79);
        display.drawTriangle(cx + 34, cy, cx - 20, cy - 16, cx - 20, cy + 16, themeAccent);
        display.fillTriangle(cx + 26, cy, cx - 14, cy - 12, cx - 14, cy + 12, 0xF7BE);
        display.fillRect(cx - 26, cy - 18, 10, 6, themeText);
        display.fillRect(cx - 26, cy + 12, 10, 6, themeText);
        display.fillRect(cx + 12, cy - 16, 8, 4, themeText);
        display.fillRect(cx + 12, cy + 12, 8, 4, themeText);
        display.fillCircle(cx - 2, cy, 3, themeAccent);
      } break;

      case 1: { // LUNA SPACE: Starfighter
        display.drawCircle(cx, cy, 32, themeBorder);
        display.drawFastHLine(cx - 38, cy, 76, 0xCE79);
        display.drawFastVLine(cx, cy - 38, 76, 0xCE79);
        display.drawTriangle(cx, cy - 24, cx - 22, cy + 18, cx + 22, cy + 18, themeAccent);
        display.fillTriangle(cx, cy - 18, cx - 14, cy + 14, cx + 14, cy + 14, 0xF7BE);
        display.fillTriangle(cx, cy + 26, cx - 6, cy + 18, cx + 6, cy + 18, 0x07E0);
      } break;

      case 2: { // FLAPPY MOCHY: Cyber Companion
        display.drawFastVLine(cx - 40, cy - 26, 18, themeBorder);
        display.drawFastVLine(cx - 40, cy + 8,  20, themeBorder);
        display.drawFastVLine(cx + 40, cy - 20, 14, themeBorder);
        display.drawFastVLine(cx + 40, cy + 12, 18, themeBorder);
        display.fillCircle(cx, cy, 12, themeAccent);
        display.drawCircle(cx, cy, 14, themeText);
        display.fillCircle(cx + 4, cy - 3, 3, 0xFFFF);
        display.fillTriangle(cx + 12, cy, cx + 12, cy + 4, cx + 20, cy + 2, 0xFD20);
        display.drawLine(cx - 8, cy, cx - 18, cy - 12, themeText);
        display.drawLine(cx - 18, cy - 12, cx - 4, cy - 4, themeText);
      } break;

      case 3: { // COIN CATCHER: Precision Prism
        display.drawCircle(cx, cy, 26, themeBorder);
        display.drawLine(cx, cy - 20, cx + 18, cy, 0xFD20);
        display.drawLine(cx + 18, cy, cx, cy + 20, 0xFD20);
        display.drawLine(cx, cy + 20, cx - 18, cy, 0xFD20);
        display.drawLine(cx - 18, cy, cx, cy - 20, 0xFD20);
        display.drawLine(cx, cy - 20, cx, cy + 20, 0xFD20);
        display.fillCircle(cx, cy, 6, 0xFD20);
      } break;

      case 4: { // MOCHY JUMP: Layered Platforms
        display.drawFastHLine(cx - 36, cy + 18, 30, themeAccent);
        display.drawFastHLine(cx - 6,  cy + 2,  32, themeAccent);
        display.drawFastHLine(cx + 12, cy - 16, 28, themeAccent);
        display.fillCircle(cx + 10, cy - 26, 6, themeText);
        display.drawLine(cx + 10, cy - 8, cx + 10, cy - 18, 0x07E0);
        display.drawLine(cx + 7, cy - 14, cx + 10, cy - 18, 0x07E0);
        display.drawLine(cx + 13, cy - 14, cx + 10, cy - 18, 0x07E0);
      } break;

      case 5: { // STACKER: Monolithic Blocks
        display.drawFastVLine(cx, cy - 28, 56, 0xCE79);
        display.drawRect(cx - 24, cy + 12, 48, 10, themeAccent);
        display.drawRect(cx - 18, cy,      36, 10, themeAccent);
        display.drawRect(cx - 12, cy - 12, 24, 10, themeText);
        display.fillRect(cx - 6,  cy - 24, 12, 8,  themeAccent);
      } break;

      case 6: { // MEMORY MATRIX: Neural Grid
        display.drawRect(cx - 22, cy - 22, 44, 44, themeBorder);
        display.drawFastHLine(cx - 22, cy, 44, themeBorder);
        display.drawFastVLine(cx, cy - 22, 44, themeBorder);
        display.fillCircle(cx - 11, cy - 11, 5, 0x07E0);
        display.fillCircle(cx + 11, cy - 11, 5, themeAccent);
        display.fillCircle(cx - 11, cy + 11, 5, 0xFD20);
        display.fillCircle(cx + 11, cy + 11, 5, 0xF800);
      } break;

      case 7: { // EXIT ARCADE
        display.drawCircle(cx, cy, 24, 0xF800);
        display.drawLine(cx - 8, cy, cx + 8, cy, 0xF800);
        display.drawLine(cx - 8, cy, cx - 2, cy - 6, 0xF800);
        display.drawLine(cx - 8, cy, cx - 2, cy + 6, 0xF800);
      } break;
    }

    // ── Floating Action Trigger Button ($Y \in [194, 228]$) ─────────────────
    int btnW = 140;
    int btnH = 34;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = 194;

    uint16_t btnColor = (opt == 7) ? 0xF800 : themeAccent;
    display.fillRoundRect(btnX, btnY, btnW, btnH, 8, btnColor);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    const char* btnTxt = (opt == 7) ? "EXIT <" : "PLAY >";
    int bW = strlen(btnTxt) * 12;
    display.setCursor(btnX + (btnW - bW) / 2, btnY + 9);
    display.print(btnTxt);

    // ── Carousel Indicator Rail ($Y=238) ────────────────────────────────────
    int dotStartX = 72;
    for (int d = 0; d < 8; d++) {
      int dx = dotStartX + d * 14;
      if (d == opt) {
        display.fillRoundRect(dx - 4, 238, 12, 4, 2, btnColor);
      } else {
        display.fillCircle(dx, 240, 2, themeBorder);
      }
    }

    // Bottom navigation hint
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(26, 256);
    display.print("SWIPE: BROWSE // TAP: LAUNCH");
  }

  // --- Game 1: Luna Racer ---
  void updateAndDrawRacer(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int carW = SCREEN_WIDTH == 240 ? 16 : 8;
    int carH = SCREEN_WIDTH == 240 ? 28 : 14;

    if (!racGameOver) {
      if (btn1 && btn2) {
        if (racNitroFuel > 0) {
          racNitroActive = true;
          racSpeed = SCREEN_WIDTH == 240 ? 8.0f : 5.0f;
          racNitroFuel -= 0.5f;
          if (millis() % 200 < 100) {
            audio.playSound(SOUND_JUMP);
          }
        } else {
          racNitroActive = false;
          racSpeed = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
        }
      } else {
        racNitroActive = false;
        racSpeed = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
      }

      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2) {
        steerDelta -= 3.5f;
      } else if (btn2 && !btn1) {
        steerDelta += 3.5f;
      }
      if (fabsf(tilt) > 0.01f) {
        steerDelta += tilt * 5.2f;
      }
      racPlayerX += steerDelta;
      if (racPlayerX < G_X + 16) racPlayerX = G_X + 16;
      if (racPlayerX > G_R - 16 - carW) racPlayerX = G_R - 16 - carW;

      racRoadScroll += racSpeed;
      if (racRoadScroll >= 40) racRoadScroll = 0;
      racScore += racSpeed / 5;

      for (int i = 0; i < 4; i++) {
        if (racCarActive[i]) {
          racCarY[i] += (racSpeed - racCarSpeed[i]);
          
          if (racCarY[i] > G_B + 10) {
            racCarY[i] = G_Y - 30;
            racCarX[i] = G_X + 24 + random(0, 3) * (G_W - 48)/3 - carW / 2;
            racCarSpeed[i] = 1.0f + random(0, 20) / 10.0f;
            racCarActive[i] = true;
          }

          if (abs(racPlayerX - racCarX[i]) < (SCREEN_WIDTH == 240 ? 14 : 7) && 
              abs(racPlayerY - racCarY[i]) < (SCREEN_WIDTH == 240 ? 26 : 13)) {
            racGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("rac", racHighScore, racScore);
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetRacer();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillScreen(0x39E7); // Grass green
    display.fillRect(G_X + 16, G_Y, G_W - 32, G_H, 0x4208); // Asphalt road
    display.drawFastVLine(G_X + 16, G_Y, G_H, TFT_WHITE);
    display.drawFastVLine(G_R - 16, G_Y, G_H, TFT_WHITE);

    // Grass pattern dots
    for (int y = G_Y; y < G_B; y += 30) {
      display.fillRect(G_X + 4, y, 4, 4, 0x2284);
      display.fillRect(G_R - 8, y, 4, 4, 0x2284);
    }

    for (int y = G_Y - 20; y < G_B; y += 40) {
      float scrollY = y + (int)racRoadScroll;
      if (scrollY >= G_Y && scrollY < G_B - 20) {
        display.fillRect(G_X + G_W / 2 - 1, scrollY, 2, SCREEN_WIDTH == 240 ? 20 : 10, TFT_YELLOW);
      }
    }

    uint16_t carColors[] = {TFT_RED, TFT_BLUE, TFT_GREEN};
    for (int i = 0; i < 4; i++) {
      if (racCarActive[i] && racCarY[i] >= G_Y && racCarY[i] < G_B) {
        display.fillRect(racCarX[i], racCarY[i], carW, carH, carColors[racCarColor[i]]);
        display.fillRect(racCarX[i] + carW/4, racCarY[i] + carH/5, carW/2, carH/5, TFT_CYAN); // Glass
        display.fillRect(racCarX[i] + carW/8, racCarY[i] + carH - carH/7, carW/4, carH/8, TFT_RED); // lights
        display.fillRect(racCarX[i] + carW - 3*carW/8, racCarY[i] + carH - carH/7, carW/4, carH/8, TFT_RED);
      }
    }

    display.fillRect(racPlayerX, racPlayerY, carW, carH, 0xFDA0); // Player orange
    display.fillRect(racPlayerX + carW/4, racPlayerY + carH/5, carW/2, carH/5, TFT_WHITE);
    display.fillRect(racPlayerX + carW/8, racPlayerY + carH - carH/7, carW/4, carH/8, TFT_RED);
    display.fillRect(racPlayerX + carW - 3*carW/8, racPlayerY + carH - carH/7, carW/4, carH/8, TFT_RED);

    if (racNitroActive && (millis() % 100 < 50)) {
      display.fillTriangle(racPlayerX + carW/4, racPlayerY + carH, racPlayerX + carW/2, racPlayerY + carH + 8, racPlayerX + 3*carW/4, racPlayerY + carH, TFT_ORANGE);
    }

    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("DIST:%04d", racScore);

    display.setCursor(SCREEN_WIDTH / 2 + 10, SCREEN_WIDTH == 240 ? 36 : 7);
    display.print("NTR:");
    display.drawRect(SCREEN_WIDTH - 36, SCREEN_WIDTH == 240 ? 39 : 9, 30, SCREEN_WIDTH == 240 ? 10 : 5, themeBorder);
    display.fillRect(SCREEN_WIDTH - 35, SCREEN_WIDTH == 240 ? 40 : 10, (int)(racNitroFuel * 28.0f / 100.0f), SCREEN_WIDTH == 240 ? 8 : 3, TFT_RED);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (racGameOver) {
      drawGameOverScreen(display, racScore, racHighScore);
    }
  }

  // --- Game 2: Luna Space ---
  void updateAndDrawSpace(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int pw = SCREEN_WIDTH == 240 ? 12 : 6;
    int ph = SCREEN_WIDTH == 240 ? 16 : 8;
    int ew = SCREEN_WIDTH == 240 ? 12 : 6;
    int eh = SCREEN_WIDTH == 240 ? 8 : 4;
    int playerColW = SCREEN_WIDTH == 240 ? 16 : 10;
    int playerColH = SCREEN_WIDTH == 240 ? 12 : 6;
    int enemyColW = SCREEN_WIDTH == 240 ? 18 : 10;
    int enemyColH = SCREEN_WIDTH == 240 ? 14 : 8;

    if (!spcGameOver) {
      if (btn1 && btn2 && (millis() - spcSmartBombCooldown > 8000)) {
        spcSmartBombCooldown = millis();
        audio.playSound(SOUND_GAMEOVER);
        for (int i = 0; i < 4; i++) spcEnemyLaserActive[i] = false;
        for (int i = 0; i < 8; i++) {
          if (spcEnemyActive[i]) {
            spcEnemyHP[i]--;
            if (spcEnemyHP[i] <= 0) {
              spcEnemyActive[i] = false;
              spcScore += 20;
            }
          }
        }
        if (spcBossActive) spcBossHP -= 3;
        for (int p = 0; p < 15; p++) {
          spcPartActive[p] = true;
          spcPartX[p] = G_X + G_W/2;
          spcPartY[p] = G_Y + G_H/2;
          spcPartVX[p] = random(-40, 41) / 10.0f;
          spcPartVY[p] = random(-40, 41) / 10.0f;
          spcPartLife[p] = 12;
        }
      }
      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2) {
        steerDelta -= 3.5f;
      } else if (btn2 && !btn1) {
        steerDelta += 3.5f;
      }
      if (fabsf(tilt) > 0.01f) {
        steerDelta += tilt * 5.5f;
      }
      spcPlayerX += steerDelta;
      if (spcPlayerX < G_X + pw + 4) spcPlayerX = G_X + pw + 4;
      if (spcPlayerX > G_R - pw - 4) spcPlayerX = G_R - pw - 4;

      if (millis() - spcLastShoot > 350) {
        spcLastShoot = millis();
        audio.playSound(SOUND_CHIRP);
        if (spcSpreadActive) {
          float angles[] = {-2.0f, 0.0f, 2.0f};
          for (int a = 0; a < 3; a++) {
            for (int i = 0; i < 6; i++) {
              if (!spcLaserActive[i]) {
                spcLaserActive[i] = true;
                spcLaserX[i] = spcPlayerX + angles[a] * 4.0f;
                spcLaserY[i] = spcPlayerY - ph;
                break;
              }
            }
          }
        } else {
          for (int i = 0; i < 6; i++) {
            if (!spcLaserActive[i]) {
              spcLaserActive[i] = true;
              spcLaserX[i] = spcPlayerX;
              spcLaserY[i] = spcPlayerY - ph;
              break;
            }
          }
        }
      }

      for (int i = 0; i < 6; i++) {
        if (spcLaserActive[i]) {
          spcLaserY[i] -= 4.0f;
          if (spcLaserY[i] < G_Y) {
            spcLaserActive[i] = false;
          }
        }
      }

      for (int i = 0; i < 4; i++) {
        if (spcEnemyLaserActive[i]) {
          spcEnemyLaserY[i] += 2.5f;
          if (spcEnemyLaserY[i] > G_B) {
            spcEnemyLaserActive[i] = false;
          }

          if (abs(spcEnemyLaserX[i] - spcPlayerX) < playerColW && 
              spcEnemyLaserY[i] >= spcPlayerY - playerColH && 
              spcEnemyLaserY[i] <= spcPlayerY + playerColH/2) {
            spcEnemyLaserActive[i] = false;
            if (millis() > spcShieldTime) {
              audio.playSound(SOUND_POWERDOWN);
              spcHealth--;
              if (spcHealth <= 0) {
                spcGameOver = true;
                audio.playSound(SOUND_GAMEOVER);
                saveHighScore("spc", spcHighScore, spcScore);
              }
            } else {
              audio.playSound(SOUND_COIN);
            }
          }
        }
      }

      bool anyEnemyOnScreen = false;
      for (int i = 0; i < 8; i++) {
        if (spcEnemyActive[i]) {
          anyEnemyOnScreen = true;
          spcEnemyY[i] += 0.5f;
          spcEnemyX[i] += sin(millis() / 200.0f + i) * 0.8f;
          
          if (spcEnemyY[i] > G_B + 10) {
            spcEnemyActive[i] = false;
            spcHealth--;
            if (spcHealth <= 0) {
              spcGameOver = true;
              audio.playSound(SOUND_GAMEOVER);
              saveHighScore("spc", spcHighScore, spcScore);
            }
          }

          if (millis() - spcEnemyLastShoot[i] > 3000) {
            spcEnemyLastShoot[i] = millis();
            for (int el = 0; el < 4; el++) {
              if (!spcEnemyLaserActive[el]) {
                spcEnemyLaserActive[el] = true;
                spcEnemyLaserX[el] = spcEnemyX[i];
                spcEnemyLaserY[el] = spcEnemyY[i] + eh;
                break;
              }
            }
          }

          for (int l = 0; l < 6; l++) {
            if (spcLaserActive[l]) {
              if (abs(spcLaserX[l] - spcEnemyX[i]) < enemyColW && 
                  abs(spcLaserY[l] - spcEnemyY[i]) < enemyColH) {
                spcLaserActive[l] = false;
                spcEnemyHP[i]--;
                audio.playSound(SOUND_JUMP);
                if (spcEnemyHP[i] <= 0) {
                  spcEnemyActive[i] = false;
                  spcScore += 20;
                  if (random(0, 10) < 3 && !spcPowerActive) {
                    spcPowerActive = true;
                    spcPowerX = spcEnemyX[i];
                    spcPowerY = spcEnemyY[i];
                    spcPowerType = random(0, 2);
                  }
                  for (int p = 0; p < 3; p++) {
                    int pIdx = random(0, 15);
                    spcPartActive[pIdx] = true;
                    spcPartX[pIdx] = spcEnemyX[i];
                    spcPartY[pIdx] = spcEnemyY[i];
                    spcPartVX[pIdx] = random(-20, 21) / 10.0f;
                    spcPartVY[pIdx] = random(-20, 21) / 10.0f;
                    spcPartLife[pIdx] = 6;
                  }
                }
              }
            }
          }
        }
      }

      if (!anyEnemyOnScreen && !spcBossActive) {
        if (spcScore >= 180) {
          spcBossActive = true;
          spcBossHP = 20;
        } else {
          for (int i = 0; i < 4; i++) {
            spcEnemyActive[i] = true;
            spcEnemyX[i] = G_X + 24 + i * (G_W - 48)/4;
            spcEnemyY[i] = G_Y - 20;
            spcEnemyHP[i] = 1;
            spcEnemyType[i] = i % 2;
            spcEnemyLastShoot[i] = millis();
          }
        }
      }

      if (spcPowerActive) {
        spcPowerY += 1.2f;
        if (spcPowerY > G_B) spcPowerActive = false;
        
        if (abs(spcPowerX - spcPlayerX) < 16 && abs(spcPowerY - spcPlayerY) < 16) {
          spcPowerActive = false;
          audio.playSound(SOUND_POWERUP);
          if (spcPowerType == 0) spcSpreadActive = true;
          else spcShieldTime = millis() + 5000;
        }
      }

      for (int i = 0; i < 15; i++) {
        if (spcPartActive[i]) {
          spcPartX[i] += spcPartVX[i];
          spcPartY[i] += spcPartVY[i];
          spcPartLife[i]--;
          if (spcPartLife[i] <= 0) spcPartActive[i] = false;
        }
      }

      if (spcBossActive) {
        spcBossX += spcBossDir * 0.8f;
        if (spcBossX < G_X + 32) spcBossDir = 1;
        if (spcBossX > G_R - 32) spcBossDir = -1;

        if (millis() - spcBossLastShoot > 1800) {
          spcBossLastShoot = millis();
          for (int l = 0; l < 2; l++) {
            for (int el = 0; el < 4; el++) {
              if (!spcEnemyLaserActive[el]) {
                spcEnemyLaserActive[el] = true;
                spcEnemyLaserX[el] = spcBossX + (l == 0 ? -12 : 12);
                spcEnemyLaserY[el] = spcBossY + 8;
                break;
              }
            }
          }
        }

        int bw = SCREEN_WIDTH == 240 ? 32 : 16;
        int bh = SCREEN_WIDTH == 240 ? 16 : 8;

        for (int l = 0; l < 6; l++) {
          if (spcLaserActive[l]) {
            if (abs(spcLaserX[l] - spcBossX) < bw + 4 && 
                abs(spcLaserY[l] - spcBossY) < bh + 4) {
              spcLaserActive[l] = false;
              spcBossHP--;
              audio.playSound(SOUND_JUMP);
              if (spcBossHP <= 0) {
                spcBossActive = false;
                spcScore += 200;
                spcGameOver = true;
                spcHealth = 99;
                audio.playSound(SOUND_POWERUP);
                saveHighScore("spc", spcHighScore, spcScore);
              }
            }
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetSpace();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillScreen(0x0002); // Deep Space dark blue

    // Starfield
    for (int i = 0; i < 8; i++) {
      int sx = (i * 27) % G_W;
      int sy = (G_Y + (i * 35) + (millis() / 20)) % G_H + G_Y;
      display.drawPixel(sx, sy, 0x7BEF);
    }

    for (int i = 0; i < 6; i++) {
      if (spcLaserActive[i]) {
        display.fillRect(spcLaserX[i], spcLaserY[i], 2, SCREEN_WIDTH == 240 ? 8 : 4, TFT_CYAN);
      }
    }

    for (int i = 0; i < 4; i++) {
      if (spcEnemyLaserActive[i]) {
        display.fillRect(spcEnemyLaserX[i], spcEnemyLaserY[i], 2, SCREEN_WIDTH == 240 ? 8 : 4, TFT_RED);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (spcEnemyActive[i]) {
        display.fillTriangle(spcEnemyX[i], spcEnemyY[i]+eh, spcEnemyX[i]-ew, spcEnemyY[i]-eh, spcEnemyX[i]+ew, spcEnemyY[i]-eh, TFT_MAGENTA);
      }
    }

    if (spcPowerActive) {
      int pwSize = SCREEN_WIDTH == 240 ? 12 : 8;
      display.fillRect(spcPowerX - pwSize/2, spcPowerY - pwSize/2, pwSize, pwSize, spcPowerType == 0 ? TFT_YELLOW : TFT_GREEN);
      display.setTextColor(TFT_BLACK);
      display.setTextSize(1);
      display.setCursor(spcPowerX - 3, spcPowerY - 3);
      display.print(spcPowerType == 0 ? "W" : "S");
    }

    for (int i = 0; i < 15; i++) {
      if (spcPartActive[i]) {
        display.drawPixel(spcPartX[i], spcPartY[i], TFT_YELLOW);
      }
    }

    if (spcBossActive) {
      int bw = SCREEN_WIDTH == 240 ? 32 : 16;
      int bh = SCREEN_WIDTH == 240 ? 16 : 8;
      display.fillRoundRect(spcBossX - bw, spcBossY - bh, bw * 2, bh * 2, 4, TFT_RED);
      display.fillRect(spcBossX - bw/4, spcBossY + bh, bw/2, bh/2, TFT_YELLOW);
      
      display.drawFastHLine(spcBossX - bw + 2, spcBossY - bh - 6, (bw - 2) * 2, TFT_BLACK);
      display.drawFastHLine(spcBossX - bw + 2, spcBossY - bh - 6, spcBossHP * (bw - 2) * 2 / 20, TFT_GREEN);
    }

    display.fillTriangle(spcPlayerX, spcPlayerY - ph, spcPlayerX - pw, spcPlayerY + ph/2, spcPlayerX + pw, spcPlayerY + ph/2, themeAccent);
    
    if (millis() < spcShieldTime) {
      display.drawCircle(spcPlayerX, spcPlayerY - ph/4, SCREEN_WIDTH == 240 ? 20 : 10, TFT_GREEN);
    }

    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", spcScore);

    int lifeX = SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 54 : 32);
    for (int i = 0; i < 3; i++) {
      int hx = lifeX + i * (SCREEN_WIDTH == 240 ? 14 : 7);
      int hy = SCREEN_WIDTH == 240 ? 42 : 10;
      display.fillCircle(hx, hy, SCREEN_WIDTH == 240 ? 3 : 2, i < spcHealth ? TFT_RED : themeBorder);
    }
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (spcGameOver) {
      if (spcHealth == 99) {
        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);
        display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
        display.setTextColor(TFT_GREEN);
        display.setCursor((SCREEN_WIDTH - 8 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, G_Y + 20);
        display.print("VICTORY!");
        
        display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
        display.setTextColor(themeText);
        display.setCursor(20, G_Y + 60);
        display.print("SPACE BOSS DESTROYED!");
        
        display.setTextColor(themeAccent);
        display.setCursor(20, G_Y + 90);
        display.print("B1: PLAY AGAIN");
      } else {
        drawGameOverScreen(display, spcScore, spcHighScore);
      }
    }
  }

  // --- Game 3: Flappy Mochy ---
  void updateAndDrawFlappy(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int mSize = SCREEN_WIDTH == 240 ? 8 : 4;
    int pW = SCREEN_WIDTH == 240 ? 24 : 16;
    int gapH = SCREEN_WIDTH == 240 ? 50 : 35;

    if (!flapGameOver) {
      if (btn2 && !lastBtn2) {
        flapPlayerVY = SCREEN_WIDTH == 240 ? -3.8f : -2.8f;
        audio.playSound(SOUND_JUMP);
      }

      flapPlayerVY += SCREEN_WIDTH == 240 ? 0.22f : 0.18f; // Gravity
      flapPlayerY += flapPlayerVY;

      flapPipeX -= SCREEN_WIDTH == 240 ? 2.2f : 1.5f;
      if (flapPipeX < G_X - pW) {
        flapPipeX = G_R;
        flapPipeGapY = G_Y + 20 + random(0, G_H - gapH - 40);
        flapScore++;
        audio.playSound(SOUND_CHIRP);
      }

      if (flapPlayerY < G_Y || flapPlayerY > G_B - mSize * 2) {
        flapGameOver = true;
        audio.playSound(SOUND_GAMEOVER);
        saveHighScore("flap", flapHighScore, flapScore);
      }

      if (flapPipeX > G_X + G_W / 2 - pW && flapPipeX < G_X + G_W / 2 + mSize) {
        if (flapPlayerY < flapPipeGapY || flapPlayerY > flapPipeGapY + gapH - mSize * 2) {
          flapGameOver = true;
          audio.playSound(SOUND_GAMEOVER);
          saveHighScore("flap", flapHighScore, flapScore);
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetFlappy();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillRect(G_X, G_Y, G_W, G_H, 0x7E5F); // Sky

    // Draw clouds
    display.fillCircle(SCREEN_WIDTH/6, G_Y + G_H/4, SCREEN_WIDTH/20, TFT_WHITE);
    display.fillCircle(SCREEN_WIDTH/6 + 10, G_Y + G_H/4, SCREEN_WIDTH/15, TFT_WHITE);
    display.fillCircle(SCREEN_WIDTH/6 + 20, G_Y + G_H/4, SCREEN_WIDTH/20, TFT_WHITE);

    display.fillRect(G_X, G_B - 6, G_W, 6, 0x03E0); // grass ground

    // Pipes
    if (flapPipeX >= G_X && flapPipeX < G_R) {
      uint16_t pipeColor = 0x1AE2;
      // Top
      display.fillRect(flapPipeX, G_Y, pW, flapPipeGapY - G_Y, pipeColor);
      display.drawRect(flapPipeX, G_Y, pW, flapPipeGapY - G_Y, TFT_BLACK);
      display.fillRect(flapPipeX - 2, flapPipeGapY - 8, pW + 4, 8, pipeColor);
      display.drawRect(flapPipeX - 2, flapPipeGapY - 8, pW + 4, 8, TFT_BLACK);
      // Bottom
      display.fillRect(flapPipeX, flapPipeGapY + gapH, pW, G_B - (flapPipeGapY + gapH), pipeColor);
      display.drawRect(flapPipeX, flapPipeGapY + gapH, pW, G_B - (flapPipeGapY + gapH), TFT_BLACK);
      display.fillRect(flapPipeX - 2, flapPipeGapY + gapH, pW + 4, 8, pipeColor);
      display.drawRect(flapPipeX - 2, flapPipeGapY + gapH, pW + 4, 8, TFT_BLACK);
    }

    // Player Mochy
    int px = G_X + G_W / 2 - mSize;
    display.fillCircle(px + mSize, flapPlayerY + mSize, mSize, TFT_YELLOW);
    display.fillCircle(px + mSize + mSize/2, flapPlayerY + mSize/2, mSize/4, TFT_BLACK);
    display.fillRect(px, flapPlayerY + mSize, mSize/2, mSize/3, TFT_ORANGE);

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SCORE:%03d", flapScore);

    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54), SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HI:%03d", flapHighScore);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (flapGameOver) {
      drawGameOverScreen(display, flapScore, flapHighScore);
    }
  }

  // --- Game 4: Coin Catcher ---
  void updateAndDrawCatcher(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int basketW = SCREEN_WIDTH == 240 ? 24 : 16;
    int coinR = SCREEN_WIDTH == 240 ? 5 : 3;

    if (!catGameOver) {
      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2) {
        steerDelta -= 4.0f;
      } else if (btn2 && !btn1) {
        steerDelta += 4.0f;
      }
      if (fabsf(tilt) > 0.01f) {
        steerDelta += tilt * 6.0f;
      }
      catPlayerX += steerDelta;
      if (catPlayerX < G_X + 8) catPlayerX = G_X + 8;
      if (catPlayerX > G_R - basketW - 8) catPlayerX = G_R - basketW - 8;

      float fallSpeed = SCREEN_WIDTH == 240 ? (2.0f + catScore / 80.0f) : (1.4f + catScore / 80.0f);
      for (int i = 0; i < 3; i++) {
        if (catCoinsActive[i]) {
          catCoinsY[i] += fallSpeed;
          
          if (catCoinsY[i] > G_B - 6) {
            if (!catIsBomb[i]) {
              catLives--;
              audio.playSound(SOUND_POWERDOWN);
              if (catLives <= 0) {
                catGameOver = true;
                audio.playSound(SOUND_GAMEOVER);
                saveHighScore("cat", catHighScore, catScore);
              }
            }
            catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
            catCoinsY[i] = G_Y - 20;
            catIsBomb[i] = (random(0, 10) < 3);
          }

          if (catCoinsY[i] >= G_B - 20 && catCoinsY[i] <= G_B - 10) {
            if (catCoinsX[i] + coinR >= catPlayerX && catCoinsX[i] <= catPlayerX + basketW) {
              if (catIsBomb[i]) {
                catLives--;
                audio.playSound(SOUND_POWERDOWN);
                if (catLives <= 0) {
                  catGameOver = true;
                  audio.playSound(SOUND_GAMEOVER);
                  saveHighScore("cat", catHighScore, catScore);
                }
              } else {
                catScore += 10;
                audio.playSound(SOUND_COIN);
              }
              catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
              catCoinsY[i] = G_Y - 20;
              catIsBomb[i] = (random(0, 10) < 3);
            }
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetCatcher();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillRect(G_X, G_Y, G_W, G_H, 0x5D1B); // Indigo/Blue sky
    display.drawFastHLine(G_X, G_B - 8, G_W, TFT_GREEN); // green floor

    // Basket
    display.fillRoundRect(catPlayerX, G_B - 14, basketW, 6, 2, 0xC3A5);
    display.fillRect(catPlayerX + basketW/4, G_B - 8, basketW/2, 2, 0x8200);

    // Coins & Bombs
    for (int i = 0; i < 3; i++) {
      if (catCoinsActive[i]) {
        if (catIsBomb[i]) {
          display.fillCircle(catCoinsX[i] + coinR, catCoinsY[i] + coinR, coinR, TFT_BLACK);
          display.fillRect(catCoinsX[i] + coinR - 1, catCoinsY[i] - 1, 2, 2, TFT_ORANGE);
        } else {
          display.fillCircle(catCoinsX[i] + coinR, catCoinsY[i] + coinR, coinR, 0xFDA0);
          display.drawCircle(catCoinsX[i] + coinR, catCoinsY[i] + coinR, coinR, TFT_YELLOW);
        }
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", catScore);

    int lifeX = SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 54 : 32);
    for (int i = 0; i < 3; i++) {
      int hx = lifeX + i * (SCREEN_WIDTH == 240 ? 14 : 7);
      int hy = SCREEN_WIDTH == 240 ? 42 : 10;
      display.fillCircle(hx, hy, SCREEN_WIDTH == 240 ? 3 : 2, i < catLives ? TFT_RED : themeBorder);
    }
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (catGameOver) {
      drawGameOverScreen(display, catScore, catHighScore);
    }
  }

  // --- Game 5: Mochy Jump ---
  void updateAndDrawJump(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int platW = SCREEN_WIDTH == 240 ? 36 : 24;
    int pW = SCREEN_WIDTH == 240 ? 12 : 8;
    int pH = SCREEN_WIDTH == 240 ? 16 : 10;

    if (!jumpGameOver) {
      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2) {
        steerDelta -= 3.2f;
      } else if (btn2 && !btn1) {
        steerDelta += 3.2f;
      }
      if (fabsf(tilt) > 0.01f) {
        steerDelta += tilt * 5.2f;
      }
      jumpPlayerX += steerDelta;

      if (jumpPlayerX < G_X - pW) jumpPlayerX = G_R - pW;
      if (jumpPlayerX > G_R + pW) jumpPlayerX = G_X - pW;

      jumpPlayerVY += 0.18f;
      jumpPlayerY += jumpPlayerVY;

      if (jumpPlayerY < G_Y + G_H / 2) {
        float diff = (G_Y + G_H / 2) - jumpPlayerY;
        jumpPlayerY = G_Y + G_H / 2;
        jumpScore += diff / 2;
        
        for (int i = 0; i < 5; i++) {
          jumpPlatY[i] += diff;
          if (jumpPlatY[i] > G_B) {
            jumpPlatY[i] = G_Y - 10 + random(0, 15);
            jumpPlatX[i] = G_X + 10 + random(0, G_W - platW - 10);
          }
        }
      }

      if (jumpPlayerVY > 0) {
        for (int i = 0; i < 5; i++) {
          if (jumpPlayerX + pW >= jumpPlatX[i] && jumpPlayerX <= jumpPlatX[i] + platW) {
            if (jumpPlayerY + pH >= jumpPlatY[i] && jumpPlayerY + pH - 4 <= jumpPlatY[i] + 4) {
              jumpPlayerVY = SCREEN_WIDTH == 240 ? -5.8f : -4.8f;
              audio.playSound(SOUND_JUMP);
              break;
            }
          }
        }
      }

      if (jumpPlayerY > G_B) {
        jumpGameOver = true;
        audio.playSound(SOUND_GAMEOVER);
        saveHighScore("jump", jumpHighScore, jumpScore);
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetJump();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillScreen(0x000F); // grid sheet dark background
    for (int x = 20; x < G_W; x += 20) {
      display.drawFastVLine(x, G_Y, G_H, 0x0842);
    }
    for (int y = G_Y + 20; y < G_B; y += 20) {
      display.drawFastHLine(G_X, y, G_W, 0x0842);
    }

    // Platforms
    for (int i = 0; i < 5; i++) {
      display.fillRoundRect(jumpPlatX[i], jumpPlatY[i], platW, 4, 1, TFT_GREEN);
      display.drawRoundRect(jumpPlatX[i], jumpPlatY[i], platW, 4, 1, TFT_WHITE);
    }

    // Player Jumper
    display.fillRoundRect(jumpPlayerX, jumpPlayerY, pW, pH, 2, TFT_YELLOW);
    display.fillCircle(jumpPlayerX + pW/3, jumpPlayerY + pH/4, 1, TFT_BLACK);
    display.fillCircle(jumpPlayerX + 2*pW/3, jumpPlayerY + pH/4, 1, TFT_BLACK);

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("ALT:%04d", jumpScore);

    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54), SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HI:%04d", jumpHighScore);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (jumpGameOver) {
      drawGameOverScreen(display, jumpScore, jumpHighScore);
    }
  }

  // --- Game 6: Stacker ---
  void updateAndDrawStacker(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int blockH = SCREEN_WIDTH == 240 ? 12 : 8;

    if (!stkGameOver) {
      stkBlockX += stkDir * stkSpeed;
      if (stkBlockX < G_X + 6) {
        stkBlockX = G_X + 6;
        stkDir = 1;
      }
      if (stkBlockX + stkBlockW > G_R - 6) {
        stkBlockX = G_R - 6 - stkBlockW;
        stkDir = -1;
      }

      if (btn2 && !lastBtn2) {
        if (stkStackHeight == 0) {
          stkBaseX = stkBlockX;
          stkStackX[0] = stkBlockX;
          stkStackW[0] = stkBlockW;
          stkStackHeight = 1;
          stkScore += 10;
          audio.playSound(SOUND_COIN);
        } else {
          float prevX = stkStackX[stkStackHeight - 1];
          float prevW = stkStackW[stkStackHeight - 1];

          float overlapLeft = max(stkBlockX, prevX);
          float overlapRight = min(stkBlockX + stkBlockW, prevX + prevW);
          float overlapW = overlapRight - overlapLeft;

          if (overlapW <= 0) {
            stkGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("stk", stkHighScore, stkScore);
          } else {
            stkBlockW = overlapW;
            stkBlockX = overlapLeft;
            
            if (stkStackHeight < 10) {
              stkStackX[stkStackHeight] = overlapLeft;
              stkStackW[stkStackHeight] = overlapW;
              stkStackHeight++;
            } else {
              for (int i = 0; i < 9; i++) {
                stkStackX[i] = stkStackX[i+1];
                stkStackW[i] = stkStackW[i+1];
              }
              stkStackX[9] = overlapLeft;
              stkStackW[9] = overlapW;
            }
            stkScore += 10;
            audio.playSound(SOUND_COIN);
            stkSpeed += 0.25f;
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetStacker();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillRect(G_X, G_Y, G_W, G_H, 0x1804); // Neon dark violet
    for (int y = G_Y; y < G_B; y += blockH + 2) {
      display.drawFastHLine(G_X, y, G_W, 0x2104);
    }

    for (int i = 0; i < stkStackHeight; i++) {
      int blockY = G_B - (blockH + 2) - i * (blockH + 2);
      display.fillRoundRect(stkStackX[i], blockY, stkStackW[i], blockH, 2, themeAccent);
      display.drawRoundRect(stkStackX[i], blockY, stkStackW[i], blockH, 2, TFT_WHITE);
    }

    if (!stkGameOver) {
      int curY = G_B - (blockH + 2) - stkStackHeight * (blockH + 2);
      display.fillRoundRect(stkBlockX, curY, stkBlockW, blockH, 2, TFT_YELLOW);
      display.drawRoundRect(stkBlockX, curY, stkBlockW, blockH, 2, TFT_WHITE);
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("STACK:%03d", stkScore);

    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54), SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HIGH:%03d", stkHighScore);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (stkGameOver) {
      drawGameOverScreen(display, stkScore, stkHighScore);
    }
  }

  // --- Game 7: Memory Matrix ---
  void updateAndDrawMemory(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = virtualBtn1;
    bool btn2 = virtualBtn2;

    int gridSpacing = SCREEN_WIDTH == 240 ? 42 : 28;
    int gridStartX = G_X + (G_W - 3 * gridSpacing) / 2;
    int gridStartY = G_Y + (G_H - 3 * gridSpacing) / 2;

    if (!memGameOver) {
      if (memState == 0) {
        unsigned long elapsed = millis() - memTimer;
        int stepTime = 600;
        int gapTime = 200;
        int cycleTime = stepTime + gapTime;
        int currentStep = elapsed / cycleTime;

        if (currentStep < memSeqLen) {
          int stepPhase = elapsed % cycleTime;
          if (stepPhase < stepTime) {
            memSeqStep = memSeq[currentStep];
          } else {
            memSeqStep = -1;
          }
        } else {
          memState = 1;
          memPlayerStep = 0;
          memSeqStep = -1;
        }
      }
      else if (memState == 1) {
        if (btn1 && !lastBtn1) {
          memSelectedTile = (memSelectedTile + 1) % 9;
          audio.playSound(SOUND_CHIRP);
        }

        if (btn2 && !lastBtn2) {
          if (memSelectedTile == memSeq[memPlayerStep]) {
            audio.playSound(SOUND_JUMP);
            memPlayerStep++;
            if (memPlayerStep == memSeqLen) {
              memScore += 10;
              memSeqLen++;
              if (memSeqLen > 15) memSeqLen = 15;
              memSeq[memSeqLen - 1] = random(0, 9);
              memState = 0;
              memTimer = millis();
            }
          } else {
            memGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("mem", memHighScore, memScore);
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetMemory();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    display.fillRect(G_X, G_Y, G_W, G_H, 0x10A2); // Green cyber grid

    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        int tileIdx = r * 3 + c;
        int tx = gridStartX + c * gridSpacing + 3;
        int ty = gridStartY + r * gridSpacing + 3;
        int tw = gridSpacing - 6;
        int th = gridSpacing - 6;

        uint16_t tileColor = 0x0182; // Dim green
        if (memState == 0 && memSeqStep == tileIdx) {
          tileColor = TFT_YELLOW;
        }
        else if (memState == 1 && memSelectedTile == tileIdx) {
          tileColor = themeAccent;
        }

        display.fillRoundRect(tx, ty, tw, th, 4, tileColor);
        display.drawRoundRect(tx, ty, tw, th, 4, TFT_WHITE);
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, themeBg);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", memScore);

    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 120 : 68), SCREEN_WIDTH == 240 ? 36 : 7);
    if (memState == 0) {
      display.setTextColor(TFT_RED);
      display.print("WATCH SEQ");
    } else {
      display.setTextColor(0x03E0);
      display.printf("STEP:%d/%d", memPlayerStep + 1, memSeqLen);
    }
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (memGameOver) {
      drawGameOverScreen(display, memScore, memHighScore);
    }
  }
};

#endif // GAMES_H
