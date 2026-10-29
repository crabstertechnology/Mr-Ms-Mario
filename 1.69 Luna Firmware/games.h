#ifndef GAMES_H
#define GAMES_H

#include "audio.h"
#include "config.h"
#include "imu.h"
#include "hideseek_title_bg.h"
#include "hideseek_timer_bg.h"
#include "hideseek_gohide_bg.h"
#include "hideseek_active_bg.h"
#include "hideseek_win_bg.h"
#include "hideseek_exploded_bg.h"
#include "tod_title_bg.h"
#include "tod_choice_bg.h"
#include "tod_truth_bg.h"
#include "tod_dare_bg.h"
#include "imposter_bg.h"
#include "charades_bg.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Arduino.h>
#include <Preferences.h>

// Forward declarations for touch coordinates provided by main sketch
int getTouchX();
int getTouchY();
bool isTouchActive();

// External references (declared once)
extern bool gamesActive;
extern bool gamePlaying;
extern bool gameOverActive;
extern int gameMenuOption;
extern int gameSelected;
extern float gamesScrollPx;
extern bool virtualBtn1;
extern bool virtualBtn2;
extern LunaIMU imu;
extern String robotVariant;
extern bool negativeDisplay;
extern bool silentMode;
extern int appLauncherPage;

// ─── Inline color helpers ────────────────────────────────────────────────────
// Blend two RGB565 colors at 50%
static inline uint16_t blend565(uint16_t a, uint16_t b) {
  return (uint16_t)(((((a >> 11) & 0x1F) + ((b >> 11) & 0x1F)) >> 1) << 11 |
                    ((((a >> 5) & 0x3F) + ((b >> 5) & 0x3F)) >> 1) << 5 |
                    (((a & 0x1F) + (b & 0x1F)) >> 1));
}

class LunaGames {
private:
  // ── High Scores ────────────────────────────────────────────────────────────
  int racHighScore;
  int spcHighScore;
  int flapHighScore;
  int catHighScore;
  int jumpHighScore;
  int stkHighScore;
  int memHighScore;
  int hsHighScore;

  // ── Game 6: Hide & Seek ────────────────────────────────────────────────────
  enum HideSeekState {
    HS_STATE_TITLE = 0,
    HS_STATE_SETUP,
    HS_STATE_HIDING,
    HS_STATE_ACTIVE,
    HS_STATE_EXPLODED,
    HS_STATE_DEFUSED
  };
  HideSeekState hsState;
  int hsTimerSeconds;
  unsigned long hsLastCountdownTickMs;
  unsigned long hsPhaseStartTime;
  unsigned long hsBombDurationMs;
  unsigned long hsHidingDurationMs;
  unsigned long hsLastTickMs;
  long hsDefusedRemainingMs;
  bool hsGameOver;
  bool hsLastTouchDown;
  int hsSoundWaveRadius;
  uint8_t hsSparkFrame;
  int hsExplosionFrame;
  float hsExplosionPartX[24];
  float hsExplosionPartY[24];
  float hsExplosionPartVX[24];
  float hsExplosionPartVY[24];
  uint16_t hsExplosionPartCol[24];

  // ── Game 1: Luna Racer ─────────────────────────────────────────────────────
  float racPlayerX;
  float racPlayerY;
  float racSpeed;
  bool racNitroActive;
  float racNitroFuel;
  float racRoadScroll;
  int racScore;
  float racDistAccum;
  bool racGameOver;
  float racCarX[4];
  float racCarY[4];
  float racCarSpeed[4];
  int racCarColor[4];
  bool racCarActive[4];
  // Nitro alternating-tap state
  unsigned long racNitroTapTime;
  int racLastTapSide; // 0=none, 1=left-pressed, 2=right-pressed
  // Flame animation frame
  uint8_t racFlameFrame;

  // ── Game 2: Luna Space ─────────────────────────────────────────────────────
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
  uint8_t spcPartLife[15]; // uint8_t saves 30 bytes vs int
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
  // Parallax star positions (deterministic, computed once)
  uint8_t spcStar1X[10];
  uint8_t spcStar1Y[10];
  uint8_t spcStar2X[6];
  uint8_t spcStar2Y[6];
  bool spcStarsInit;

  // ── Game 3: Flappy Mochy ───────────────────────────────────────────────────
  float flapPlayerY;
  float flapPlayerVY;
  float flapPipeX;
  float flapPipeGapY;
  int flapScore;
  bool flapGameOver;
  float flapCloud1X;
  float flapCloud2X;

  // ── Game 4: Coin Catcher ───────────────────────────────────────────────────
  float catPlayerX;
  float catCoinsX[3];
  float catCoinsY[3];
  bool catCoinsActive[3];
  bool catIsBomb[3];
  int catScore;
  int catLives;
  bool catGameOver;

  // ── Game 5: Mochy Jump ─────────────────────────────────────────────────────
  float jumpPlayerX;
  float jumpPlayerY;
  float jumpPlayerVY;
  float jumpPlatX[5];
  float jumpPlatY[5];
  int jumpScore;
  bool jumpGameOver;
  bool jumpSquash;

  // ── Game 6: Stacker ────────────────────────────────────────────────────────
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
  bool stkFlashNew; // flash newly placed block

  // ── Game 7: Memory Matrix ──────────────────────────────────────────────────
  int memSeq[16];
  int memSeqLen;
  int memSeqStep;
  int memPlayerStep;
  int memSelectedTile;
  int memState; // 0: showing seq, 1: player input
  unsigned long memTimer;
  int memScore;
  bool memGameOver;

  // ── Input edge-detect ──────────────────────────────────────────────────────
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
    hsHighScore = 0;
    hsState = HS_STATE_SETUP;
    hsTimerSeconds = 60;
    hsPhaseStartTime = 0;
    hsBombDurationMs = 0;
    hsHidingDurationMs = 10000;
    hsLastTickMs = 0;
    hsDefusedRemainingMs = 0;
    hsGameOver = false;
    hsLastTouchDown = false;
    hsSoundWaveRadius = 0;
    hsSparkFrame = 0;
    hsExplosionFrame = 0;
    lastBtn1 = false;
    lastBtn2 = false;
    spcStarsInit = false;
  }

  // ── Screen geometry (240×280 portrait) ────────────────────────────────────
#if SCREEN_WIDTH == 240
  static const int G_X = 0;
  static const int G_Y = 58;
  static const int G_W = 240;
  static const int G_H = 222;
  static const int G_B = 280;
  static const int G_R = 240;
#else
  static const int G_X = 0;
  static const int G_Y = 22;
  static const int G_W = 128;
  static const int G_H = 138;
  static const int G_B = 160;
  static const int G_R = 128;
#endif

  // ── Persistent storage ─────────────────────────────────────────────────────
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
    hsHighScore = prefs.getInt("hs", 0);
    prefs.end();
  }

  void saveHighScore(const char *key, int &highScore, int currentScore) {
    if (currentScore > highScore) {
      highScore = currentScore;
      Preferences prefs;
      prefs.begin("luna_scores", false);
      prefs.putInt(key, highScore);
      prefs.end();
    }
  }

  // ── Global Sound Management ────────────────────────────────────────────────
  void toggleSound(LunaAudio &audio) {
    audio.silentMode = !audio.silentMode;
    silentMode = audio.silentMode;
    Preferences prefs;
    prefs.begin("luna", false);
    prefs.putBool("silent", silentMode);
    prefs.end();
    if (audio.silentMode) {
      audio.stopBuzzer();
    } else {
      audio.playSound(SOUND_CHIRP);
    }
  }

  void drawSoundToggleIcon(GFXcanvas16 &display, LunaAudio &audio, int x = 196, int y = 12) {
    bool isMuted = audio.silentMode;
    uint16_t borderCol = isMuted ? 0xF800 : 0x07E0;
    uint16_t bgCol = 0x0842; // Translucent dark glass

    display.fillRoundRect(x, y, 34, 24, 7, bgCol);
    display.drawRoundRect(x, y, 34, 24, 7, borderCol);

    // Speaker body & cone
    display.fillRect(x + 6, y + 8, 4, 8, 0xFFFF);
    display.fillTriangle(x + 9, y + 8, x + 15, y + 4, x + 15, y + 20, 0xFFFF);

    if (!isMuted) {
      // Sound waves (Active sound - vivid green)
      display.drawLine(x + 18, y + 8, x + 20, y + 10, 0x07E0);
      display.drawLine(x + 20, y + 10, x + 20, y + 14, 0x07E0);
      display.drawLine(x + 20, y + 14, x + 18, y + 16, 0x07E0);

      display.drawLine(x + 22, y + 6, x + 25, y + 9, 0x07E0);
      display.drawLine(x + 25, y + 9, x + 25, y + 15, 0x07E0);
      display.drawLine(x + 25, y + 15, x + 22, y + 18, 0x07E0);
    } else {
      // Red Mute slash across speaker
      display.drawLine(x + 17, y + 5, x + 25, y + 18, 0xF800);
      display.drawLine(x + 25, y + 5, x + 17, y + 18, 0xF800);
    }
  }

  // ── IMU tilt steering ──────────────────────────────────────────────────────
  float getTiltSteer() {
    if (!imu.isInitialized())
      return 0.0f;
    float ax = 0, ay = 0, az = 1, gx = 0, gy = 0, gz = 0;
    if (!imu.readMotion(ax, ay, az, gx, gy, gz))
      return 0.0f;
    float lateral = -ay + (-gy * 0.0025f);
    const float dz = 0.06f;
    if (lateral > dz)
      return constrain((lateral - dz) / 0.35f, 0.0f, 1.6f);
    if (lateral < -dz)
      return -constrain((-lateral - dz) / 0.35f, 0.0f, 1.6f);
    return 0.0f;
  }

  // ── Reset functions ────────────────────────────────────────────────────────
  void resetRacer() {
    const int carW = SCREEN_WIDTH == 240 ? 16 : 8;
    const int carH = SCREEN_WIDTH == 240 ? 28 : 14;
    racPlayerX = G_X + G_W / 2 - carW / 2;
    racPlayerY = G_B - carH - 6;
    racSpeed = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
    racNitroActive = false;
    racNitroFuel = 100.0f;
    racRoadScroll = 0;
    racScore = 0;
    racDistAccum = 0.0f;
    racGameOver = false;
    gameOverActive = false;
    racNitroTapTime = 0;
    racLastTapSide = 0;
    racFlameFrame = 0;
    for (int i = 0; i < 4; i++) {
      racCarX[i] = G_X + 24 + random(0, 3) * (G_W - 48) / 3 - carW / 2;
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
    gameOverActive = false;
    spcLastShoot = 0;
    spcShieldTime = 0;
    spcSmartBombCooldown = 0;
    spcSpreadActive = false;
    for (int i = 0; i < 6; i++)
      spcLaserActive[i] = false;
    for (int i = 0; i < 8; i++)
      spcEnemyActive[i] = false;
    for (int i = 0; i < 4; i++)
      spcEnemyLaserActive[i] = false;
    for (int i = 0; i < 15; i++)
      spcPartActive[i] = false;
    spcPowerActive = false;
    spcBossActive = false;
    spcBossHP = 20;
    spcBossX = G_X + G_W / 2;
    spcBossY = G_Y + 15;
    spcBossDir = 1;
    spcBossLastShoot = 0;
    // Init star positions once
    if (!spcStarsInit) {
      for (int i = 0; i < 10; i++) {
        spcStar1X[i] = random(0, G_W);
        spcStar1Y[i] = random(G_Y, G_B);
      }
      for (int i = 0; i < 6; i++) {
        spcStar2X[i] = random(0, G_W);
        spcStar2Y[i] = random(G_Y, G_B);
      }
      spcStarsInit = true;
    }
  }

  void resetFlappy() {
    flapPlayerY = G_Y + G_H / 2;
    flapPlayerVY = 0;
    flapPipeX = G_R;
    int gapH = SCREEN_WIDTH == 240 ? 50 : 35;
    flapPipeGapY = G_Y + 20 + random(0, G_H - gapH - 40);
    flapScore = 0;
    flapGameOver = false;
    gameOverActive = false;
    flapCloud1X = G_W * 0.25f;
    flapCloud2X = G_W * 0.70f;
  }

  void resetCatcher() {
    const int basketW = SCREEN_WIDTH == 240 ? 46 : 22;
    catPlayerX = G_X + G_W / 2 - basketW / 2;
    catScore = 0;
    catLives = 3;
    catGameOver = false;
    gameOverActive = false;
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
    gameOverActive = false;
    jumpSquash = false;
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
    stkFlashNew = false;
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

  void resetHideSeek() {
    hsState = HS_STATE_TITLE;
    if (hsTimerSeconds < 15 || hsTimerSeconds > 600) hsTimerSeconds = 180;
    hsLastCountdownTickMs = millis();
    hsPhaseStartTime = 0;
    hsBombDurationMs = 0;
    hsHidingDurationMs = 10000;
    hsLastTickMs = 0;
    hsDefusedRemainingMs = 0;
    hsGameOver = false;
    gameOverActive = false;
    hsLastTouchDown = false;
    hsSoundWaveRadius = 0;
    hsSparkFrame = 0;
    hsExplosionFrame = 0;
  }

  // ── Game Over eligibility ──────────────────────────────────────────────────
  bool canExitActiveGame() {
    bool go = false;
    if (gameSelected == 1)
      go = racGameOver;
    else if (gameSelected == 2)
      go = spcGameOver;
    else if (gameSelected == 3)
      go = flapGameOver;
    else if (gameSelected == 4)
      go = catGameOver;
    else if (gameSelected == 5)
      go = jumpGameOver;
    else if (gameSelected == 6)
      go = hsGameOver || (hsState == HS_STATE_SETUP);
    else if (gameSelected == 7 || gameSelected == 8 || gameSelected == 9)
      go = true;
    gameOverActive = go;
    return go;
  }

  // ── Shared: draw a pixel-art heart at (x, y) size s ──────────────────────
  void drawHeart(GFXcanvas16 &display, int x, int y, int s, uint16_t col,
                 bool filled) {
    if (filled) {
      display.fillCircle(x, y, s, col);
      display.fillCircle(x + s * 2, y, s, col);
      display.fillTriangle(x - s, y, x + s * 3, y, x + s, y + s * 2, col);
    } else {
      display.drawCircle(x, y, s, col);
      display.drawCircle(x + s * 2, y, s, col);
      display.drawTriangle(x - s, y, x + s * 3, y, x + s, y + s * 2, col);
    }
  }

  // ── Shared: Game Over screen ───────────────────────────────────────────────
  void drawGameOverScreen(GFXcanvas16 &display, int score, int highScore) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8;
    const bool newRecord = (score > 0 && score >= highScore);

    // Dark card
    display.fillRect(20, 60, SCREEN_WIDTH - 40, 170, 0x0841);
    display.drawRect(20, 60, SCREEN_WIDTH - 40, 170,
                     newRecord ? 0xFD20 : 0xF800);
    display.drawRect(21, 61, SCREEN_WIDTH - 42, 168,
                     newRecord ? 0xFD20 : 0xF800);

    // GAME OVER title
    display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
    display.setTextColor(0xF800);
    int goW = 9 * (SCREEN_WIDTH == 240 ? 18 : 12);
    display.setCursor((SCREEN_WIDTH - goW) / 2, SCREEN_WIDTH == 240 ? 78 : 70);
    display.print("GAME OVER");

    // Score pill
    display.fillRoundRect(40, SCREEN_WIDTH == 240 ? 120 : 95, SCREEN_WIDTH - 80,
                          SCREEN_WIDTH == 240 ? 24 : 18, 6, 0x18C3);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    char scoreBuf[24];
    snprintf(scoreBuf, sizeof(scoreBuf), "SC:%d", score);
    int sw = strlen(scoreBuf) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - sw) / 2, SCREEN_WIDTH == 240 ? 126 : 99);
    display.print(scoreBuf);

    // New record badge
    if (newRecord) {
      display.setTextColor(0xFD20);
      display.setCursor((SCREEN_WIDTH - 10 * (SCREEN_WIDTH == 240 ? 12 : 6)) /
                            2,
                        SCREEN_WIDTH == 240 ? 153 : 120);
      display.print("NEW RECORD!");
    } else {
      display.setTextColor(0x8410);
      char hiBuf[20];
      snprintf(hiBuf, sizeof(hiBuf), "BEST:%d", highScore);
      int hw = strlen(hiBuf) * (SCREEN_WIDTH == 240 ? 12 : 6);
      display.setCursor((SCREEN_WIDTH - hw) / 2,
                        SCREEN_WIDTH == 240 ? 153 : 120);
      display.print(hiBuf);
    }

    // Replay / exit prompts
    display.setTextColor(themeAccent);
    const char *pAgain = "TAP TO REPLAY";
    int pW = strlen(pAgain) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - pW) / 2, SCREEN_WIDTH == 240 ? 182 : 142);
    display.print(pAgain);

    display.setTextColor(0x8410);
    const char *bMenu = "SWIPE TO EXIT";
    int bW = strlen(bMenu) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - bW) / 2, SCREEN_WIDTH == 240 ? 200 : 156);
    display.print(bMenu);
  }

  // ============================================================
  //  ARCADE MENU
  // ============================================================
  void drawMenu(GFXcanvas16 &display) {
    const uint16_t themeAccent = 0x001F; // Royal blue
    const uint16_t themeBg = 0xFFFF;
    const uint16_t themeText = 0x1082;
    const uint16_t themeBorder = 0xCE79;
    const uint16_t themeSubText = 0x632C;

    display.fillRect(0, 22, SCREEN_WIDTH, SCREEN_HEIGHT - 22, themeBg);

    // Header
    display.setTextSize(2);
    display.setTextColor(themeText);
    display.setCursor(20, 28);
    display.print("LUNA ARCADE");
    // Animated accent dots
    unsigned long t = millis();
    uint8_t pulse = (t / 500) % 2;
    display.fillCircle(168, 35, pulse ? 4 : 3, themeAccent);
    display.fillCircle(180, 35, pulse ? 3 : 4, 0x07FF);
    display.drawFastHLine(20, 46, SCREEN_WIDTH - 40, themeBorder);

    const int opt = constrain(gameMenuOption, 0, 5);

    const char *titlesL1[] = {"LUNA", "LUNA",  "FLAPPY",
                              "COIN", "MOCHY", "EXIT"};
    const char *titlesL2[] = {"RACER",   "SPACE", "MOCHY",
                              "CATCHER", "JUMP",  "ARCADE"};
    int highScores[] = {racHighScore, spcHighScore,  flapHighScore,
                        catHighScore, jumpHighScore, 0};

    // Game title (large 2-line)
    display.setTextSize(3);
    display.setTextColor(themeText);
    display.setCursor(20, 56);
    display.print(titlesL1[opt]);
    display.setTextColor(themeAccent);
    display.setCursor(20, 84);
    display.print(titlesL2[opt]);

    // Tagline
    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(20, 114);
    if (opt < 5) {
      char buf[48];
      if (opt == 0 || opt == 1 || opt == 3 || opt == 4)
        snprintf(buf, sizeof(buf), "HI:%d // GYRO TILT ACTIVE",
                 highScores[opt]);
      else
        snprintf(buf, sizeof(buf), "SIMULATION // HI-SCORE: %d",
                 highScores[opt]);
      display.print(buf);
    } else {
      display.print("RETURN // WATCH INTERFACE");
    }

    // ── Animated Preview Silhouette (Y: 128..185) ──────────────────────────
    const int cx = 120;
    const int cy = 154;
    // Subtle oscillation using millis
    const int wave = (int)(sinf(t / 500.0f) * 3);

    switch (opt) {
    case 0: { // LUNA RACER
      // Road markings
      display.fillRect(cx - 30, cy + wave, 6, 20, 0xCE79);
      display.fillRect(cx + 24, cy + wave, 6, 20, 0xCE79);
      // Neon car body
      display.fillRoundRect(cx - 14, cy - 18 + wave, 28, 36, 3, themeAccent);
      display.fillRoundRect(cx - 9, cy - 10 + wave, 18, 14, 2,
                            0xFFFF); // windshield
      // Wheels
      display.fillCircle(cx - 10, cy + 20 + wave, 4, themeText);
      display.fillCircle(cx + 10, cy + 20 + wave, 4, themeText);
      display.fillCircle(cx - 10, cy - 14 + wave, 4, themeText);
      display.fillCircle(cx + 10, cy - 14 + wave, 4, themeText);
      // Nitro flame (flicker)
      display.fillTriangle(cx - 6, cy + 20 + wave, cx, cy + 28 + wave, cx + 6,
                           cy + 20 + wave, 0xFB80);
    } break;

    case 1: { // LUNA SPACE
      display.drawCircle(cx, cy + wave, 32, themeBorder);
      display.drawFastHLine(cx - 38, cy + wave, 76, 0xCE79);
      display.drawFastVLine(cx, cy - 38 + wave, 76, 0xCE79);
      display.fillTriangle(cx, cy - 22 + wave, cx - 20, cy + 16 + wave, cx + 20,
                           cy + 16 + wave, themeAccent);
      display.fillTriangle(cx, cy - 14 + wave, cx - 12, cy + 10 + wave, cx + 12,
                           cy + 10 + wave, 0xF7BE);
      // Engine glow
      display.fillCircle(cx, cy + 18 + wave, 3, 0x07E0);
      display.fillCircle(cx - 14, cy + 14 + wave, 2, 0xFD20);
      display.fillCircle(cx + 14, cy + 14 + wave, 2, 0xFD20);
    } break;

    case 2: { // FLAPPY MOCHY
      // Pipes
      display.drawFastVLine(cx - 40, cy - 24 + wave, 20, themeBorder);
      display.drawFastVLine(cx - 40, cy + 10 + wave, 22, themeBorder);
      display.drawFastVLine(cx + 40, cy - 18 + wave, 14, themeBorder);
      display.drawFastVLine(cx + 40, cy + 12 + wave, 18, themeBorder);
      // Mochy character
      display.fillCircle(cx, cy + wave, 12, themeAccent);
      display.drawCircle(cx, cy + wave, 14, themeText);
      display.fillCircle(cx + 4, cy - 3 + wave, 4, TFT_WHITE);
      display.fillCircle(cx + 5, cy - 2 + wave, 2, TFT_BLACK);
      display.fillTriangle(cx + 12, cy + wave, cx + 12, cy + 4 + wave, cx + 20,
                           cy + 2 + wave, 0xFD20);
      // Wing
      display.drawLine(cx - 8, cy + wave, cx - 18, cy - 10 + wave, themeText);
      display.drawLine(cx - 18, cy - 10 + wave, cx - 4, cy - 4 + wave,
                           themeText);
    } break;

    case 3: { // COIN CATCHER
      // Spinning coin illusion
      int coinW = (int)(cosf(t / 300.0f) * 12);
      if (abs(coinW) < 2)
        coinW = 2;
      display.fillRoundRect(cx - abs(coinW), cy - 12 + wave, abs(coinW) * 2, 24,
                            4, 0xFD20);
      display.drawCircle(cx, cy + wave, 26, themeBorder);
      // Basket
      display.fillRoundRect(cx - 24, cy + 28 + wave, 48, 8, 3, 0xC3A5);
      display.drawLine(cx - 24, cy + 28 + wave, cx - 28, cy + 22 + wave,
                       0x8410);
      display.drawLine(cx + 24, cy + 28 + wave, cx + 28, cy + 22 + wave,
                       0x8410);
    } break;

    case 4: { // MOCHY JUMP
      display.fillRoundRect(cx - 36, cy + 18 + wave, 30, 4, 2, themeAccent);
      display.fillRoundRect(cx - 6, cy + 2 + wave, 32, 4, 2, themeAccent);
      display.fillRoundRect(cx + 12, cy - 16 + wave, 28, 4, 2, themeAccent);
      // Character with squash
      display.fillRoundRect(cx + 4, cy - 28 + wave, 12, 14, 2, TFT_YELLOW);
      display.fillCircle(cx + 7, cy - 24 + wave, 1, TFT_BLACK);
      display.fillCircle(cx + 13, cy - 24 + wave, 1, TFT_BLACK);
      // Jump trail
      display.drawLine(cx + 10, cy - 14 + wave, cx + 10, cy - 10 + wave,
                       0x07E0);
    } break;

    case 5: { // EXIT
      display.drawCircle(cx, cy + wave, 24, 0xF800);
      display.drawCircle(cx, cy + wave, 22, 0xF800);
      display.drawLine(cx - 8, cy + wave, cx + 8, cy + wave, 0xF800);
      display.drawLine(cx - 8, cy + wave, cx - 2, cy - 6 + wave, 0xF800);
      display.drawLine(cx - 8, cy + wave, cx - 2, cy + 6 + wave, 0xF800);
    } break;
    }

    // ── Play Button ────────────────────────────────────────────────────────
    const int btnW = 140, btnH = 34;
    const int btnX = (SCREEN_WIDTH - btnW) / 2, btnY = 194;
    const uint16_t btnColor = (opt == 5) ? 0xF800 : themeAccent;
    // Subtle shadow
    display.fillRoundRect(btnX + 2, btnY + 2, btnW, btnH, 8, 0x8410);
    display.fillRoundRect(btnX, btnY, btnW, btnH, 8, btnColor);
    // Inner highlight strip
    display.drawFastHLine(btnX + 8, btnY + 4, btnW - 16,
                          blend565(btnColor, TFT_WHITE));

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    const char *btnTxt = (opt == 5) ? "EXIT <" : "PLAY >";
    int bW = strlen(btnTxt) * 12;
    display.setCursor(btnX + (btnW - bW) / 2, btnY + 9);
    display.print(btnTxt);

    // ── Carousel indicator ─────────────────────────────────────────────────
    const int dotStartX = 85;
    for (int d = 0; d < 6; d++) {
      int dx = dotStartX + d * 14;
      if (d == opt) {
        display.fillRoundRect(dx - 3, 238, 10, 4, 2, btnColor);
      } else {
        display.fillCircle(dx, 240, 2, themeBorder);
      }
    }

    display.setTextSize(1);
    display.setTextColor(themeSubText);
    display.setCursor(26, 256);
    display.print("SWIPE: BROWSE  //  TAP: LAUNCH");
  }

  // ============================================================
  //  GAME 1: LUNA RACER
  // ============================================================
  void updateAndDrawRacer(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    const uint16_t themeBg = 0x0841; // Dark HUD bar
    const uint16_t themeText = TFT_WHITE;
    const uint16_t themeBorder = 0x4208;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;
    const bool justBtn1 = btn1 && !lastBtn1;
    const bool justBtn2 = btn2 && !lastBtn2;

    const int carW = SCREEN_WIDTH == 240 ? 18 : 9;
    const int carH = SCREEN_WIDTH == 240 ? 30 : 15;

    if (!racGameOver) {
      // ── Nitro: Physical power button (GPIO 40) or alternating rapid tap ──
      bool pwrNitroPressed = (digitalRead(40) == LOW);
      if (pwrNitroPressed) {
        if (racNitroFuel > 5.0f)
          racNitroActive = true;
      } else if (justBtn1) {
        if (racLastTapSide == 2 && (millis() - racNitroTapTime) < 450) {
          if (racNitroFuel > 5.0f)
            racNitroActive = true;
        }
        racLastTapSide = 1;
        racNitroTapTime = millis();
      } else if (justBtn2) {
        if (racLastTapSide == 1 && (millis() - racNitroTapTime) < 450) {
          if (racNitroFuel > 5.0f)
            racNitroActive = true;
        }
        racLastTapSide = 2;
        racNitroTapTime = millis();
      }

      // Nitro burn / extinguish
      if (racNitroActive) {
        racSpeed = SCREEN_WIDTH == 240 ? 8.5f : 5.5f;
        racNitroFuel -= 0.7f;
        racFlameFrame = (racFlameFrame + 1) % 6;
        if (millis() % 400 < 200)
          audio.playSound(SOUND_JUMP);
        if (racNitroFuel <= 0) {
          racNitroFuel = 0;
          racNitroActive = false;
        }
      } else {
        // Passive recharge
        racNitroFuel = min(100.0f, racNitroFuel + 0.10f);
        racSpeed = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
        racFlameFrame = 0;
      }

      // ── Steering ──────────────────────────────────────────────────────────
      float steerDelta = 0.0f;
      if (btn1 && !btn2)
        steerDelta -= 3.5f;
      else if (btn2 && !btn1)
        steerDelta += 3.5f;
      float tilt = getTiltSteer();
      if (fabsf(tilt) > 0.01f)
        steerDelta += tilt * 5.2f;
      racPlayerX = constrain(racPlayerX + steerDelta, (float)(G_X + 18),
                             (float)(G_R - 18 - carW));

      // ── Road physics & distance ───────────────────────────────────────────
      racRoadScroll += racSpeed;
      if (racRoadScroll >= 40)
        racRoadScroll = 0;
      racDistAccum += racSpeed * 0.25f;
      racScore = (int)racDistAccum;

      // ── Traffic cars ──────────────────────────────────────────────────────
      for (int i = 0; i < 4; i++) {
        if (racCarActive[i]) {
          racCarY[i] += (racSpeed - racCarSpeed[i]);
          if (racCarY[i] > G_B + 10) {
            racCarY[i] = G_Y - 30;
            racCarX[i] = G_X + 24 + random(0, 3) * (G_W - 48) / 3 - carW / 2;
            racCarSpeed[i] = 1.0f + random(0, 20) / 10.0f;
            racCarActive[i] = true;
          }
          // Collision
          if (abs((int)racPlayerX - (int)racCarX[i]) <
                  (SCREEN_WIDTH == 240 ? 14 : 7) &&
              abs((int)racPlayerY - (int)racCarY[i]) <
                  (SCREEN_WIDTH == 240 ? 26 : 13)) {
            racGameOver = true;
            gameOverActive = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("rac", racHighScore, racScore);
          }
        }
      }

    } else {
      if (justBtn1)
        resetRacer();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Draw road ─────────────────────────────────────────────────────────
    // Grass
    display.fillScreen(0x2285);
    // Darker outer shoulder
    display.fillRect(G_X, G_Y, 18, G_H, 0x0C43);
    display.fillRect(G_R - 18, G_Y, 18, G_H, 0x0C43);
    // Asphalt
    display.fillRect(G_X + 18, G_Y, G_W - 36, G_H, 0x2104);

    // Road edge white lines
    display.drawFastVLine(G_X + 18, G_Y, G_H, TFT_WHITE);
    display.drawFastVLine(G_R - 18, G_Y, G_H, TFT_WHITE);

    // Animated center lane dashes (neon cyan)
    for (int y = G_Y - 20; y < G_B; y += 40) {
      int sy = (int)(y + racRoadScroll);
      if (sy >= G_Y && sy < G_B - 20)
        display.fillRect(G_X + G_W / 2 - 1, sy, 2,
                         SCREEN_WIDTH == 240 ? 20 : 10, 0x07FF);
    }

    // Grass detail strips
    for (int y = G_Y + 10; y < G_B; y += 25) {
      display.fillRect(G_X + 4, y, 6, 3, 0x03E0);
      display.fillRect(G_R - 10, y, 6, 3, 0x03E0);
    }

    // ── Draw traffic cars (with proper front detail) ──────────────────────
    const uint16_t carBodyColors[] = {0xF800, 0x001F,
                                      0x03E0}; // red, blue, green
    const uint16_t carRoofColors[] = {0xC000, 0x000C, 0x0280};
    for (int i = 0; i < 4; i++) {
      if (racCarActive[i] && racCarY[i] >= G_Y && racCarY[i] < G_B) {
        int cx = (int)racCarX[i], cy = (int)racCarY[i];
        uint16_t bodyCol = carBodyColors[racCarColor[i]];
        uint16_t roofCol = carRoofColors[racCarColor[i]];
        // Body
        display.fillRoundRect(cx, cy, carW, carH, 2, bodyCol);
        // Roof
        display.fillRect(cx + carW / 4, cy + carH / 5, carW / 2, carH * 2 / 5,
                         roofCol);
        // Windshield
        display.fillRect(cx + carW / 4 + 1, cy + carH / 5 + 1, carW / 2 - 2,
                         carH / 5, 0x07FF);
        // Headlights
        display.fillRect(cx + 1, cy, carW / 4, carH / 8, 0xFFE0);
        display.fillRect(cx + carW * 3 / 4 - 1, cy, carW / 4, carH / 8, 0xFFE0);
        // Tail lights
        display.fillRect(cx + 1, cy + carH - carH / 8, carW / 4, carH / 8,
                         0xF800);
        display.fillRect(cx + carW * 3 / 4 - 1, cy + carH - carH / 8, carW / 4,
                         carH / 8, 0xF800);
      }
    }

    // ── Draw player car ──────────────────────────────────────────────────
    {
      int px = (int)racPlayerX, py = (int)racPlayerY;
      // Body
      display.fillRoundRect(px, py, carW, carH, 2, 0xFB40);
      // Roof / cabin
      display.fillRect(px + carW / 4, py + carH / 5, carW / 2, carH * 2 / 5,
                       0xC840);
      // Windshield (white)
      display.fillRect(px + carW / 4 + 1, py + carH / 5 + 1, carW / 2 - 2,
                       carH / 5, TFT_WHITE);
      // Front headlights (white-yellow)
      display.fillRect(px + 1, py, carW / 4, carH / 8, 0xFFFF);
      display.fillRect(px + carW * 3 / 4 - 1, py, carW / 4, carH / 8, 0xFFFF);
      // Tail lights
      display.fillRect(px + 1, py + carH - carH / 8, carW / 4, carH / 8,
                       0xF800);
      display.fillRect(px + carW * 3 / 4 - 1, py + carH - carH / 8, carW / 4,
                       carH / 8, 0xF800);
      // Wheels
      display.fillCircle(px + carW / 5, py + carH - 2, carW / 5, 0x18C3);
      display.fillCircle(px + carW * 4 / 5, py + carH - 2, carW / 5, 0x18C3);
      display.fillCircle(px + carW / 5, py + 4, carW / 5, 0x18C3);
      display.fillCircle(px + carW * 4 / 5, py + 4, carW / 5, 0x18C3);
    }

    // ── Nitro flame FX ───────────────────────────────────────────────────
    if (racNitroActive) {
      int px = (int)racPlayerX, py = (int)racPlayerY;
      const uint16_t flameColors[] = {0xFB40, 0xFD20, 0xFFE0,
                                      0xFB40, 0xF800, 0xFD60};
      uint16_t fc = flameColors[racFlameFrame];
      int flameH = 8 + (racFlameFrame % 3) * 4;
      display.fillTriangle(px + carW / 4, py + carH, px + carW / 2,
                           py + carH + flameH, px + 3 * carW / 4, py + carH,
                           fc);
      display.fillTriangle(px + carW / 2 - 3, py + carH, px + carW / 2,
                           py + carH + flameH + 4, px + carW / 2 + 3, py + carH,
                           TFT_WHITE);
    }

    // ── HUD ──────────────────────────────────────────────────────────────
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);

    // Score
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("DIST:%04d", racScore);

    // Nitro label + bar
    display.setCursor(SCREEN_WIDTH / 2 + 6, SCREEN_WIDTH == 240 ? 30 : 5);
    display.setTextColor(racNitroActive ? 0xFD20 : 0x8410);
    display.print("NIT");
    int barX = SCREEN_WIDTH - 42, barY = SCREEN_WIDTH == 240 ? 32 : 8;
    int barW = 36, barH = SCREEN_WIDTH == 240 ? 12 : 6;
    display.drawRoundRect(barX, barY, barW, barH, 2, 0x4A49);
    int fillW = (int)(racNitroFuel * (barW - 2) / 100.0f);
    uint16_t fuelCol =
        racNitroActive ? 0xFD20 : (racNitroFuel > 50 ? 0x07E0 : 0xF800);
    display.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 1, fuelCol);

    // Tap hint
    display.setTextSize(1);
    display.setTextColor(0x4A49);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 52 : 18);
    display.print("PWR BTN = NITRO");

    if (racGameOver)
      drawGameOverScreen(display, racScore, racHighScore);
  }

  // ============================================================
  //  GAME 2: LUNA SPACE
  // ============================================================
  void updateAndDrawSpace(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;
    const bool justBtn1 = btn1 && !lastBtn1;
    const bool justBtn2 = btn2 && !lastBtn2;

    const int pw = SCREEN_WIDTH == 240 ? 12 : 6;
    const int ph = SCREEN_WIDTH == 240 ? 16 : 8;
    const int ew = SCREEN_WIDTH == 240 ? 12 : 6;
    const int eh = SCREEN_WIDTH == 240 ? 8 : 4;
    const int playerColW = SCREEN_WIDTH == 240 ? 16 : 10;
    const int playerColH = SCREEN_WIDTH == 240 ? 12 : 6;
    const int enemyColW = SCREEN_WIDTH == 240 ? 18 : 10;
    const int enemyColH = SCREEN_WIDTH == 240 ? 14 : 8;

    if (!spcGameOver) {
      // Smart bomb (btn1+btn2 within 200ms of each other — use edge detect
      // trick)
      if (justBtn1 && btn2 && (millis() - spcSmartBombCooldown > 8000)) {
        spcSmartBombCooldown = millis();
        audio.playSound(SOUND_GAMEOVER);
        for (int i = 0; i < 4; i++)
          spcEnemyLaserActive[i] = false;
        for (int i = 0; i < 8; i++) {
          if (spcEnemyActive[i]) {
            spcEnemyHP[i]--;
            if (spcEnemyHP[i] <= 0) {
              spcEnemyActive[i] = false;
              spcScore += 20;
            }
          }
        }
        if (spcBossActive)
          spcBossHP -= 3;
        for (int p = 0; p < 15; p++) {
          spcPartActive[p] = true;
          spcPartX[p] = G_X + G_W / 2;
          spcPartY[p] = G_Y + G_H / 2;
          spcPartVX[p] = random(-40, 41) / 10.0f;
          spcPartVY[p] = random(-40, 41) / 10.0f;
          spcPartLife[p] = 14;
        }
      }

      // Steering
      float steerDelta = 0.0f;
      if (btn1 && !btn2)
        steerDelta -= 3.5f;
      else if (btn2 && !btn1)
        steerDelta += 3.5f;
      float tilt = getTiltSteer();
      if (fabsf(tilt) > 0.01f)
        steerDelta += tilt * 5.5f;
      spcPlayerX = constrain(spcPlayerX + steerDelta, (float)(G_X + pw + 4),
                             (float)(G_R - pw - 4));

      // Auto-fire
      if (millis() - spcLastShoot > 320) {
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

      // Move player lasers
      for (int i = 0; i < 6; i++) {
        if (spcLaserActive[i]) {
          spcLaserY[i] -= 5.0f;
          if (spcLaserY[i] < G_Y)
            spcLaserActive[i] = false;
        }
      }

      // Move enemy lasers + hit player
      for (int i = 0; i < 4; i++) {
        if (spcEnemyLaserActive[i]) {
          spcEnemyLaserY[i] += 2.5f;
          if (spcEnemyLaserY[i] > G_B) {
            spcEnemyLaserActive[i] = false;
            continue;
          }
          if (abs((int)spcEnemyLaserX[i] - (int)spcPlayerX) < playerColW &&
              spcEnemyLaserY[i] >= spcPlayerY - playerColH &&
              spcEnemyLaserY[i] <= spcPlayerY + playerColH / 2) {
            spcEnemyLaserActive[i] = false;
            if (millis() > spcShieldTime) {
              audio.playSound(SOUND_POWERDOWN);
              if (--spcHealth <= 0) {
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

      // Update enemies
      bool anyEnemy = false;
      for (int i = 0; i < 8; i++) {
        if (spcEnemyActive[i]) {
          anyEnemy = true;
          spcEnemyY[i] += 0.5f;
          spcEnemyX[i] += sinf(millis() / 200.0f + i) * 0.8f;
          if (spcEnemyY[i] > G_B + 10) {
            spcEnemyActive[i] = false;
            if (--spcHealth <= 0) {
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
          // Check player laser hits
          for (int l = 0; l < 6; l++) {
            if (spcLaserActive[l] &&
                abs((int)spcLaserX[l] - (int)spcEnemyX[i]) < enemyColW &&
                abs((int)spcLaserY[l] - (int)spcEnemyY[i]) < enemyColH) {
              spcLaserActive[l] = false;
              audio.playSound(SOUND_JUMP);
              if (--spcEnemyHP[i] <= 0) {
                spcEnemyActive[i] = false;
                spcScore += 20;
                if (random(0, 10) < 3 && !spcPowerActive) {
                  spcPowerActive = true;
                  spcPowerX = spcEnemyX[i];
                  spcPowerY = spcEnemyY[i];
                  spcPowerType = random(0, 2);
                }
                for (int p = 0; p < 4; p++) {
                  int pIdx = random(0, 15);
                  spcPartActive[pIdx] = true;
                  spcPartX[pIdx] = spcEnemyX[i];
                  spcPartY[pIdx] = spcEnemyY[i];
                  spcPartVX[pIdx] = random(-25, 26) / 10.0f;
                  spcPartVY[pIdx] = random(-25, 26) / 10.0f;
                  spcPartLife[pIdx] = 8;
                }
              }
            }
          }
        }
      }

      // Spawn wave
      if (!anyEnemy && !spcBossActive) {
        if (spcScore >= 180) {
          spcBossActive = true;
          spcBossHP = 20;
        } else {
          for (int i = 0; i < 4; i++) {
            spcEnemyActive[i] = true;
            spcEnemyX[i] = G_X + 24 + i * (G_W - 48) / 4;
            spcEnemyY[i] = G_Y - 20;
            spcEnemyHP[i] = 1;
            spcEnemyType[i] = i % 2;
            spcEnemyLastShoot[i] = millis();
          }
        }
      }

      // Power-up movement
      if (spcPowerActive) {
        spcPowerY += 1.2f;
        if (spcPowerY > G_B)
          spcPowerActive = false;
        if (abs((int)spcPowerX - (int)spcPlayerX) < 16 &&
            abs((int)spcPowerY - (int)spcPlayerY) < 16) {
          spcPowerActive = false;
          audio.playSound(SOUND_POWERUP);
          if (spcPowerType == 0)
            spcSpreadActive = true;
          else
            spcShieldTime = millis() + 5000;
        }
      }

      // Particles
      for (int i = 0; i < 15; i++) {
        if (spcPartActive[i]) {
          spcPartX[i] += spcPartVX[i];
          spcPartY[i] += spcPartVY[i];
          if (--spcPartLife[i] <= 0)
            spcPartActive[i] = false;
        }
      }

      // Boss update
      if (spcBossActive) {
        spcBossX += spcBossDir * 0.8f;
        if (spcBossX < G_X + 32)
          spcBossDir = 1;
        if (spcBossX > G_R - 32)
          spcBossDir = -1;
        if (millis() - spcBossLastShoot > 1600) {
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
        const int bw = SCREEN_WIDTH == 240 ? 32 : 16;
        const int bh = SCREEN_WIDTH == 240 ? 16 : 8;
        for (int l = 0; l < 6; l++) {
          if (spcLaserActive[l] &&
              abs((int)spcLaserX[l] - (int)spcBossX) < bw + 4 &&
              abs((int)spcLaserY[l] - (int)spcBossY) < bh + 4) {
            spcLaserActive[l] = false;
            audio.playSound(SOUND_JUMP);
            if (--spcBossHP <= 0) {
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
    } else {
      if (justBtn1)
        resetSpace();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Draw background: deep space ───────────────────────────────────────
    display.fillScreen(0x0002);

    // Parallax stars layer 1 (slow)
    unsigned long t = millis();
    for (int i = 0; i < 10; i++) {
      int sy = G_Y + (spcStar1Y[i] + t / 40) % G_H;
      display.drawPixel(spcStar1X[i], sy, 0x7BEF);
    }
    // Parallax stars layer 2 (fast)
    for (int i = 0; i < 6; i++) {
      int sy = G_Y + (spcStar2Y[i] + t / 15) % G_H;
      display.drawPixel(spcStar2X[i], sy, TFT_WHITE);
    }

    // Lasers (player: cyan, enemy: red-orange)
    for (int i = 0; i < 6; i++) {
      if (spcLaserActive[i]) {
        display.fillRect((int)spcLaserX[i], (int)spcLaserY[i], 2,
                         SCREEN_WIDTH == 240 ? 8 : 4, 0x07FF);
        display.drawPixel((int)spcLaserX[i], (int)spcLaserY[i], TFT_WHITE);
      }
    }
    for (int i = 0; i < 4; i++) {
      if (spcEnemyLaserActive[i]) {
        display.fillRect((int)spcEnemyLaserX[i], (int)spcEnemyLaserY[i], 2,
                         SCREEN_WIDTH == 240 ? 8 : 4, 0xF800);
        display.drawPixel((int)spcEnemyLaserX[i], (int)spcEnemyLaserY[i],
                          0xFD20);
      }
    }

    // Particles (orange/yellow sparks)
    const uint16_t partColors[] = {0xFD20, 0xFB40, 0xF800, 0xFFE0};
    for (int i = 0; i < 15; i++) {
      if (spcPartActive[i]) {
        uint16_t pc = partColors[spcPartLife[i] % 4];
        display.drawPixel((int)spcPartX[i], (int)spcPartY[i], pc);
        if (spcPartLife[i] > 4)
          display.drawPixel((int)spcPartX[i] + 1, (int)spcPartY[i], pc);
      }
    }

    // Power-up
    if (spcPowerActive) {
      const int pwSize = SCREEN_WIDTH == 240 ? 12 : 8;
      uint16_t pwCol = spcPowerType == 0 ? 0xFFE0 : 0x07E0;
      display.fillRoundRect((int)spcPowerX - pwSize / 2,
                            (int)spcPowerY - pwSize / 2, pwSize, pwSize, 3,
                            pwCol);
      display.setTextColor(TFT_BLACK);
      display.setTextSize(1);
      display.setCursor((int)spcPowerX - 3, (int)spcPowerY - 3);
      display.print(spcPowerType == 0 ? "W" : "S");
    }

    // Enemies — type 0: diamond, type 1: X-fighter
    for (int i = 0; i < 8; i++) {
      if (spcEnemyActive[i]) {
        int ex = (int)spcEnemyX[i], ey = (int)spcEnemyY[i];
        if (spcEnemyType[i] == 0) {
          // Diamond
          display.fillTriangle(ex, ey - eh, ex - ew, ey, ex + ew, ey, 0xF81F);
          display.fillTriangle(ex, ey + eh, ex - ew, ey, ex + ew, ey, 0xF81F);
          display.fillCircle(ex, ey, ew / 3, TFT_WHITE);
        } else {
          // X-fighter
          display.fillRect(ex - ew, ey - 2, ew * 2, 4, 0xFC00);
          display.fillRect(ex - 2, ey - eh, 4, eh * 2, 0xFC00);
          display.fillCircle(ex, ey, 3, 0xFFFF);
        }
      }
    }

    // Boss
    if (spcBossActive) {
      const int bw = SCREEN_WIDTH == 240 ? 32 : 16;
      const int bh = SCREEN_WIDTH == 240 ? 16 : 8;
      int bx = (int)spcBossX, by = (int)spcBossY;
      // Main body
      display.fillRoundRect(bx - bw, by - bh, bw * 2, bh * 2, 4, 0xA000);
      // Wings
      display.fillTriangle(bx - bw, by, bx - bw - 12, by - bh, bx - bw, by - bh,
                           0xF800);
      display.fillTriangle(bx + bw, by, bx + bw + 12, by - bh, bx + bw, by - bh,
                           0xF800);
      // Core
      display.fillCircle(bx, by, bh / 2, 0xFFE0);
      // Engine cannons
      display.fillRect(bx - bw / 3 - 2, by + bh, 4, bh / 2, 0xFD20);
      display.fillRect(bx + bw / 3 - 2, by + bh, 4, bh / 2, 0xFD20);
      // HP bar
      int hpBarW = bw * 2 - 4;
      display.drawFastHLine(bx - bw + 2, by - bh - 6, hpBarW, 0x4208);
      display.drawFastHLine(bx - bw + 2, by - bh - 6, spcBossHP * hpBarW / 20,
                            0x07E0);
    }

    // Player ship — body + wings + canopy + engine glow
    {
      int px = (int)spcPlayerX, py = (int)spcPlayerY;
      // Main thrust engine glow
      display.fillCircle(px, py + ph / 2, 3, 0x07E0);
      // Body
      display.fillTriangle(px, py - ph, px - pw, py + ph / 2, px + pw,
                           py + ph / 2, themeAccent);
      // Wings
      display.fillTriangle(px - pw, py, px - pw - 6, py + ph / 2, px - pw / 2,
                           py + ph / 2, 0x4208);
      display.fillTriangle(px + pw, py, px + pw + 6, py + ph / 2, px + pw / 2,
                           py + ph / 2, 0x4208);
      // Canopy
      display.fillRect(px - 2, py - ph / 2, 4, ph / 3, 0x07FF);
      // Side mini-engines
      display.fillCircle(px - pw - 2, py + ph / 2, 2, 0xFD20);
      display.fillCircle(px + pw + 2, py + ph / 2, 2, 0xFD20);
      // Shield ring
      if (millis() < spcShieldTime)
        display.drawCircle(px, py - ph / 4, SCREEN_WIDTH == 240 ? 20 : 10,
                           0x07E0);
    }

    // ── HUD ──────────────────────────────────────────────────────────────
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", spcScore);

    // Hearts for health
    for (int i = 0; i < 3; i++) {
      int hx = SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 54 : 32) +
               i * (SCREEN_WIDTH == 240 ? 18 : 10);
      int hy = SCREEN_WIDTH == 240 ? 38 : 9;
      drawHeart(display, hx, hy, 3, i < spcHealth ? 0xF800 : 0x4208,
                i < spcHealth);
    }

    if (spcGameOver) {
      if (spcHealth == 99) {
        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0841);
        display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
        display.setTextColor(0x07E0);
        display.setCursor(
            (SCREEN_WIDTH - 8 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, G_Y + 20);
        display.print("VICTORY!");
        display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
        display.setTextColor(TFT_WHITE);
        display.setCursor(20, G_Y + 60);
        display.print("SPACE BOSS DESTROYED!");
        display.setTextColor(themeAccent);
        display.setCursor(20, G_Y + 90);
        display.print("TAP TO PLAY AGAIN");
      } else {
        drawGameOverScreen(display, spcScore, spcHighScore);
      }
    }
  }

  // ============================================================
  //  GAME 3: FLAPPY MOCHY
  // ============================================================
  void updateAndDrawFlappy(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    const uint16_t themeBorder = 0xCE79;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int mSize = SCREEN_WIDTH == 240 ? 9 : 5;
    const int pW = SCREEN_WIDTH == 240 ? 24 : 16;
    const int gapH = SCREEN_WIDTH == 240 ? 52 : 36;

    if (!flapGameOver) {
      if (btn2 && !lastBtn2) {
        flapPlayerVY = SCREEN_WIDTH == 240 ? -4.0f : -3.0f;
        audio.playSound(SOUND_JUMP);
      }
      flapPlayerVY += SCREEN_WIDTH == 240 ? 0.22f : 0.18f;
      flapPlayerY += flapPlayerVY;

      // Animate clouds
      flapCloud1X -= 0.4f;
      if (flapCloud1X < -30)
        flapCloud1X = G_R + 10;
      flapCloud2X -= 0.7f;
      if (flapCloud2X < -30)
        flapCloud2X = G_R + 10;

      flapPipeX -= SCREEN_WIDTH == 240 ? 2.2f : 1.5f;
      if (flapPipeX < G_X - pW) {
        flapPipeX = G_R;
        flapPipeGapY = G_Y + 20 + random(0, G_H - gapH - 40);
        flapScore++;
        audio.playSound(SOUND_CHIRP);
      }

      if (flapPlayerY < G_Y || flapPlayerY > G_B - mSize * 2) {
        flapGameOver = true;
        gameOverActive = true;
        audio.playSound(SOUND_GAMEOVER);
        saveHighScore("flap", flapHighScore, flapScore);
      }
      if (flapPipeX > G_X + G_W / 2 - pW && flapPipeX < G_X + G_W / 2 + mSize) {
        if (flapPlayerY < flapPipeGapY ||
            flapPlayerY > flapPipeGapY + gapH - mSize * 2) {
          flapGameOver = true;
          gameOverActive = true;
          audio.playSound(SOUND_GAMEOVER);
          saveHighScore("flap", flapHighScore, flapScore);
        }
      }
    } else {
      if (btn1 && !lastBtn1)
        resetFlappy();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Sky gradient (3 bands) ─────────────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H / 3, 0xAEFF);           // top light sky
    display.fillRect(G_X, G_Y + G_H / 3, G_W, G_H / 3, 0x7E5F); // mid sky
    display.fillRect(G_X, G_Y + 2 * G_H / 3, G_W, G_H - 2 * (G_H / 3), 0x5D1F); // bottom darker

    // Animated clouds
    auto drawCloud = [&](int cx, int cy) {
      display.fillCircle(cx, cy, 7, TFT_WHITE);
      display.fillCircle(cx + 10, cy - 2, 9, TFT_WHITE);
      display.fillCircle(cx + 20, cy, 7, TFT_WHITE);
    };
    drawCloud((int)flapCloud1X, G_Y + G_H / 6);
    drawCloud((int)flapCloud2X, G_Y + G_H / 4 + 10);

    // Ground
    display.fillRect(G_X, G_B - 10, G_W, 10, 0x03E0);
    display.fillRect(G_X, G_B - 10, G_W, 3, 0x07E0); // highlight
    for (int gx = G_X; gx < G_R; gx += 12)
      display.drawFastVLine(gx, G_B - 10, 10, 0x02A0);

    // Pipes
    if (flapPipeX >= G_X && flapPipeX < G_R) {
      const uint16_t pipeCol = 0x1AE2;
      const uint16_t pipeHL = 0x37E7;
      // Top pipe
      display.fillRect((int)flapPipeX, G_Y, pW, (int)(flapPipeGapY - G_Y),
                       pipeCol);
      display.fillRect((int)flapPipeX, G_Y, 3, (int)(flapPipeGapY - G_Y),
                       pipeHL);
      display.fillRect((int)flapPipeX - 2, (int)flapPipeGapY - 9, pW + 4, 9,
                       pipeCol);
      display.drawRect((int)flapPipeX - 2, (int)flapPipeGapY - 9, pW + 4, 9,
                       TFT_BLACK);
      // Bottom pipe
      int bPipeTop = (int)flapPipeGapY + gapH;
      display.fillRect((int)flapPipeX, bPipeTop, pW, G_B - bPipeTop, pipeCol);
      display.fillRect((int)flapPipeX, bPipeTop, 3, G_B - bPipeTop, pipeHL);
      display.fillRect((int)flapPipeX - 2, bPipeTop, pW + 4, 9, pipeCol);
      display.drawRect((int)flapPipeX - 2, bPipeTop, pW + 4, 9, TFT_BLACK);
    }

    // Mochy character — 2-frame wing animation based on VY
    {
      int px = G_X + G_W / 2 - mSize;
      int py = (int)flapPlayerY;
      // Body
      display.fillCircle(px + mSize, py + mSize, mSize, TFT_YELLOW);
      display.drawCircle(px + mSize, py + mSize, mSize, 0xFDA0);
      // Eye
      display.fillCircle(px + mSize + mSize / 2, py + mSize / 2, mSize / 3 + 1,
                         TFT_WHITE);
      display.fillCircle(px + mSize + mSize / 2 + 1, py + mSize / 2, mSize / 4,
                         TFT_BLACK);
      // Beak
      display.fillTriangle(px + mSize * 2 - 2, py + mSize, px + mSize * 2 + 3,
                           py + mSize + 2, px + mSize * 2 - 2, py + mSize + 4,
                           0xFD20);
      // Wing (flap based on VY)
      if (flapPlayerVY < 0) {
        display.fillTriangle(px, py + mSize, px - mSize - 2, py + mSize / 2, px,
                             py + mSize / 2, 0xFDA0);
      } else {
        display.fillTriangle(px, py + mSize, px - mSize - 2,
                             py + mSize + mSize / 2, px, py + mSize + 2,
                             0xFDA0);
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", flapScore);
    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54),
                      SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HI:%03d", flapHighScore);

    if (flapGameOver)
      drawGameOverScreen(display, flapScore, flapHighScore);
  }

  // ============================================================
  //  GAME 4: COIN CATCHER
  // ============================================================
  void updateAndDrawCatcher(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int basketW = SCREEN_WIDTH == 240 ? 46 : 22;
    const int basketH = SCREEN_WIDTH == 240 ? 16 : 8;
    const int coinR   = SCREEN_WIDTH == 240 ? 11 : 5;

    if (!catGameOver) {
      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2)
        steerDelta -= 4.2f;
      else if (btn2 && !btn1)
        steerDelta += 4.2f;
      if (fabsf(tilt) > 0.01f)
        steerDelta += tilt * 6.5f;
      catPlayerX = constrain(catPlayerX + steerDelta, (float)(G_X + 6),
                             (float)(G_R - basketW - 6));

      float fallSpeed = SCREEN_WIDTH == 240 ? (2.2f + catScore / 80.0f)
                                            : (1.4f + catScore / 80.0f);
      for (int i = 0; i < 3; i++) {
        if (catCoinsActive[i]) {
          catCoinsY[i] += fallSpeed;
          if (catCoinsY[i] > G_B - 6) {
            if (!catIsBomb[i]) {
              if (--catLives <= 0) {
                catGameOver = true;
                gameOverActive = true;
                audio.playSound(SOUND_GAMEOVER);
                saveHighScore("cat", catHighScore, catScore);
              } else
                audio.playSound(SOUND_POWERDOWN);
            }
            catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
            catCoinsY[i] = G_Y - 24;
            catIsBomb[i] = (random(0, 10) < 3);
          }
          if (catCoinsY[i] + coinR * 2 >= (G_B - basketH - 8) && catCoinsY[i] <= G_B - 10) {
            if (catCoinsX[i] + coinR * 2 >= catPlayerX &&
                catCoinsX[i] <= catPlayerX + basketW) {
              if (catIsBomb[i]) {
                if (--catLives <= 0) {
                  catGameOver = true;
                  gameOverActive = true;
                  audio.playSound(SOUND_GAMEOVER);
                  saveHighScore("cat", catHighScore, catScore);
                } else
                  audio.playSound(SOUND_POWERDOWN);
              } else {
                catScore += 10;
                audio.playSound(SOUND_COIN);
              }
              catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
              catCoinsY[i] = G_Y - 24;
              catIsBomb[i] = (random(0, 10) < 3);
            }
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1)
        resetCatcher();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Background: Animated Twilight Sunset & Hills ────────────────────
    unsigned long t = millis();
    int skyBandH = G_H / 3;
    display.fillRect(G_X, G_Y, G_W, skyBandH, 0x10A9);                 // deep twilight indigo
    display.fillRect(G_X, G_Y + skyBandH, G_W, skyBandH, 0x38CE);     // dusty plum purple
    display.fillRect(G_X, G_Y + 2 * skyBandH, G_W, G_H - 2 * skyBandH, 0x89C6); // warm sunset amber

    // Distant mountain silhouette (dark plum purple)
    display.fillTriangle(G_X + 10, G_B - 22, G_X + 70, G_B - 65, G_X + 130, G_B - 22, 0x2106);
    display.fillTriangle(G_X + 110, G_B - 22, G_X + 175, G_B - 52, G_X + 235, G_B - 22, 0x2106);

    // Mid-ground rolling hills (deep forest teal)
    display.fillTriangle(G_X, G_B - 14, G_X + 50, G_B - 36, G_X + 110, G_B - 14, 0x09E4);
    display.fillTriangle(G_X + 90, G_B - 14, G_X + 160, G_B - 42, G_X + 230, G_B - 14, 0x09E4);
    display.fillTriangle(G_X + 180, G_B - 14, G_X + 220, G_B - 30, G_R, G_B - 14, 0x09E4);

    // Floating animated golden fireflies drifting through twilight
    for (int p = 0; p < 7; p++) {
      int fx = (int)((p * 37 + (t / 40)) % G_W);
      int fy = G_Y + 12 + (int)((sinf((t / 300.0f) + p * 1.8f) * 12.0f) + (p * 24)) % (G_H - 50);
      uint16_t fCol = ((t / 200 + p) % 2 == 0) ? 0xFFE0 : 0xFD20;
      display.fillCircle(fx, fy, 2, fCol);
      display.drawPixel(fx, fy, TFT_WHITE);
    }

    // Lush meadow ground
    display.fillRect(G_X, G_B - 14, G_W, 14, 0x0A63);
    display.fillRect(G_X, G_B - 14, G_W, 3, 0x2586);
    for (int gx = G_X + 6; gx < G_R; gx += 14) {
      display.drawLine(gx, G_B - 14, gx + 2, G_B - 18, 0x47E0);
      display.drawLine(gx + 4, G_B - 14, gx + 2, G_B - 18, 0x47E0);
    }

    // Basket — Bigger textured wicker basket with weave and arched handles
    {
      int bx = (int)catPlayerX, by = G_B - basketH - 8;
      // Wicker body
      display.fillRoundRect(bx + 2, by + 4, basketW - 4, basketH - 3, 3, 0x9A60);
      // Wicker cross-hatch weave pattern
      for (int wx = bx + 6; wx < bx + basketW - 6; wx += 8) {
        display.drawLine(wx, by + 5, wx + 4, by + basketH, 0x62A0);
        display.drawLine(wx + 4, by + 5, wx, by + basketH, 0x62A0);
      }
      // Top rim
      display.fillRoundRect(bx, by, basketW, 5, 2, 0xD564);
      display.drawFastHLine(bx + 2, by + 1, basketW - 4, 0xFDE0);
      // Arched side handles
      display.drawCircle(bx - 2, by + 4, 3, 0xD564);
      display.drawCircle(bx + basketW + 1, by + 4, 3, 0xD564);
    }

    // Coins & bombs — Bigger size with rich animated details
    for (int i = 0; i < 3; i++) {
      if (catCoinsActive[i]) {
        int cx = (int)catCoinsX[i] + coinR, cy = (int)catCoinsY[i] + coinR;
        if (catIsBomb[i]) {
          // Cartoon Bomb body
          display.fillCircle(cx, cy, coinR, 0x18C3);
          display.fillCircle(cx - 3, cy - 3, 3, 0x52AA); // 3D gloss shine
          // Brass collar & fuse rope
          display.fillRect(cx - 3, cy - coinR - 2, 6, 3, 0x9482);
          display.drawLine(cx, cy - coinR - 2, cx + 4, cy - coinR - 6, 0xCE79);
          // Animated flickering fire spark
          bool sparkPhase = (t / 120 + i) % 2;
          display.fillCircle(cx + 4, cy - coinR - 7, sparkPhase ? 4 : 2, TFT_ORANGE);
          display.fillCircle(cx + 4, cy - coinR - 7, 1, TFT_YELLOW);
          // Warning crossbones mark
          display.drawLine(cx - 3, cy - 2, cx + 3, cy + 4, 0xF800);
          display.drawLine(cx + 3, cy - 2, cx - 3, cy + 4, 0xF800);
        } else {
          // Large Gold Coin with shimmer & bevel
          display.fillCircle(cx, cy, coinR, 0xD4A0);
          display.fillCircle(cx, cy, coinR - 2, 0xFFE0);
          display.drawCircle(cx, cy, coinR - 1, 0xFFF8);
          // Specular gleam shine
          if ((t / 200 + i) % 3 == 0) {
            display.fillCircle(cx - 3, cy - 3, 2, TFT_WHITE);
          } else {
            display.drawPixel(cx - 3, cy - 3, TFT_WHITE);
          }
          // Big embossed $ symbol
          display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
          display.setTextColor(0xB380);
          display.setCursor(cx - (SCREEN_WIDTH == 240 ? 5 : 2), cy - (SCREEN_WIDTH == 240 ? 7 : 3));
          display.print("$");
        }
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", catScore);

    // Heart lives
    for (int i = 0; i < 3; i++) {
      int hx = SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 54 : 32) +
               i * (SCREEN_WIDTH == 240 ? 18 : 10);
      int hy = SCREEN_WIDTH == 240 ? 38 : 9;
      drawHeart(display, hx, hy, 3, i < catLives ? 0xF800 : 0x4208,
                i < catLives);
    }

    if (catGameOver)
      drawGameOverScreen(display, catScore, catHighScore);
  }

  // ============================================================
  //  GAME 5: MOCHY JUMP
  // ============================================================
  void updateAndDrawJump(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int platW = SCREEN_WIDTH == 240 ? 36 : 24;
    const int pW = SCREEN_WIDTH == 240 ? 14 : 9;
    const int pH = SCREEN_WIDTH == 240 ? 16 : 10;

    float steerDelta = 0.0f; // hoisted to function scope for draw section

    if (!jumpGameOver) {
      float tilt = getTiltSteer();
      if (btn1 && !btn2)
        steerDelta -= 3.2f;
      else if (btn2 && !btn1)
        steerDelta += 3.2f;
      if (fabsf(tilt) > 0.01f)
        steerDelta += tilt * 5.2f;
      jumpPlayerX += steerDelta;

      // Wrap around
      if (jumpPlayerX < G_X - pW)
        jumpPlayerX = G_R - pW;
      if (jumpPlayerX > G_R + pW)
        jumpPlayerX = G_X - pW;

      jumpPlayerVY += 0.18f;
      jumpPlayerY += jumpPlayerVY;

      jumpSquash = false;

      if (jumpPlayerY < G_Y + G_H / 2) {
        float diff = (G_Y + G_H / 2) - jumpPlayerY;
        jumpPlayerY = G_Y + G_H / 2;
        jumpScore += (int)(diff / 2);
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
          if (jumpPlayerX + pW >= jumpPlatX[i] &&
              jumpPlayerX <= jumpPlatX[i] + platW &&
              jumpPlayerY + pH >= jumpPlatY[i] &&
              jumpPlayerY + pH - 4 <= jumpPlatY[i] + 4) {
            jumpPlayerVY = SCREEN_WIDTH == 240 ? -5.8f : -4.8f;
            jumpSquash = true;
            audio.playSound(SOUND_JUMP);
            break;
          }
        }
      }

      if (jumpPlayerY > G_B) {
        jumpGameOver = true;
        gameOverActive = true;
        audio.playSound(SOUND_GAMEOVER);
        saveHighScore("jump", jumpHighScore, jumpScore);
      }
    } else {
      if (btn1 && !lastBtn1)
        resetJump();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Background: cosmic grid ───────────────────────────────────────────
    display.fillScreen(0x000F);
    // Near grid (bright)
    for (int x = G_X + 20; x < G_R; x += 20)
      display.drawFastVLine(x, G_Y, G_H, 0x0842);
    for (int y = G_Y + 20; y < G_B; y += 20)
      display.drawFastHLine(G_X, y, G_W, 0x0842);
    // Far grid (dim, offset)
    for (int x = G_X + 10; x < G_R; x += 20)
      display.drawFastVLine(x, G_Y, G_H, 0x0421);
    for (int y = G_Y + 10; y < G_B; y += 20)
      display.drawFastHLine(G_X, y, G_W, 0x0421);

    // Platforms with shimmer glow
    unsigned long t = millis();
    for (int i = 0; i < 5; i++) {
      int px = (int)jumpPlatX[i], py = (int)jumpPlatY[i];
      display.fillRoundRect(px, py, platW, 4, 1, themeAccent);
      // Shimmer sweep
      int shimX = px + (int)((t / 500 + i * 30) % platW);
      display.drawPixel(shimX, py, TFT_WHITE);
      display.drawPixel(shimX + 1, py, TFT_WHITE);
      display.drawRoundRect(px, py, platW, 4, 1, TFT_WHITE);
    }

    // Player — squash on land, normal otherwise
    {
      int px = (int)jumpPlayerX, py = (int)jumpPlayerY;
      int drawH = jumpSquash ? pH - 3 : pH;
      int drawY = jumpSquash ? py + 3 : py;
      display.fillRoundRect(px, drawY, pW, drawH, 2, TFT_YELLOW);
      // Eyes (look in movement direction)
      int eyeOff = (steerDelta > 0) ? 2 : (steerDelta < 0 ? 0 : 1);
      display.fillCircle(px + eyeOff, drawY + drawH / 4, 1, TFT_BLACK);
      display.fillCircle(px + eyeOff + 4, drawY + drawH / 4, 1, TFT_BLACK);
    }

    // Ghost indicator at opposite edge when wrapping near edge
    if (jumpPlayerX < G_X + 10) {
      display.drawRoundRect(G_R - pW - 2, (int)jumpPlayerY, pW, pH, 2, 0x4A49);
    } else if (jumpPlayerX > G_R - 10) {
      display.drawRoundRect(G_X + 2, (int)jumpPlayerY, pW, pH, 2, 0x4A49);
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("ALT:%04d", jumpScore);
    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54),
                      SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HI:%04d", jumpHighScore);

    if (jumpGameOver)
      drawGameOverScreen(display, jumpScore, jumpHighScore);
  }

  // ============================================================
  //  GAME 6: STACKER
  // ============================================================
  void updateAndDrawStacker(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int blockH = SCREEN_WIDTH == 240 ? 12 : 8;

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
        stkFlashNew = false;
        if (stkStackHeight == 0) {
          stkBaseX = stkBlockX;
          stkStackX[0] = stkBlockX;
          stkStackW[0] = stkBlockW;
          stkStackHeight = 1;
          stkScore += 10;
          audio.playSound(SOUND_COIN);
          stkFlashNew = true;
        } else {
          float prevX = stkStackX[stkStackHeight - 1];
          float prevW = stkStackW[stkStackHeight - 1];
          float oL = max(stkBlockX, prevX);
          float oR = min(stkBlockX + stkBlockW, prevX + prevW);
          float oW = oR - oL;
          if (oW <= 0) {
            stkGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("stk", stkHighScore, stkScore);
          } else {
            stkBlockW = oW;
            stkBlockX = oL;
            if (stkStackHeight < 10) {
              stkStackX[stkStackHeight] = oL;
              stkStackW[stkStackHeight] = oW;
              stkStackHeight++;
            } else {
              for (int i = 0; i < 9; i++) {
                stkStackX[i] = stkStackX[i + 1];
                stkStackW[i] = stkStackW[i + 1];
              }
              stkStackX[9] = oL;
              stkStackW[9] = oW;
            }
            stkScore += 10;
            stkSpeed += 0.25f;
            stkFlashNew = true;
            audio.playSound(SOUND_COIN);
          }
        }
      } else {
        stkFlashNew = false;
      }
    } else {
      if (btn1 && !lastBtn1)
        resetStacker();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Background: neon violet grid ─────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H, 0x1804);
    for (int y = G_Y; y < G_B; y += blockH + 2)
      display.drawFastHLine(G_X, y, G_W, 0x2104);
    // Vertical guide line
    display.drawFastVLine(G_X + G_W / 2, G_Y, G_H, 0x2905);

    // Right-side stack progress bar
    {
      int pbX = G_R - 6, pbH = G_H;
      display.fillRect(pbX, G_Y, 5, pbH, 0x0841);
      int fillH = stkStackHeight * pbH / 12;
      display.fillRect(pbX, G_B - fillH, 5, fillH, themeAccent);
    }

    // Stacked blocks — gradient fill: top brighter, bottom darker
    for (int i = 0; i < stkStackHeight; i++) {
      int blockY = G_B - (blockH + 2) - i * (blockH + 2);
      uint16_t topCol =
          (i == stkStackHeight - 1 && stkFlashNew) ? TFT_WHITE : themeAccent;
      uint16_t botCol = (i == stkStackHeight - 1 && stkFlashNew)
                            ? 0xC000
                            : blend565(themeAccent, 0x0000);
      display.fillRoundRect((int)stkStackX[i], blockY, (int)stkStackW[i],
                            blockH, 2, topCol);
      display.fillRect((int)stkStackX[i], blockY + blockH / 2,
                       (int)stkStackW[i], blockH / 2, botCol);
      display.drawRoundRect((int)stkStackX[i], blockY, (int)stkStackW[i],
                            blockH, 2, TFT_WHITE);
      // Top highlight strip
      display.drawFastHLine((int)stkStackX[i] + 2, blockY + 1,
                            (int)stkStackW[i] - 4, blend565(topCol, TFT_WHITE));
    }

    // Moving active block
    if (!stkGameOver) {
      int curY = G_B - (blockH + 2) - stkStackHeight * (blockH + 2);
      unsigned long t = millis();
      uint16_t blockCol = (t / 300) % 2 ? TFT_YELLOW : 0xFFE0;
      display.fillRoundRect((int)stkBlockX, curY, (int)stkBlockW, blockH, 2,
                            blockCol);
      display.drawRoundRect((int)stkBlockX, curY, (int)stkBlockW, blockH, 2,
                            TFT_WHITE);
      display.drawFastHLine((int)stkBlockX + 2, curY + 1, (int)stkBlockW - 4,
                            TFT_WHITE);
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("STCK:%03d", stkScore);
    display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 90 : 54),
                      SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("HI:%03d", stkHighScore);

    if (stkGameOver)
      drawGameOverScreen(display, stkScore, stkHighScore);
  }

  // ============================================================
  //  GAME 7: MEMORY MATRIX
  // ============================================================
  void updateAndDrawMemory(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int gridSpacing = SCREEN_WIDTH == 240 ? 42 : 28;
    const int gridStartX = G_X + (G_W - 3 * gridSpacing) / 2;
    const int gridStartY = G_Y + (G_H - 3 * gridSpacing) / 2;

    if (!memGameOver) {
      if (memState == 0) {
        unsigned long elapsed = millis() - memTimer;
        const int stepTime = 600;
        const int gapTime = 200;
        const int cycleTime = stepTime + gapTime;
        int currentStep = elapsed / cycleTime;
        if (currentStep < memSeqLen) {
          memSeqStep = ((elapsed % cycleTime) < (unsigned long)stepTime)
                           ? memSeq[currentStep]
                           : -1;
        } else {
          memState = 1;
          memPlayerStep = 0;
          memSeqStep = -1;
        }
      } else if (memState == 1) {
        if (btn1 && !lastBtn1) {
          memSelectedTile = (memSelectedTile + 1) % 9;
          audio.playSound(SOUND_CHIRP);
        }
        if (btn2 && !lastBtn2) {
          if (memSelectedTile == memSeq[memPlayerStep]) {
            audio.playSound(SOUND_JUMP);
            if (++memPlayerStep == memSeqLen) {
              memScore += 10;
              memSeqLen++;
              if (memSeqLen > 15)
                memSeqLen = 15;
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
      if (btn1 && !lastBtn1)
        resetMemory();
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // ── Background: deep cyber blue ───────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H, 0x000F);
    // Subtle grid lines
    for (int gx = gridStartX - 4; gx <= gridStartX + 3 * gridSpacing;
         gx += gridSpacing)
      display.drawFastVLine(gx, gridStartY, 3 * gridSpacing, 0x0841);
    for (int gy = gridStartY - 4; gy <= gridStartY + 3 * gridSpacing;
         gy += gridSpacing)
      display.drawFastHLine(gridStartX, gy, 3 * gridSpacing, 0x0841);

    // Tiles
    unsigned long t = millis();
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        int tileIdx = r * 3 + c;
        int tx = gridStartX + c * gridSpacing + 3;
        int ty = gridStartY + r * gridSpacing + 3;
        int tw = gridSpacing - 6;
        int th = gridSpacing - 6;

        uint16_t tileColor = 0x0182;
        bool isActive = false;

        if (memState == 0 && memSeqStep == tileIdx) {
          tileColor = TFT_YELLOW;
          isActive = true;
        } else if (memState == 1 && memSelectedTile == tileIdx) {
          tileColor = themeAccent;
          isActive = true;
        }

        display.fillRoundRect(tx, ty, tw, th, 5, tileColor);
        display.drawRoundRect(tx, ty, tw, th, 5, isActive ? TFT_WHITE : 0x18C3);
        // Extra glow ring for active tile
        if (isActive) {
          display.drawRoundRect(tx - 1, ty - 1, tw + 2, th + 2, 6,
                                blend565(tileColor, TFT_WHITE));
        }
        // Tile number dimly
        if (!isActive) {
          display.setTextSize(1);
          display.setTextColor(0x18C3);
          display.setCursor(tx + tw / 2 - 3, ty + th / 2 - 4);
          display.print(tileIdx + 1);
        }
      }
    }

    // Progress dots below grid (show how far into input sequence)
    if (memState == 1 && memSeqLen > 0) {
      int dotY = gridStartY + 3 * gridSpacing + 8;
      int dotW = min(memSeqLen, 15) * 8;
      int dotX = G_X + (G_W - dotW) / 2;
      for (int d = 0; d < min(memSeqLen, 15); d++) {
        uint16_t dc = (d < memPlayerStep)
                          ? 0x07E0
                          : (d == memPlayerStep ? TFT_YELLOW : 0x4208);
        display.fillCircle(dotX + d * 8 + 3, dotY + 3, 3, dc);
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("SC:%03d", memScore);

    // Phase badge
    {
      int badgeX = SCREEN_WIDTH / 2 - 30, badgeY = SCREEN_WIDTH == 240 ? 30 : 4;
      uint16_t badgeCol = (memState == 0) ? 0xF800 : 0x03E0;
      display.fillRoundRect(badgeX, badgeY, 60, SCREEN_WIDTH == 240 ? 16 : 12,
                            4, badgeCol);
      display.setTextSize(1);
      display.setTextColor(TFT_WHITE);
      display.setCursor(badgeX + 4, badgeY + (SCREEN_WIDTH == 240 ? 4 : 2));
      display.print(memState == 0 ? " WATCH  " : "  INPUT ");
    }

    if (memState == 1 && !memGameOver) {
      display.setTextSize(1);
      display.setTextColor(0x4A49);
      display.setCursor(SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 100 : 60),
                        SCREEN_WIDTH == 240 ? 36 : 7);
      display.printf("S%d/%d", memPlayerStep + 1, memSeqLen);
    }

    if (memGameOver)
      drawGameOverScreen(display, memScore, memHighScore);
  }

  // ============================================================
  //  GAME 6: HIDE & SEEK (TIME BOMB DETONATION GAME)
  // ============================================================
  void updateAndDrawHideSeek(GFXcanvas16 &display, LunaAudio &audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8;
    const unsigned long now = millis();

    // Touch input state
    bool isDown = isTouchActive();
    int touchX = getTouchX();
    int touchY = getTouchY();
    bool justTapped = isDown && !hsLastTouchDown;
    hsLastTouchDown = isDown;

    const bool btn1 = virtualBtn1;
    const bool justBtn1 = btn1 && !lastBtn1;
    lastBtn1 = btn1;

    // ── Global Sound Toggle Check (Top-Right: X >= 180, Y <= 50) ──
    if (justTapped && touchX >= 180 && touchY <= 50) {
      toggleSound(audio);
    }

    // ──────────────────────────────────────────────────────────
    // PART 1: INPUT PROCESSING & STATE MACHINE TRANSITIONS
    // ──────────────────────────────────────────────────────────
    if (hsState == HS_STATE_TITLE) {
      gameOverActive = false;
      hsGameOver = false;

      if (justTapped && !(touchX >= 180 && touchY <= 50)) {
        // Back button (top-left: X=[5..55], Y=[5..55])
        if (touchX <= 55 && touchY <= 55) {
          gamePlaying = false;
          gamesActive = false;
          currentScreen = SCREEN_MENU;
          appLauncherPage = 1;
          audio.playSound(SOUND_CHIRP);
          return;
        }

        // Play Button / Screen Tap
        hsState = HS_STATE_SETUP;
        audio.playSound(SOUND_POWERUP);
      }
    }
    else if (hsState == HS_STATE_SETUP) {
      gameOverActive = false;
      hsGameOver = false;

      if (justTapped && !(touchX >= 180 && touchY <= 50)) {
        // 1. Back button (top left circle: X=[5..65], Y=[5..65])
        if (touchX <= 65 && touchY <= 65) {
          hsState = HS_STATE_TITLE;
          audio.playSound(SOUND_CHIRP);
        }
        // 2. Minutes Up Arrow (▲): X=[50..110], Y=[85..125]
        else if (touchX >= 50 && touchX <= 110 && touchY >= 85 && touchY <= 125) {
          hsTimerSeconds = min(600, hsTimerSeconds + 60);
          audio.playSound(SOUND_JUMP);
        }
        // 3. Minutes Down Arrow (▼): X=[50..110], Y=[165..205]
        else if (touchX >= 50 && touchX <= 110 && touchY >= 165 && touchY <= 205) {
          hsTimerSeconds = max(15, hsTimerSeconds - 60);
          audio.playSound(SOUND_JUMP);
        }
        // 4. Seconds Up Arrow (▲): X=[130..190], Y=[85..125]
        else if (touchX >= 130 && touchX <= 190 && touchY >= 85 && touchY <= 125) {
          hsTimerSeconds = min(600, hsTimerSeconds + 15);
          audio.playSound(SOUND_JUMP);
        }
        // 5. Seconds Down Arrow (▼): X=[130..190], Y=[165..205]
        else if (touchX >= 130 && touchX <= 190 && touchY >= 165 && touchY <= 205) {
          hsTimerSeconds = max(15, hsTimerSeconds - 15);
          audio.playSound(SOUND_JUMP);
        }
        // 6. Yellow Checkmark Button (Confirm): X=[20..220], Y=[210..265]
        else if (touchY >= 210 && touchY <= 265 && touchX >= 20 && touchX <= 220) {
          hsState = HS_STATE_HIDING;
          hsPhaseStartTime = now;
          hsHidingDurationMs = 10000; // 10-second hiding countdown
          hsLastCountdownTickMs = now;
          audio.playSound(SOUND_STARTUP);
        }
      }
    }
    else if (hsState == HS_STATE_HIDING) {
      // Handle Back button
      if (justTapped && !(touchX >= 180 && touchY <= 50) && touchX <= 60 && touchY <= 60) {
        hsState = HS_STATE_SETUP;
        audio.playSound(SOUND_CHIRP);
      } else {
        long hideElapsed = (long)(now - hsPhaseStartTime);
        long hideLeft = (long)hsHidingDurationMs - hideElapsed;
        int secLeft = (int)(hideLeft / 1000) + 1;

        // Per-second countdown audio tick
        if (now - hsLastCountdownTickMs >= 1000 && secLeft > 0 && secLeft <= 10) {
          hsLastCountdownTickMs = now;
          if (secLeft <= 3) {
            audio.playSound(SOUND_ALERT_BEEP); // Urgent pitch on final 3 seconds
          } else {
            audio.playSound(SOUND_CHIRP); // Crisp tick beep each second
          }
        }

        // Tap screen to skip hiding phase, or when timer expires
        if ((justTapped && !(touchX >= 180 && touchY <= 50) && touchY > 60) || hideLeft <= 0) {
          hsState = HS_STATE_ACTIVE;
          hsPhaseStartTime = now;
          hsBombDurationMs = hsTimerSeconds * 1000UL;
          hsLastTickMs = 0;
          hsSoundWaveRadius = 0;
          audio.playSound(SOUND_ALERT_BEEP);
        }
      }
    }
    else if (hsState == HS_STATE_ACTIVE) {
      // Back button
      if (justTapped && !(touchX >= 180 && touchY <= 50) && touchX <= 55 && touchY <= 55) {
        hsState = HS_STATE_TITLE;
        audio.stopBuzzer();
        audio.playSound(SOUND_CHIRP);
      } else {
        long activeElapsed = (long)(now - hsPhaseStartTime);
        long remainingMs = (long)hsBombDurationMs - activeElapsed;

        // ── TIME RUN OUT -> DETONATION! ──
        if (remainingMs <= 0) {
          hsState = HS_STATE_EXPLODED;
          hsGameOver = true;
          gameOverActive = true;
          hsExplosionFrame = 0;
          audio.stopBuzzer();
          audio.playSound(SOUND_EXPLOSION);
        }
        // ── SEEKER DEFUSED THE BOMB! ──
        else if (justTapped && !(touchX >= 180 && touchY <= 50) && touchY > 55) {
          hsState = HS_STATE_DEFUSED;
          hsDefusedRemainingMs = max(0L, remainingMs);
          hsGameOver = true;
          gameOverActive = true;
          audio.stopBuzzer();
          audio.playSound(SOUND_DEFUSED);
          saveHighScore("hs", hsHighScore, (int)(remainingMs / 1000));
        }
        else {
          // Calculate progress ratio (0.0f to 1.0f)
          float ratio = (float)activeElapsed / (float)hsBombDurationMs;
          ratio = constrain(ratio, 0.0f, 1.0f);

          // Sound ticking system
          int interval;
          if (remainingMs <= 5000) {
            interval = 140; // Final 5 seconds frantic
          } else if (remainingMs <= 12000) {
            interval = 280;
          } else {
            interval = (int)(2600.0f - (ratio * 2100.0f));
            if (interval < 350) interval = 350;
          }

          int vol = (int)(4.0f + ratio * 96.0f);
          vol = constrain(vol, 4, 100);

          int freq = (int)(650.0f + (ratio * ratio) * 2450.0f);
          if (remainingMs <= 5000) freq = 3100;

          int dur = (int)(18.0f + ratio * 55.0f);

          if (now - hsLastTickMs >= (unsigned long)interval) {
            hsLastTickMs = now;
            hsSoundWaveRadius = 8;
            audio.playVolumeBeep(freq, dur, vol);
          }
        }
      }
    }
    else if (hsState == HS_STATE_EXPLODED) {
      gameOverActive = true;
      hsGameOver = true;

      if ((justTapped && !(touchX >= 180 && touchY <= 50)) || justBtn1) {
        // 1. Back button (top-left: X=[5..60], Y=[5..60])
        // 2. Home button (bottom-right: X=[165..235], Y=[195..265])
        if ((touchX <= 60 && touchY <= 60) || (touchX >= 165 && touchY >= 195)) {
          gamePlaying = false;
          gamesActive = false;
          gameOverActive = false;
          currentScreen = SCREEN_MENU;
          appLauncherPage = 1;
          audio.playSound(SOUND_CHIRP);
          return;
        }

        // 3. Play Again button (bottom-left: X=[15..165], Y=[195..265]) or screen tap
        resetHideSeek();
        audio.playSound(SOUND_POWERUP);
      }
    }
    else if (hsState == HS_STATE_DEFUSED) {
      gameOverActive = true;
      hsGameOver = true;

      if ((justTapped && !(touchX >= 180 && touchY <= 50)) || justBtn1) {
        // 1. Back button (top-left: X=[5..60], Y=[5..60])
        // 2. Home button (bottom-right: X=[165..235], Y=[195..265])
        if ((touchX <= 60 && touchY <= 60) || (touchX >= 165 && touchY >= 195)) {
          gamePlaying = false;
          gamesActive = false;
          gameOverActive = false;
          currentScreen = SCREEN_MENU;
          appLauncherPage = 1;
          audio.playSound(SOUND_CHIRP);
          return;
        }

        // 3. Play Again button (bottom-left: X=[15..165], Y=[195..265]) or screen tap
        resetHideSeek();
        audio.playSound(SOUND_POWERUP);
      }
    }

    // ──────────────────────────────────────────────────────────
    // PART 2: ATOMIC RENDERING (ALWAYS RENDERS CURRENT STATE IN THE SAME FRAME)
    // ──────────────────────────────────────────────────────────
    if (hsState == HS_STATE_TITLE) {
      gameOverActive = false;
      hsGameOver = false;

      // Render pristine title wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_title_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);
    }
    else if (hsState == HS_STATE_SETUP) {
      gameOverActive = false;
      hsGameOver = false;

      // Render pristine setup wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_timer_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);

      // Digital time readout inside cutouts
      int m = hsTimerSeconds / 60;
      int s = hsTimerSeconds % 60;

      display.setTextSize(3);
      display.setTextColor(0xFFFF);
      // Minutes display (cutout centered around X=78, Y=142)
      display.setCursor(62, 132);
      display.printf("%02d", m);

      // Seconds display (cutout centered around X=160, Y=142)
      display.setCursor(144, 132);
      display.printf("%02d", s);
    }
    else if (hsState == HS_STATE_HIDING) {
      // Render pristine go-hide wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_gohide_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);

      long hideElapsed = (long)(now - hsPhaseStartTime);
      long hideLeft = (long)hsHidingDurationMs - hideElapsed;
      int secLeft = max(1, (int)(hideLeft / 1000) + 1);

      // Hiding Countdown Badge
      char cntBuf[16];
      snprintf(cntBuf, sizeof(cntBuf), "%ds", secLeft);

      // Floating countdown badge at center-bottom
      display.fillRoundRect(80, 215, 80, 28, 14, 0x18C6);
      display.drawRoundRect(80, 215, 80, 28, 14, 0xFD20);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      int bw = strlen(cntBuf) * 12;
      display.setCursor(80 + (80 - bw) / 2, 222);
      display.print(cntBuf);
    }
    else if (hsState == HS_STATE_ACTIVE) {
      // Render pristine active bomb wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_active_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);

      long activeElapsed = (long)(now - hsPhaseStartTime);
      long remainingMs = max(0L, (long)hsBombDurationMs - activeElapsed);

      // Render live countdown inside the cat bomb's display screen cutout
      // Display cutout center: X=120, Y=155
      int remSec = (int)(remainingMs / 1000);
      int bm = remSec / 60;
      int bs = remSec % 60;

      char bombTimeBuf[16];
      snprintf(bombTimeBuf, sizeof(bombTimeBuf), "%02d:%02d", bm, bs);

      display.setTextSize(3);
      // Glow color based on danger level
      uint16_t timeCol = (remainingMs <= 10000) ? 0xF800 : ((remainingMs <= 30000) ? 0xFD20 : 0x07E0);
      display.setTextColor(timeCol);
      int tw = strlen(bombTimeBuf) * 18;
      display.setCursor((SCREEN_WIDTH - tw) / 2, 145);
      display.print(bombTimeBuf);
    }
    else if (hsState == HS_STATE_EXPLODED) {
      // Render pristine lost/exploded wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_exploded_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);
    }
    else if (hsState == HS_STATE_DEFUSED) {
      // Render pristine won/congratulations wallpaper + Sound icon
      memcpy_P(display.getBuffer(), hideseek_win_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);
    }
  }

  // ──────────────────────────────────────────────────────────
  //  WORD-WRAPPING HELPER FOR GAME CARDS
  // ──────────────────────────────────────────────────────────
  void drawCenteredWrappedText(GFXcanvas16 &display, const char *text, int boxX, int boxY, int boxW, int lineH, uint8_t textSize, uint16_t color) {
    display.setTextSize(textSize);
    display.setTextColor(color);
    int charW = (textSize == 1) ? 6 : (textSize == 2 ? 12 : 18);
    int maxCharsPerLine = boxW / charW;
    if (maxCharsPerLine < 1) maxCharsPerLine = 1;

    int len = strlen(text);
    int start = 0;
    int curY = boxY;

    while (start < len) {
      int end = start + maxCharsPerLine;
      if (end >= len) {
        end = len;
      } else {
        int space = end;
        while (space > start && text[space] != ' ') {
          space--;
        }
        if (space > start) {
          end = space;
        }
      }

      int lineChars = end - start;
      char lineBuf[64];
      if (lineChars >= (int)sizeof(lineBuf)) lineChars = sizeof(lineBuf) - 1;
      strncpy(lineBuf, text + start, lineChars);
      lineBuf[lineChars] = '\0';

      int lineW = lineChars * charW;
      int lineX = boxX + (boxW - lineW) / 2;
      display.setCursor(lineX, curY);
      display.print(lineBuf);

      curY += lineH;
      start = end;
      while (start < len && text[start] == ' ') {
        start++;
      }
    }
  }

  // ============================================================
  //  GAME 7: TRUTH OR DARE (UNIVERSAL / TEEN FRIENDLY)
  // ============================================================
  int todState = 0; // 0 = TITLE, 1 = CHOICE, 2 = TRUTH, 3 = DARE
  int todChoice = 0; // 0 = TRUTH, 1 = DARE
  int todPromptIdx = 0;
  bool todLastTouchDown = false;

  void resetTruthOrDare() {
    todState = 0;
    todChoice = 0;
    todPromptIdx = random(0, 25);
    todLastTouchDown = false;
  }

  void updateAndDrawTruthOrDare(GFXcanvas16 &display, LunaAudio &audio) {
    const unsigned long now = millis();
    bool isDown = isTouchActive();
    int touchX = getTouchX();
    int touchY = getTouchY();
    bool justTapped = isDown && !todLastTouchDown;
    todLastTouchDown = isDown;

    static const char *const todTruths[25] = {
      "What is your weirdest habit when alone?",
      "If you could trade lives with anyone here for a day, who?",
      "What is the funniest thing you believed as a child?",
      "What is the most useless thing you spent money on?",
      "What is your biggest fear that makes no sense?",
      "What is your most played guilty pleasure song?",
      "Who was your first cartoon character crush?",
      "What is one rule you secretly love breaking?",
      "What food do you pretend to like to be polite?",
      "Have you ever blamed a pet for something you broke?",
      "If you had a superpower for 1 hour, what would it be?",
      "What is the most embarrassing haircut you ever had?",
      "What is the weirdest dream you remember having?",
      "What's something silly you thought was true until recently?",
      "If you found $1,000 right now, what do you buy first?",
      "What movie can you watch 100 times without getting bored?",
      "What is the weirdest food combo you secretly enjoy?",
      "If your pet could talk for 1 minute, what would it say?",
      "What is the worst fashion disaster you ever wore?",
      "If you were stranded on an island, who here do you take?",
      "What is a secret talent you haven't told anyone?",
      "What is the longest you stayed awake and why?",
      "What is one song that always gets stuck in your head?",
      "If you could rename yourself right now, what name?",
      "What is your favorite comfort food when sad?"
    };

    static const char *const todDares[25] = {
      "Do your best impression of a cat begging for treats!",
      "Speak only in movie quotes for your next 2 turns!",
      "Do 10 squats while singing Happy Birthday dramatically!",
      "Talk without showing your teeth until the next round!",
      "Do a slow-motion catwalk across the room like a model!",
      "Pretend you are a robot running out of battery!",
      "Hold a staring contest with the person across from you!",
      "Speak in an opera singing voice for the next 1 minute!",
      "Balance a pen or spoon on your nose for 10 seconds!",
      "Act like you are swimming across the room with drama!",
      "Do your silliest dance move with a deadpan serious face!",
      "Say the alphabet backwards from Z to P without stopping!",
      "Pretend you are an alien visiting Earth for the first time!",
      "Give a dramatic 30-second speech about why cheese is great!",
      "Laugh hysterically for 15 seconds without stopping!",
      "Walk backwards every time you move for the next 2 minutes!",
      "Pretend your hands are dinosaur claws for 1 minute!",
      "Strike 3 different superhero poses and hold each for 5s!",
      "Try to juggle 3 small imaginary objects convincingly!",
      "Narrate everything another player does for the next 1 min!",
      "Pretend you are a secret agent on a stealth mission!",
      "Do your best impression of a grumpy grandpa!",
      "Talk like you are under water until your next turn!",
      "Recite a nursery rhyme with pure villain drama!",
      "Pretend the floor is hot lava for 30 seconds!"
    };

    // ── Global Sound Toggle Check (Top-Right: X >= 180, Y <= 50) ──
    if (justTapped && touchX >= 180 && touchY <= 50) {
      toggleSound(audio);
    }
    else if (justTapped) {
      // ──────────────────────────────────────────────────────────
      // PART 1: INPUT PROCESSING & STATE MACHINE TRANSITIONS
      // (State changes immediately without returning early, so rendering
      //  runs in the exact same frame — avoiding any black screen!)
      // ──────────────────────────────────────────────────────────
      if (todState == 0) {
        // Back button (top-left: X=[5..55], Y=[5..55])
        if (touchX <= 55 && touchY <= 55) {
          gamePlaying = false;
          gamesActive = false;
          currentScreen = SCREEN_MENU;
          appLauncherPage = 1;
          audio.playSound(SOUND_CHIRP);
          return;
        }

        // Tap anywhere else -> Go to Choice screen (State 1)
        todState = 1;
        audio.playSound(SOUND_POWERUP);
      }
      else if (todState == 1) {
        // Back button (top-left: X=[5..55], Y=[5..55])
        if (touchX <= 55 && touchY <= 55) {
          todState = 0;
          audio.playSound(SOUND_CHIRP);
        }
        // TRUTH Card (Left side: X < 120, Y >= 95)
        else if (touchX < 120 && touchY >= 95) {
          todChoice = 0;
          todPromptIdx = random(0, 25);
          todState = 2;
          audio.playSound(SOUND_COIN);
        }
        // DARE Card (Right side: X >= 120, Y >= 95)
        else if (touchX >= 120 && touchY >= 95) {
          todChoice = 1;
          todPromptIdx = random(0, 25);
          todState = 3;
          audio.playSound(SOUND_POWERUP);
        }
      }
      else if (todState == 2) {
        // Back button (top-left: X=[5..55], Y=[5..55])
        if (touchX <= 55 && touchY <= 55) {
          todState = 1;
          audio.playSound(SOUND_CHIRP);
        }
        // Action buttons at bottom (Y >= 205):
        else if (touchY >= 205) {
          // Green Checkmark [✔] (Left: X <= 88) -> Done!
          if (touchX <= 88) {
            audio.playSound(SOUND_COIN);
            todState = 1;
          }
          // Red X [✖] (Center: X in 89..152) -> Pass / Forfeit!
          else if (touchX <= 152) {
            audio.playSound(SOUND_ALERT_BEEP);
            todState = 1;
          }
          // Purple Reload [↺] (Right: X > 152) -> Next / Reroll!
          else {
            todPromptIdx = (todPromptIdx + 1 + random(1, 4)) % 25;
            audio.playSound(SOUND_JUMP);
          }
        }
      }
      else if (todState == 3) {
        // Back button (top-left: X=[5..55], Y=[5..55])
        if (touchX <= 55 && touchY <= 55) {
          todState = 1;
          audio.playSound(SOUND_CHIRP);
        }
        // Action buttons at bottom (Y >= 205):
        else if (touchY >= 205) {
          // Green Checkmark [✔] (Left: X <= 88) -> Done!
          if (touchX <= 88) {
            audio.playSound(SOUND_COIN);
            todState = 1;
          }
          // Red X [✖] (Center: X in 89..152) -> Pass / Forfeit!
          else if (touchX <= 152) {
            audio.playSound(SOUND_ALERT_BEEP);
            todState = 1;
          }
          // Purple Reload [↺] (Right: X > 152) -> Next / Reroll!
          else {
            todPromptIdx = (todPromptIdx + 1 + random(1, 4)) % 25;
            audio.playSound(SOUND_JUMP);
          }
        }
      }
    }

    // ──────────────────────────────────────────────────────────
    // PART 2: ATOMIC RENDERING (ALWAYS RENDERS CURRENT STATE IN SAME FRAME)
    // ──────────────────────────────────────────────────────────
    if (todState == 0) {
      // STATE 0: TITLE SCREEN (tod_title_bg)
      memcpy_P(display.getBuffer(), tod_title_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);
    }
    else if (todState == 1) {
      // STATE 1: CHOICE SCREEN (tod_choice_bg)
      memcpy_P(display.getBuffer(), tod_choice_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);
    }
    else if (todState == 2) {
      // STATE 2: TRUTH PROMPT SCREEN (tod_truth_bg)
      // 1. Render pristine Truth Wallpaper + Sound icon
      memcpy_P(display.getBuffer(), tod_truth_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);

      // 2. Render Truth prompt text inside pink card canvas (X: 25..215, Y: 115..205)
      const char *prompt = todTruths[todPromptIdx];
      uint8_t tSize = (strlen(prompt) > 65) ? 1 : 2;
      int lineH = (tSize == 1) ? 14 : 20;
      int startY = (tSize == 1) ? 122 : 126;
      drawCenteredWrappedText(display, prompt, 25, startY, 190, lineH, tSize, 0x40A8);
    }
    else if (todState == 3) {
      // STATE 3: DARE PROMPT SCREEN (tod_dare_bg)
      // 1. Render pristine Dare Wallpaper + Sound icon
      memcpy_P(display.getBuffer(), tod_dare_bg, 240 * 280 * sizeof(uint16_t));
      drawSoundToggleIcon(display, audio);

      // 2. Render Dare prompt text inside blue card canvas (X: 25..215, Y: 115..205)
      const char *prompt = todDares[todPromptIdx];
      uint8_t tSize = (strlen(prompt) > 65) ? 1 : 2;
      int lineH = (tSize == 1) ? 14 : 20;
      int startY = (tSize == 1) ? 122 : 126;
      drawCenteredWrappedText(display, prompt, 25, startY, 190, lineH, tSize, 0x09CD);
    }
  }

  // ============================================================
  //  GAME 8: IMPOSTER (SECRET AGENT DEDUCTION)
  // ============================================================
  int impState = 0; // 0=PLAYERS, 1=PASS, 2=REVEAL, 3=DISCUSS, 4=VOTE, 5=RESULT
  int impPlayerCount = 3; // 3..6
  int impCurrentPlayer = 0;
  int impImposterIdx = 0;
  int impWordIdx = 0;
  unsigned long impTimerStartMs = 0;
  bool impLastTouchDown = false;

  void resetImposter() {
    impState = 0;
    impCurrentPlayer = 0;
    impImposterIdx = random(0, impPlayerCount);
    impWordIdx = random(0, 30);
    impTimerStartMs = 0;
    impLastTouchDown = false;
  }

  void updateAndDrawImposter(GFXcanvas16 &display, LunaAudio &audio) {
    const unsigned long now = millis();
    bool isDown = isTouchActive();
    int touchX = getTouchX();
    int touchY = getTouchY();
    bool justTapped = isDown && !impLastTouchDown;
    impLastTouchDown = isDown;

    static const char *const impWords[30] = {
      "AIRPLANE", "HOSPITAL", "PIZZA", "CINEMA", "BEACH",
      "FOOTBALL", "SUBMARINE", "CASTLE", "CIRCUS", "BAKERY",
      "CAMPING", "LIBRARY", "ZOO", "PIRATE", "SPACE STATION",
      "VOLCANO", "CHOCOLATE", "AMUSEMENT PARK", "FIRE TRUCK", "MAGICIAN",
      "MUSEUM", "HOTEL", "RESTAURANT", "TRAIN", "GUITAR",
      "DOCTOR", "HELICOPTER", "DETECTIVE", "SWIMMING POOL", "SUPERMARKET"
    };

    static const char *const impCategories[30] = {
      "Travel", "Places", "Food", "Entertainment", "Vacation",
      "Sports", "Ocean", "Fantasy", "Entertainment", "Food",
      "Outdoors", "Places", "Animals", "Characters", "Sci-Fi",
      "Nature", "Food", "Places", "Vehicles", "Characters",
      "Places", "Travel", "Food", "Vehicles", "Music",
      "Jobs", "Vehicles", "Jobs", "Sports", "Places"
    };

    // 1. Base Layer: AI Generated 3D Detective Cyber-Lavender Wallpaper
    memcpy_P(display.getBuffer(), imposter_bg, 240 * 280 * sizeof(uint16_t));

    if (impState == 0) {
      // ── STATE 0: SELECT PLAYER COUNT (3..6) ───────────────────
      if (justTapped && touchX <= 45 && touchY <= 45) {
        gamePlaying = false;
        gamesActive = false;
        currentScreen = SCREEN_MENU;
        appLauncherPage = 1;
        audio.playSound(SOUND_CHIRP);
        return;
      }

      display.setTextSize(2);
      display.setTextColor(0x07FF);
      display.setCursor(24, 20);
      display.print("IMPOSTER 🕵️");

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      display.setCursor(34, 46);
      display.print("CHOOSE PLAYERS (3-6):");

      // 4 Player Count Chips: [ 3 ] [ 4 ] [ 5 ] [ 6 ] (Y=66..110)
      for (int i = 0; i < 4; i++) {
        int count = 3 + i;
        int cx = 28 + i * 48;
        int cy = 66;
        int cw = 40;
        int ch = 44;
        bool isSel = (impPlayerCount == count);
        display.fillRoundRect(cx, cy, cw, ch, 8, isSel ? 0xFD20 : 0x10A2);
        display.drawRoundRect(cx, cy, cw, ch, 8, isSel ? 0xFFFF : 0x3186);

        display.setTextSize(2);
        display.setTextColor(isSel ? 0x0000 : 0xFFFF);
        display.setCursor(cx + 14, cy + 14);
        display.print(count);

        if (justTapped && touchX >= cx && touchX <= cx + cw && touchY >= cy && touchY <= cy + ch) {
          impPlayerCount = count;
          audio.playSound(SOUND_COIN);
        }
      }

      // Mission Info inside card
      display.setTextSize(1);
      display.setTextColor(0xFDA0);
      display.setCursor(28, 124);
      display.print("* 1 player is secretly IMPOSTER!");
      display.setTextColor(0xCE79);
      display.setCursor(28, 140);
      display.print("* Teammates get same word.");
      display.setCursor(28, 156);
      display.print("* Imposter must blend in!");
      display.setTextColor(0x07FF);
      display.setCursor(28, 172);
      display.print("* Discuss & vote out the spy!");

      // [ START MISSION ▶ ] Button (Y=192..234)
      const int sX = 32, sY = 192, sW = SCREEN_WIDTH - 64, sH = 42;
      display.fillRoundRect(sX, sY, sW, sH, 10, 0x001F);
      display.drawRoundRect(sX, sY, sW, sH, 10, 0x07FF);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(sX + 22, sY + 13);
      display.print("START GAME ▶");

      if (justTapped && touchY >= sY) {
        impCurrentPlayer = 0;
        impImposterIdx = random(0, impPlayerCount);
        impWordIdx = random(0, 30);
        impState = 1;
        audio.playSound(SOUND_POWERUP);
      }
    } else if (impState == 1) {
      // ── STATE 1: PASS WATCH TO PLAYER X ───────────────────────
      display.setTextSize(2);
      display.setTextColor(0x07FF);
      display.setCursor(24, 20);
      display.print("SECRET HANDOFF");

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      display.setCursor(48, 52);
      display.print("PASS WATCH TO:");

      display.setTextSize(3);
      display.setTextColor(0xFFE0);
      char pStr[24];
      snprintf(pStr, sizeof(pStr), "PLAYER %d", impCurrentPlayer + 1);
      int pw = strlen(pStr) * 18;
      display.setCursor((SCREEN_WIDTH - pw) / 2, 78);
      display.print(pStr);

      display.setTextSize(1);
      display.setTextColor(0xF814);
      display.setCursor(34, 126);
      display.print("Keep screen hidden from others!");

      // [ TAP TO REVEAL ] Button (Y=188..234)
      const int rvX = 32, rvY = 188, rvW = SCREEN_WIDTH - 64, rvH = 46;
      display.fillRoundRect(rvX, rvY, rvW, rvH, 10, 0x2940);
      display.drawRoundRect(rvX, rvY, rvW, rvH, 10, 0xFD20);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(rvX + 16, rvY + 15);
      display.print("TAP TO REVEAL 👁");

      if (justTapped && touchY >= rvY) {
        impState = 2;
        audio.playSound(SOUND_CHIRP);
      }
    } else if (impState == 2) {
      // ── STATE 2: REVEAL ROLE TO PLAYER ────────────────────────
      bool isImposter = (impCurrentPlayer == impImposterIdx);

      if (isImposter) {
        // Red alert Imposter screen
        display.setTextSize(2);
        display.setTextColor(0xF800);
        display.setCursor(38, 20);
        display.print("TOP SECRET!");

        display.setTextSize(1);
        display.setTextColor(0xFFFF);
        display.setCursor(48, 48);
        display.print("YOU ARE THE...");

        display.setTextSize(3);
        display.setTextColor(0xFFE0);
        display.setCursor(42, 68);
        display.print("IMPOSTER");

        display.setTextSize(1);
        display.setTextColor(0xFDA0);
        char cBuf[48];
        snprintf(cBuf, sizeof(cBuf), "Category hint: %s", impCategories[impWordIdx]);
        display.setCursor(34, 108);
        display.print(cBuf);

        display.setTextColor(0xFFFF);
        display.setCursor(30, 130);
        display.print("You don't know the word!");
        display.setCursor(30, 144);
        display.print("Listen to clues & blend in!");
      } else {
        // Teammate screen
        display.setTextSize(1);
        display.setTextColor(0x07E0);
        display.setCursor(44, 20);
        display.print("YOU ARE A TEAMMATE 🛡");

        display.setTextColor(0xCE79);
        display.setCursor(56, 44);
        display.print("THE SECRET WORD:");

        display.setTextSize(2);
        display.setTextColor(0xFFFF);
        const char *wrd = impWords[impWordIdx];
        int ww = strlen(wrd) * 12;
        display.setCursor((SCREEN_WIDTH - ww) / 2, 66);
        display.print(wrd);

        display.setTextSize(1);
        display.setTextColor(0x07FF);
        char catStr[36];
        snprintf(catStr, sizeof(catStr), "[ Category: %s ]", impCategories[impWordIdx]);
        int cw = strlen(catStr) * 6;
        display.setCursor((SCREEN_WIDTH - cw) / 2, 98);
        display.print(catStr);

        display.setTextColor(0xCE79);
        display.setCursor(30, 126);
        display.print("Say 1 related word in round.");
        display.setCursor(30, 140);
        display.print("Don't make it too obvious!");
      }

      // [ HIDE & NEXT ▶ ] Button
      const int nX = 32, nY = 192, nW = SCREEN_WIDTH - 64, nH = 44;
      display.fillRoundRect(nX, nY, nW, nH, 10, 0x10A2);
      display.drawRoundRect(nX, nY, nW, nH, 10, 0xFFFF);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(nX + 18, nY + 14);
      display.print("HIDE & NEXT ▶");

      if (justTapped && touchY >= nY) {
        impCurrentPlayer++;
        if (impCurrentPlayer >= impPlayerCount) {
          impState = 3; // All players checked in, start discussion!
          impTimerStartMs = now;
          audio.playSound(SOUND_ALERT_BEEP);
        } else {
          impState = 1; // Pass to next player
          audio.playSound(SOUND_CHIRP);
        }
      }
    } else if (impState == 3) {
      // ── STATE 3: DISCUSSION & COUNTDOWN TIMER ─────────────────
      int elapsedSec = (now - impTimerStartMs) / 1000;
      int remSec = max(0, 90 - elapsedSec);

      display.setTextSize(2);
      display.setTextColor(0x07FF);
      display.setCursor(24, 18);
      display.print("DISCUSS CLUES ⏳");

      // Circular Sweep Progress Ring (CX=120, CY=95, R=38)
      const int cx = 120, cy = 95, r = 38;
      display.fillCircle(cx, cy, r, 0x1082);
      uint16_t ringCol = (remSec > 30) ? 0x07E0 : ((remSec > 10) ? 0xFD20 : 0xF800);
      display.drawCircle(cx, cy, r, ringCol);
      display.drawCircle(cx, cy, r - 1, ringCol);

      // Radar ripple sweep
      int rip = (now / 40) % (r - 4);
      display.drawCircle(cx, cy, rip, 0x3186);

      // MM:SS readout
      int m = remSec / 60;
      int s = remSec % 60;
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(cx - 30, cy - 7);
      display.printf("%02d:%02d", m, s);

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      display.setCursor(34, 146);
      display.print("Each player says 1 clue word!");
      display.setCursor(44, 160);
      display.print("Listen for suspicious words!");

      // [ VOTE NOW! 🗳 ] Button
      const int vX = 32, vY = 192, vW = SCREEN_WIDTH - 64, vH = 44;
      display.fillRoundRect(vX, vY, vW, vH, 10, 0x2000);
      display.drawRoundRect(vX, vY, vW, vH, 10, 0xF800);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(vX + 22, vY + 14);
      display.print("VOTE NOW! 🗳");

      if ((justTapped && touchY >= vY) || remSec == 0) {
        impState = 4;
        audio.playSound(SOUND_ALERT_BEEP);
      }
    } else if (impState == 4) {
      // ── STATE 4: VOTING ACCUSATION ────────────────────────────
      display.setTextSize(2);
      display.setTextColor(0xFFE0);
      display.setCursor(34, 20);
      display.print("VOTING TIME! 🕵️");

      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(40, 56);
      display.print("All players count down:");
      display.setTextSize(2);
      display.setTextColor(0xFFE0);
      display.setCursor(62, 78);
      display.print("3... 2... 1!");
      display.setTextSize(1);
      display.setTextColor(0xCE79);
      display.setCursor(46, 110);
      display.print("Point at your suspect!");
      display.setTextColor(0xFDA0);
      display.setCursor(38, 128);
      display.print("Did you catch the imposter?");

      // [ REVEAL IMPOSTER 🔍 ] Button
      const int rvX = 30, rvY = 188, rvW = SCREEN_WIDTH - 60, rvH = 48;
      display.fillRoundRect(rvX, rvY, rvW, rvH, 10, 0x001F);
      display.drawRoundRect(rvX, rvY, rvW, rvH, 10, 0x07FF);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(rvX + 14, rvY + 16);
      display.print("REVEAL RESULT 🔍");

      if (justTapped && touchY >= rvY) {
        impState = 5;
        audio.playSound(SOUND_POWERUP);
      }
    } else if (impState == 5) {
      // ── STATE 5: REVEAL RESULT ────────────────────────────────
      display.setTextSize(2);
      display.setTextColor(0x07E0);
      display.setCursor(42, 20);
      display.print("MISSION OVER!");

      display.setTextSize(1);
      display.setTextColor(0xFDA0);
      display.setCursor(54, 52);
      display.print("THE IMPOSTER WAS:");

      display.setTextSize(3);
      display.setTextColor(0xFFE0);
      char pStr[24];
      snprintf(pStr, sizeof(pStr), "PLAYER %d", impImposterIdx + 1);
      int pw = strlen(pStr) * 18;
      display.setCursor((SCREEN_WIDTH - pw) / 2, 74);
      display.print(pStr);

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      display.setCursor(56, 114);
      display.print("SECRET WORD WAS:");

      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *wrd = impWords[impWordIdx];
      int ww = strlen(wrd) * 12;
      display.setCursor((SCREEN_WIDTH - ww) / 2, 134);
      display.print(wrd);

      // [ PLAY AGAIN ▶ ] Button
      const int paX = 32, paY = 192, paW = SCREEN_WIDTH - 64, paH = 44;
      display.fillRoundRect(paX, paY, paW, paH, 10, 0x0280);
      display.drawRoundRect(paX, paY, paW, paH, 10, 0x07E0);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(paX + 26, paY + 13);
      display.print("PLAY AGAIN ▶");

      if (justTapped && touchY >= paY) {
        impState = 0;
        audio.playSound(SOUND_STARTUP);
      }
    }
  }

  // ============================================================
  //  GAME 9: CHARADES (HEADS UP ACTING PARTY GAME)
  // ============================================================
  int chaState = 0; // 0=SETUP, 1=PLAYING, 2=SUMMARY
  int chaScore = 0;
  int chaPasses = 0;
  int chaWordIdx = 0;
  unsigned long chaRoundStartMs = 0;
  unsigned long chaFeedbackTimer = 0;
  int chaFeedbackType = 0; // 0=none, 1=correct, 2=pass
  bool chaLastTouchDown = false;

  void resetCharades() {
    chaState = 0;
    chaScore = 0;
    chaPasses = 0;
    chaWordIdx = random(0, 40);
    chaRoundStartMs = 0;
    chaFeedbackTimer = 0;
    chaFeedbackType = 0;
    chaLastTouchDown = false;
  }

  void updateAndDrawCharades(GFXcanvas16 &display, LunaAudio &audio) {
    const unsigned long now = millis();
    bool isDown = isTouchActive();
    int touchX = getTouchX();
    int touchY = getTouchY();
    bool justTapped = isDown && !chaLastTouchDown;
    chaLastTouchDown = isDown;

    static const char *const chaWords[40] = {
      "MONKEY", "GUITAR", "PIZZA", "SUPERMAN", "SWIMMING",
      "DENTIST", "POPCORN", "ASTRONAUT", "SURFING", "PENGUIN",
      "BABY", "BRUSH TEETH", "BALLERINA", "ICE CREAM", "ZOMBIE",
      "MICHAEL JACKSON", "AIRPLANE", "SNEEZING", "KANGAROO", "CHEF",
      "DRIVING A CAR", "FISHING", "SPIDER-MAN", "PUPPY", "DRUMMER",
      "WEIGHT LIFTER", "ROBOT", "SLEEP WALKER", "PHOTO GRAPHER", "MAGICIAN",
      "PIRATE", "SPAGHETTI", "SINGING", "ELEPHANT", "NURSE",
      "TENNIS", "COWBOY", "FIRE FIGHTER", "ROLLER COASTER", "LION"
    };

    static const char *const chaCategories[40] = {
      "ANIMALS", "OBJECTS", "FOOD", "CHARACTERS", "ACTIONS",
      "JOBS", "FOOD", "JOBS", "ACTIONS", "ANIMALS",
      "CHARACTERS", "ACTIONS", "JOBS", "FOOD", "CHARACTERS",
      "CELEBRITIES", "OBJECTS", "ACTIONS", "ANIMALS", "JOBS",
      "ACTIONS", "ACTIONS", "CHARACTERS", "ANIMALS", "JOBS",
      "ACTIONS", "CHARACTERS", "ACTIONS", "JOBS", "JOBS",
      "CHARACTERS", "FOOD", "ACTIONS", "ANIMALS", "JOBS",
      "ACTIONS", "CHARACTERS", "JOBS", "ACTIONS", "ANIMALS"
    };

    // 1. Base Layer: AI Generated 3D Broadway Theater Stage Wallpaper
    memcpy_P(display.getBuffer(), charades_bg, 240 * 280 * sizeof(uint16_t));

    if (chaState == 0) {
      // ── STATE 0: SETUP & INSTRUCTIONS ─────────────────────────
      if (justTapped && touchX <= 45 && touchY <= 45) {
        gamePlaying = false;
        gamesActive = false;
        currentScreen = SCREEN_MENU;
        appLauncherPage = 1;
        audio.playSound(SOUND_CHIRP);
        return;
      }

      // Decorative Header Badge in upper placard crest (Y=48..66)
      display.fillRoundRect(58, 48, 124, 18, 9, 0x6802); // Rich Theater Maroon
      display.drawRoundRect(58, 48, 124, 18, 9, 0xFDE0); // Gold Trim
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(84, 53);
      display.print("- CHARADES -");

      // Rule 1: Cleanly centered inside white placard (X=52..188)
      display.setTextColor(0x0842); // Deep Onyx/Navy for maximum contrast
      display.setCursor(63, 75);
      display.print("1. Hold on forehead");
      display.setCursor(78, 88);
      display.print("facing partner");

      // Rule 2
      display.setCursor(63, 106);
      display.print("2. Partner acts out");
      display.setCursor(66, 119);
      display.print("word WITHOUT sound");

      // Interactive button guides: color-coded matching capsules
      display.fillRoundRect(58, 136, 124, 16, 5, 0x0400); // Forest green
      display.drawRoundRect(58, 136, 124, 16, 5, 0x07E0);
      display.setTextColor(0xFFFF);
      display.setCursor(63, 140);
      display.print("RIGHT: CORRECT (+1)");

      display.fillRoundRect(58, 158, 124, 16, 5, 0x9200); // Warm amber
      display.drawRoundRect(58, 158, 124, 16, 5, 0xFD20);
      display.setTextColor(0xFFFF);
      display.setCursor(69, 162);
      display.print("LEFT: PASS (SKIP)");

      // Round info
      display.setTextColor(0x8002);
      display.setCursor(75, 184);
      display.print("ROUND TIME: 60s");

      // [ START ROUND > ] Button in background button dock (Y=233..258, X=56..184)
      const int sX = 56, sY = 233, sW = 128, sH = 25;
      display.fillRoundRect(sX, sY, sW, sH, 7, 0x04A4); // Rich Emerald
      display.drawRoundRect(sX, sY, sW, sH, 7, 0x2FE6); // Vivid Green border
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(sX + 25, sY + 9);
      display.print("START ROUND >");

      if (justTapped && touchY >= 215) {
        chaState = 1;
        chaScore = 0;
        chaPasses = 0;
        chaWordIdx = random(0, 40);
        chaRoundStartMs = now;
        chaFeedbackType = 0;
        audio.playSound(SOUND_STARTUP);
      }
    } else if (chaState == 1) {
      // ── STATE 1: ACTIVE 60s ROUND ─────────────────────────────
      int elapsedSec = (now - chaRoundStartMs) / 1000;
      int remSec = max(0, 60 - elapsedSec);

      if (remSec == 0) {
        chaState = 2; // Time up!
        audio.playSound(SOUND_ALERT_BEEP);
      }

      // Top Status inside marquee canopy (Y=10..28)
      // Left: Timer Badge
      uint16_t timeBorder = (remSec <= 10) ? 0xF800 : 0xFFE0;
      display.fillRoundRect(12, 10, 72, 18, 6, 0x2802);
      display.drawRoundRect(12, 10, 72, 18, 6, timeBorder);
      display.setTextSize(1);
      display.setTextColor((remSec <= 10) ? 0xF800 : 0xFFFF);
      char tmStr[16];
      snprintf(tmStr, sizeof(tmStr), "TIME %ds", remSec);
      int tmW = strlen(tmStr) * 6;
      display.setCursor(12 + (72 - tmW) / 2, 15);
      display.print(tmStr);

      // Right: Score Badge
      display.fillRoundRect(156, 10, 72, 18, 6, 0x2802);
      display.drawRoundRect(156, 10, 72, 18, 6, 0xFFE0);
      display.setTextColor(0xFFE0);
      char scStr[16];
      snprintf(scStr, sizeof(scStr), "SCORE %d", chaScore);
      int scW = strlen(scStr) * 6;
      display.setCursor(156 + (72 - scW) / 2, 15);
      display.print(scStr);

      // Category Pill in placard arch (Y=50..68, X=60..180)
      display.fillRoundRect(60, 50, 120, 18, 8, 0x6004); // Theatrical Crimson
      display.drawRoundRect(60, 50, 120, 18, 8, 0xFDE0); // Gold border
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      const char *cat = chaCategories[chaWordIdx];
      int cw = strlen(cat) * 6;
      display.setCursor((SCREEN_WIDTH - cw) / 2, 55);
      display.print(cat);

      // Word Display inside allotted card space (X=52..188, Y=70..220)
      const char *wrd = chaWords[chaWordIdx];
      int wLen = strlen(wrd);
      bool hasSpace = (strchr(wrd, ' ') != NULL);

      if (hasSpace || wLen >= 12) {
        // Multi-line wrap in Size 2, line height 22
        drawCenteredWrappedText(display, wrd, 52, 114, 136, 22, 2, 0x0842);
      } else if (wLen <= 7) {
        // Huge single word in Size 3
        int ww = wLen * 18;
        display.setTextSize(3);
        display.setTextColor(0x0842);
        display.setCursor((SCREEN_WIDTH - ww) / 2, 120);
        display.print(wrd);
      } else {
        // Medium single word in Size 2
        int ww = wLen * 12;
        display.setTextSize(2);
        display.setTextColor(0x0842);
        display.setCursor((SCREEN_WIDTH - ww) / 2, 124);
        display.print(wrd);
      }

      // Visual Feedback Flash Overlay (Y=182..204, below word, above buttons)
      if (now - chaFeedbackTimer < 350) {
        if (chaFeedbackType == 1) {
          display.fillRoundRect(56, 182, 128, 22, 6, 0x07E0);
          display.drawRoundRect(56, 182, 128, 22, 6, 0x0400);
          display.setTextSize(2);
          display.setTextColor(0x0000);
          display.setCursor(68, 185);
          display.print("+1 POINT!");
        } else if (chaFeedbackType == 2) {
          display.fillRoundRect(56, 182, 128, 22, 6, 0xFD20);
          display.drawRoundRect(56, 182, 128, 22, 6, 0x9200);
          display.setTextSize(2);
          display.setTextColor(0x0000);
          display.setCursor(78, 185);
          display.print("PASSED >>");
        }
      }

      // ── TOUCH CONTROLS (Green Right = Correct, Orange Left = Pass) ──
      // Aligned with the two 3D button capsules on the porcelain placard
      const int btnY = 233, btnH = 25;

      // PASS Button (Left: X=58..118)
      const int pX = 58, pW = 60;
      display.fillRoundRect(pX, btnY, pW, btnH, 7, 0xD100); // Warm Coral / Amber fill
      display.drawRoundRect(pX, btnY, pW, btnH, 7, 0xFCA0); // Bright border
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(pX + 12, btnY + 9);
      display.print("PASS >");

      // CORRECT Button (Right: X=122..182)
      const int cX = 122, cW = 60;
      display.fillRoundRect(cX, btnY, cW, btnH, 7, 0x0480); // Emerald Green fill
      display.drawRoundRect(cX, btnY, cW, btnH, 7, 0x2FE6); // Bright Green border
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(cX + 9, btnY + 9);
      display.print("CORRECT");

      // Handle Taps across the lower interactive zone
      if (justTapped && touchY >= 215) {
        if (touchX < 120) {
          // PASS
          chaPasses++;
          chaWordIdx = (chaWordIdx + 1 + random(1, 4)) % 40;
          chaFeedbackType = 2;
          chaFeedbackTimer = now;
          audio.playSound(SOUND_CHIRP);
        } else {
          // CORRECT
          chaScore++;
          chaWordIdx = (chaWordIdx + 1 + random(1, 4)) % 40;
          chaFeedbackType = 1;
          chaFeedbackTimer = now;
          audio.playSound(SOUND_COIN);
        }
      }
    } else if (chaState == 2) {
      // ── STATE 2: ROUND OVER & FINAL SCORE ─────────────────────
      // Header Badge in upper placard crest (Y=48..66)
      display.fillRoundRect(58, 48, 124, 18, 9, 0x6802);
      display.drawRoundRect(58, 48, 124, 18, 9, 0xFDE0);
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(90, 53);
      display.print("TIME'S UP!");

      // "FINAL SCORE" Subtitle
      display.setTextColor(0x3186); // Slate/Charcoal
      display.setCursor(87, 78);
      display.print("FINAL SCORE");

      // Huge Score Value
      display.setTextSize(3);
      display.setTextColor(0x0480); // Emerald Green
      char scBuf[16];
      snprintf(scBuf, sizeof(scBuf), "%d", chaScore);
      int scLen = strlen(scBuf) * 18;
      display.setCursor((SCREEN_WIDTH - scLen) / 2, 94);
      display.print(scBuf);

      // Passed count
      display.setTextSize(1);
      display.setTextColor(0x6328);
      char pBuf[32];
      snprintf(pBuf, sizeof(pBuf), "Passed: %d words", chaPasses);
      int pbw = strlen(pBuf) * 6;
      display.setCursor((SCREEN_WIDTH - pbw) / 2, 128);
      display.print(pBuf);

      // Rank title pill
      const char *rank = (chaScore >= 8) ? "BROADWAY STAR!" : ((chaScore >= 5) ? "MASTER ACTOR!" : "GOOD EFFORT!");
      uint16_t rkFill = (chaScore >= 8) ? 0x0380 : ((chaScore >= 5) ? 0x2104 : 0x5102);
      uint16_t rkBorder = (chaScore >= 8) ? 0x2FE6 : ((chaScore >= 5) ? 0xFDA0 : 0x9B46);
      display.fillRoundRect(56, 146, 128, 20, 6, rkFill);
      display.drawRoundRect(56, 146, 128, 20, 6, rkBorder);
      display.setTextColor(0xFFE0); // Gold text
      int rw = strlen(rank) * 6;
      display.setCursor((SCREEN_WIDTH - rw) / 2, 152);
      display.print(rank);

      // Replay hint
      display.setTextColor(0x3186);
      display.setCursor(63, 178);
      display.print("Tap below to replay");

      // [ PLAY AGAIN > ] Button in background button dock (Y=233..258, X=56..184)
      const int paX = 56, paY = 233, paW = 128, paH = 25;
      display.fillRoundRect(paX, paY, paW, paH, 7, 0x04A4);
      display.drawRoundRect(paX, paY, paW, paH, 7, 0x2FE6);
      display.setTextSize(1);
      display.setTextColor(0xFFFF);
      display.setCursor(paX + 28, paY + 9);
      display.print("PLAY AGAIN >");

      if (justTapped && touchY >= 215) {
        chaState = 0;
        audio.playSound(SOUND_STARTUP);
      }
    }
  }
};

#endif // GAMES_H
