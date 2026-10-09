#ifndef GAMES_H
#define GAMES_H

#include "audio.h"
#include "config.h"
#include "imu.h"
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
    HS_STATE_SETUP = 0,
    HS_STATE_HIDING,
    HS_STATE_ACTIVE,
    HS_STATE_EXPLODED,
    HS_STATE_DEFUSED
  };
  HideSeekState hsState;
  int hsTimerSeconds;
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
    hsState = HS_STATE_SETUP;
    if (hsTimerSeconds < 15 || hsTimerSeconds > 300) hsTimerSeconds = 60;
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

    // ──────────────────────────────────────────────────────────
    // STATE 1: SETUP (TIMER SELECTION)
    // ──────────────────────────────────────────────────────────
    if (hsState == HS_STATE_SETUP) {
      gameOverActive = false;
      hsGameOver = false;

      // Handle Preset Buttons: [30s], [60s], [90s], [2m]
      if (justTapped && touchY >= 142 && touchY <= 174) {
        if (touchX >= 12 && touchX <= 64) {
          hsTimerSeconds = 30;
          audio.playSound(SOUND_COIN);
        } else if (touchX >= 66 && touchX <= 118) {
          hsTimerSeconds = 60;
          audio.playSound(SOUND_COIN);
        } else if (touchX >= 120 && touchX <= 172) {
          hsTimerSeconds = 90;
          audio.playSound(SOUND_COIN);
        } else if (touchX >= 174 && touchX <= 226) {
          hsTimerSeconds = 120;
          audio.playSound(SOUND_COIN);
        }
      }

      // Handle Stepper Buttons: [- 15s] and [+ 15s]
      if (justTapped && touchY >= 176 && touchY <= 214) {
        if (touchX >= 20 && touchX <= 60) {
          hsTimerSeconds = max(15, hsTimerSeconds - 15);
          audio.playSound(SOUND_JUMP);
        } else if (touchX >= 180 && touchX <= 220) {
          hsTimerSeconds = min(300, hsTimerSeconds + 15);
          audio.playSound(SOUND_JUMP);
        }
      }

      // Handle Dial Touch / Drag Scrubber (CX=120, CY=95, R=42)
      if (justTapped && touchY >= 50 && touchY <= 140 && touchX >= 75 && touchX <= 165) {
        float dx = touchX - 120;
        float dy = touchY - 95;
        float ang = atan2f(dy, dx) + 1.5708f; // relative to top (12 o'clock)
        if (ang < 0.0f) ang += 6.28318f;
        int dialed = (int)((ang / 6.28318f) * 120.0f);
        dialed = ((dialed + 7) / 15) * 15;
        if (dialed < 15) dialed = 15;
        if (dialed > 120) dialed = 120;
        hsTimerSeconds = dialed;
        audio.playSound(SOUND_COIN);
      }

      // Handle "ARM & HIDE" Bottom Card
      if (justTapped && touchY >= 216 && touchY <= 276) {
        hsState = HS_STATE_HIDING;
        hsPhaseStartTime = now;
        hsHidingDurationMs = 10000; // 10-second hiding countdown
        audio.playSound(SOUND_POWERUP);
      }

      // ──────────────────────────────────────────────────────────
      // RENDER EXACT LUNA HIDE & SEEK UI (LAVENDER / PURPLE)
      // ──────────────────────────────────────────────────────────
      // Deep magical violet/purple background
      display.fillScreen(0x18C8);

      // Star sparkles in background
      display.drawPixel(22, 34, 0xFFE0);
      display.drawPixel(214, 28, 0xFFE0);
      display.drawPixel(45, 120, 0xD65F);
      display.drawPixel(198, 126, 0xD65F);

      // ── TOP HEADER: HIDE & SEEK LOGO WITH CAT EARS ──
      // Cat ears over "H" & "I"
      display.fillTriangle(68, 20, 64, 13, 73, 16, 0x4170);
      display.fillTriangle(67, 19, 65, 15, 71, 17, 0xFBAE); // Pink inner ear
      display.fillTriangle(79, 16, 85, 13, 83, 20, 0x4170);
      display.fillTriangle(80, 17, 84, 15, 82, 19, 0xFBAE); // Pink inner ear

      // Excitement comic rays around logo
      display.drawLine(58, 14, 54, 11, 0xFFE0);
      display.drawLine(182, 14, 186, 11, 0xFFE0);

      // Logo container pill
      display.fillRoundRect(56, 17, 128, 22, 11, 0x28AE);
      display.drawRoundRect(56, 17, 128, 22, 11, 0x51D5);

      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(62, 21);
      display.print("HIDE");
      display.setTextColor(0xD65F);
      display.setCursor(110, 21);
      display.print("&");
      display.setTextColor(0xFFE0);
      display.setCursor(124, 21);
      display.print("SEEK");

      // ── CHARACTERS ──
      // Left White Kitten Peeking behind drape
      display.fillRoundRect(0, 48, 12, 54, 5, 0x4994);
      display.fillCircle(18, 76, 10, 0xFFFF);
      display.fillTriangle(14, 69, 12, 62, 18, 67, 0xFBAE);
      display.fillCircle(14, 80, 3, 0xFBAE); // Pink cheek
      display.drawPixel(17, 75, 0x18C6);     // Eye
      display.drawPixel(18, 76, 0x18C6);
      display.fillCircle(24, 82, 3, 0xFFFF); // Paw
      display.drawCircle(24, 82, 3, 0xD65F);
      display.drawLine(16, 56, 14, 50, 0xFFE0); // Whiskers/rays
      display.drawLine(22, 58, 24, 52, 0xFFE0);

      // Right Black Kitten Peeking behind chair
      display.fillRoundRect(228, 64, 12, 44, 5, 0x4994);
      display.fillCircle(222, 76, 10, 0x18A4);
      display.fillTriangle(226, 69, 228, 62, 222, 67, 0x18A4);
      display.fillCircle(218, 75, 3, 0xFFFF); // Shiny eye
      display.fillCircle(218, 75, 1, 0x18A4);
      display.drawCircle(212, 82, 5, 0x07FF); // Magnifying glass
      display.drawLine(216, 86, 220, 90, 0xCE79);
      display.drawLine(224, 56, 226, 50, 0xFFE0);
      display.drawLine(218, 58, 216, 52, 0xFFE0);

      // ── CENTER CIRCULAR TIMER DIAL (CX=120, CY=95, R=42) ──
      // Glowing purple outer ring
      display.drawCircle(120, 95, 43, 0x5194);
      display.drawCircle(120, 95, 42, 0x915C);
      display.drawCircle(120, 95, 41, 0xC1BF);

      // Dreamy lavender inner face
      display.fillCircle(120, 95, 39, 0xF7BE);

      // Clouds at bottom of dial
      display.fillRoundRect(88, 110, 64, 20, 8, 0xC55E);
      display.fillRoundRect(96, 116, 48, 14, 6, 0x915C);

      // Sparkles in dial
      display.drawPixel(102, 78, 0x915C);
      display.drawPixel(138, 80, 0xC1BF);

      // Digital time readout: MM:SS
      int m = hsTimerSeconds / 60;
      int s = hsTimerSeconds % 60;

      // Minutes in dark indigo
      display.setTextSize(3);
      display.setTextColor(0x18C6);
      display.setCursor(78, 85);
      display.printf("%02d", m);

      // Colon in vibrant purple
      display.setTextColor(0x915C);
      display.setCursor(114, 85);
      display.print(":");

      // Seconds in vibrant magenta
      display.setTextColor(0xC1BF);
      display.setCursor(128, 85);
      display.printf("%02d", s);

      // Draggable scrubber knob on circular track
      float knobAngle = ((float)hsTimerSeconds / 60.0f) * 6.28318f - 1.5708f;
      int kx = 120 + (int)(cosf(knobAngle) * 42.0f);
      int ky = 95 + (int)(sinf(knobAngle) * 42.0f);
      display.fillCircle(kx, ky, 6, 0xFFFF);
      display.drawCircle(kx, ky, 6, 0xC1BF);
      display.drawPixel(kx, ky, 0x915C);

      // ── QUICK PRESET PILLS: 30s | 60s | 90s | 2m (Y=146..170) ──
      const int presets[] = {30, 60, 90, 120};
      const char *pLabels[] = {"30s", "60s", "90s", "2m"};
      for (int i = 0; i < 4; i++) {
        int px = 14 + i * 54;
        bool isSel = (hsTimerSeconds == presets[i]);
        if (isSel) {
          display.fillRoundRect(px, 146, 48, 24, 12, 0x915C);
          display.drawRoundRect(px, 146, 48, 24, 12, 0xD65F);
          display.setTextSize(1);
          display.setTextColor(0xFFFF);
        } else {
          display.fillRoundRect(px, 146, 48, 24, 12, 0xEF7D);
          display.drawRoundRect(px, 146, 48, 24, 12, 0xC57C);
          display.setTextSize(1);
          display.setTextColor(0x28AE);
        }
        int plW = strlen(pLabels[i]) * 6;
        display.setCursor(px + (48 - plW) / 2, 154);
        display.print(pLabels[i]);
      }

      // ── FINE ADJUSTER CAPSULE: [-] ··· clock ··· [+] (Y=178..210) ──
      display.fillRoundRect(22, 178, 196, 32, 16, 0x3914);
      display.drawRoundRect(22, 178, 196, 32, 16, 0x6A38);

      // Minus Button (Left)
      display.fillCircle(40, 194, 12, 0xEF7D);
      display.drawCircle(40, 194, 12, 0xC57C);
      display.setTextSize(2);
      display.setTextColor(0x28AE);
      display.setCursor(35, 187);
      display.print("-");

      // Center Clock Arc of Dots
      for (int d = 0; d < 7; d++) {
        int dotX = 85 + d * 10;
        int dotY = 186 + abs(d - 3);
        display.drawPixel(dotX, dotY, 0xFFFF);
      }
      display.drawCircle(120, 197, 6, 0xD65F);
      display.drawLine(120, 197, 120, 193, 0xFFFF);
      display.drawLine(120, 197, 123, 197, 0xFFFF);

      // Plus Button (Right)
      display.fillCircle(200, 194, 12, 0x915C);
      display.drawCircle(200, 194, 12, 0xD65F);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      display.setCursor(194, 187);
      display.print("+");

      // ── BOTTOM WAVY CARD: "ARM & HIDE" (Y=218..274) ──
      display.fillRoundRect(14, 218, SCREEN_WIDTH - 28, 54, 16, 0x28AE);
      display.drawRoundRect(14, 218, SCREEN_WIDTH - 28, 54, 16, 0x6A38);
      display.drawFastHLine(24, 220, SCREEN_WIDTH - 48, 0x915C);

      // Animated upward chevron ^
      int chevY = 224 + ((now / 220) % 3);
      display.drawLine(120, chevY, 115, chevY + 4, 0xD65F);
      display.drawLine(120, chevY, 125, chevY + 4, 0xD65F);

      // ARM & HIDE text
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *armTxt = "ARM & HIDE";
      int atW = strlen(armTxt) * 12;
      display.setCursor((SCREEN_WIDTH - atW) / 2, 233);
      display.print(armTxt);

      // Bottom illuminated pill handle
      display.fillRoundRect(95, 258, 50, 4, 2, 0xFFFF);
      display.drawRoundRect(93, 257, 54, 6, 3, 0xD65F);
    }

    // ──────────────────────────────────────────────────────────
    // STATE 2: HIDING (10-SECOND STEALTH PHASE)
    // ──────────────────────────────────────────────────────────
    else if (hsState == HS_STATE_HIDING) {
      long hideElapsed = (long)(now - hsPhaseStartTime);
      long hideLeft = (long)hsHidingDurationMs - hideElapsed;

      // Tap to skip hiding phase and start ticking right away
      if ((justTapped && touchY >= 210) || hideLeft <= 0) {
        hsState = HS_STATE_ACTIVE;
        hsPhaseStartTime = now;
        hsBombDurationMs = hsTimerSeconds * 1000UL;
        hsLastTickMs = 0;
        hsSoundWaveRadius = 0;
        audio.playSound(SOUND_ALERT_BEEP);
        return;
      }

      display.fillScreen(0x0000); // Pitch stealth black

      // Top Hazard Stripes Banner
      for (int x = 0; x < SCREEN_WIDTH; x += 20) {
        display.fillTriangle(x, 22, x + 10, 22, x, 34, 0xFD20);
        display.fillTriangle(x + 10, 22, x + 20, 22, x + 10, 34, 0x0000);
      }

      // Status Title
      display.setTextSize(2);
      display.setTextColor(0xFFE0);
      const char *hdTitle = "GO HIDE WATCH!";
      int hw = strlen(hdTitle) * 12;
      display.setCursor((SCREEN_WIDTH - hw) / 2, 44);
      display.print(hdTitle);

      // Running figure animation (Y=72..102)
      int runnerX = 120 + (int)(sinf(now / 200.0f) * 16);
      int legFrame = (now / 150) % 2;
      // Head
      display.fillCircle(runnerX, 76, 6, 0x07FF);
      // Torso
      display.drawLine(runnerX, 82, runnerX, 94, 0x07FF);
      // Arms
      display.drawLine(runnerX, 85, runnerX - 8, 88, 0x07FF);
      display.drawLine(runnerX, 85, runnerX + 8, 83, 0x07FF);
      // Legs (running cycle)
      if (legFrame == 0) {
        display.drawLine(runnerX, 94, runnerX - 7, 103, 0x07FF);
        display.drawLine(runnerX, 94, runnerX + 7, 98, 0x07FF);
        display.drawLine(runnerX + 7, 98, runnerX + 10, 103, 0x07FF);
      } else {
        display.drawLine(runnerX, 94, runnerX + 7, 103, 0x07FF);
        display.drawLine(runnerX, 94, runnerX - 7, 98, 0x07FF);
        display.drawLine(runnerX - 7, 98, runnerX - 10, 103, 0x07FF);
      }

      // Giant Glowing Countdown Number (Y=114..160)
      int secLeft = (int)(hideLeft / 1000) + 1;
      char cntBuf[8];
      snprintf(cntBuf, sizeof(cntBuf), "%d", secLeft);
      display.setTextSize(6);
      uint16_t numColor = ((now / 250) % 2 == 0) ? 0xFFFF : 0xFFE0;
      display.setTextColor(numColor);
      int nw = strlen(cntBuf) * 36;
      display.setCursor((SCREEN_WIDTH - nw) / 2, 114);
      display.print(cntBuf);

      // Instructive Text
      display.setTextSize(1);
      display.setTextColor(0xCE79);
      const char *hSub1 = "TICKING STARTS AUTOMATICALLY";
      int hs1W = strlen(hSub1) * 6;
      display.setCursor((SCREEN_WIDTH - hs1W) / 2, 172);
      display.print(hSub1);

      const char *hSub2 = "SEEKER MUST LISTEN CAREFULLY";
      int hs2W = strlen(hSub2) * 6;
      display.setCursor((SCREEN_WIDTH - hs2W) / 2, 188);
      display.print(hSub2);

      // Skip Button: [ READY! START NOW > ]
      const int rX = 24, rY = 220, rW = SCREEN_WIDTH - 48, rH = 38;
      display.fillRoundRect(rX, rY, rW, rH, 8, 0x001F);
      display.drawRoundRect(rX, rY, rW, rH, 8, 0x07FF);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *rTxt = "START NOW >";
      int rtW = strlen(rTxt) * 12;
      display.setCursor(rX + (rW - rtW) / 2, rY + 11);
      display.print(rTxt);
    }

    // ──────────────────────────────────────────────────────────
    // STATE 3: ACTIVE (TIME BOMB TICKING & SEEKING PHASE)
    // ──────────────────────────────────────────────────────────
    else if (hsState == HS_STATE_ACTIVE) {
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

        // Spawn 24 explosion particles
        for (int i = 0; i < 24; i++) {
          hsExplosionPartX[i] = 120.0f;
          hsExplosionPartY[i] = 100.0f;
          float angle = (i * 15.0f) * 3.14159f / 180.0f;
          float spd = 2.0f + (random(10, 40) / 10.0f);
          hsExplosionPartVX[i] = cosf(angle) * spd;
          hsExplosionPartVY[i] = sinf(angle) * spd;
          hsExplosionPartCol[i] = (i % 3 == 0) ? 0xFFFF : ((i % 3 == 1) ? 0xFFE0 : 0xF800);
        }
        return;
      }

      // ── SEEKER DEFUSED THE BOMB! ──
      // Tapping the screen or defuse button during active seeking disarms it!
      if (justTapped) {
        hsState = HS_STATE_DEFUSED;
        hsDefusedRemainingMs = remainingMs;
        hsGameOver = true;
        gameOverActive = true;
        audio.stopBuzzer();
        audio.playSound(SOUND_DEFUSED);
        saveHighScore("hs", hsHighScore, (int)(remainingMs / 1000));
        return;
      }

      // Calculate progress ratio (0.0f to 1.0f)
      float ratio = (float)activeElapsed / (float)hsBombDurationMs;
      ratio = constrain(ratio, 0.0f, 1.0f);

      // ── BUZZER TICKING SYSTEM (GRADUALLY INCREASING VOLUME & PITCH) ──
      // Interval: starts at 2600ms, decreases to 140ms
      int interval;
      if (remainingMs <= 5000) {
        interval = 140; // Final 5 seconds: frantic urgent pulsing!
      } else if (remainingMs <= 12000) {
        interval = 280; // Final 12 seconds: fast warning ticks
      } else {
        interval = (int)(2600.0f - (ratio * 2100.0f));
        if (interval < 350) interval = 350;
      }

      // Volume percent: 4% (faint whisper tick) -> 100% (max square wave power!)
      int vol = (int)(4.0f + ratio * 96.0f);
      vol = constrain(vol, 4, 100);

      // Frequency: starts at 650Hz (deep subtle click), rises to 3100Hz (sharp piercing alarm)
      int freq = (int)(650.0f + (ratio * ratio) * 2450.0f);
      if (remainingMs <= 5000) freq = 3100;

      // Beep duration: 18ms up to 75ms
      int dur = (int)(18.0f + ratio * 55.0f);

      // Play tick beep if interval elapsed
      if (now - hsLastTickMs >= (unsigned long)interval) {
        hsLastTickMs = now;
        hsSoundWaveRadius = 10; // Trigger visual sound wave ring
        audio.playVolumeBeep(freq, dur, vol);
      }

      // ── RENDER ACTIVE BOMB DISPLAY ──
      // Shake effect during last 5 seconds
      int shakeX = (remainingMs <= 5000) ? random(-2, 3) : 0;
      int shakeY = (remainingMs <= 5000) ? random(-2, 3) : 0;

      // Background: dark tactical grid (or flashing red in final 5 seconds)
      if (remainingMs <= 5000 && ((now / 140) % 2 == 0)) {
        display.fillScreen(0x4000); // Alert red flash
      } else {
        display.fillScreen(0x0841); // Dark slate
      }

      // Top Status Text
      display.setTextSize(1);
      const char *stTxt;
      uint16_t stCol;
      if (ratio < 0.30f) {
        stTxt = "STEALTH // FAINT TICKS";
        stCol = 0x07E0; // Green
      } else if (ratio < 0.70f) {
        stTxt = "SEEKING // AUDIBLE BEEPS";
        stCol = 0xFFE0; // Yellow
      } else if (ratio < 0.90f) {
        stTxt = "DANGER // HIGH VOLUME";
        stCol = 0xFD20; // Orange
      } else {
        stTxt = "CRITICAL // DETONATION IMMINENT!";
        stCol = 0xF800; // Red
      }
      display.setTextColor(stCol);
      int stW = strlen(stTxt) * 6;
      display.setCursor((SCREEN_WIDTH - stW) / 2, 26);
      display.print(stTxt);
      display.drawFastHLine(20, 36, SCREEN_WIDTH - 40, 0x4208);

      // Bomb Graphic (Center X=120, Y=86)
      const int bx = 120 + shakeX;
      const int by = 86 + shakeY;

      // Concentric Sound Wave Ripple
      if (hsSoundWaveRadius > 0) {
        uint16_t rippleCol = (ratio >= 0.75f) ? 0xF800 : 0x07FF;
        display.drawCircle(bx, by, hsSoundWaveRadius, rippleCol);
        display.drawCircle(bx, by, hsSoundWaveRadius + 3, blend565(rippleCol, 0x0000));
        hsSoundWaveRadius += 4;
        if (hsSoundWaveRadius > 55) hsSoundWaveRadius = 0;
      }

      // Bomb Sphere Body
      display.fillCircle(bx, by, 22, 0x2104);
      display.drawCircle(bx, by, 22, (ratio >= 0.75f) ? 0xF800 : 0xCE79);
      // Specular highlight
      display.fillCircle(bx - 7, by - 7, 4, 0x7BEF);
      display.fillCircle(bx - 8, by - 8, 2, 0xFFFF);

      // Bomb Metallic Collar
      display.fillRect(bx - 5, by - 26, 10, 5, 0x632C);

      // Curved Rope Fuse
      display.drawLine(bx, by - 26, bx + 8, by - 34, 0xBDF7);
      display.drawLine(bx + 8, by - 34, bx + 16, by - 30, 0xBDF7);

      // Lit Spark at Fuse Tip
      int fx = bx + 17, fy = by - 29;
      display.fillCircle(fx, fy, 3, 0xFFE0);
      int spk = (now / 60) % 4;
      if (spk == 0) {
        display.drawPixel(fx + 3, fy - 4, 0xFD20);
        display.drawPixel(fx + 5, fy + 1, 0xF800);
      } else if (spk == 1) {
        display.drawPixel(fx - 3, fy - 4, 0xFFFF);
        display.drawPixel(fx + 4, fy - 3, 0xFD20);
      } else if (spk == 2) {
        display.drawPixel(fx + 2, fy - 5, 0xFFE0);
        display.drawPixel(fx + 6, fy - 1, 0xFFFF);
      }

      // Skull or alert badge in bomb center
      if (remainingMs <= 5000 && ((now / 150) % 2 == 0)) {
        display.fillCircle(bx, by, 8, 0xF800);
        display.fillCircle(bx - 3, by - 2, 2, 0x0000);
        display.fillCircle(bx + 3, by - 2, 2, 0x0000);
      } else {
        display.drawCircle(bx, by, 8, 0x07FF);
      }

      // Digital Countdown Timer (Y=124)
      int totalSecLeft = (int)(remainingMs / 1000);
      int tm = totalSecLeft / 60;
      int ts = totalSecLeft % 60;
      int tenths = (int)((remainingMs % 1000) / 100);
      char timeBuf[16];
      snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d.%d", tm, ts, tenths);

      display.setTextSize(3);
      uint16_t tmColor = (remainingMs <= 5000) ? 0xF800 : ((ratio >= 0.70f) ? 0xFD20 : 0xFFFF);
      display.setTextColor(tmColor);
      int timW = strlen(timeBuf) * 18;
      display.setCursor((SCREEN_WIDTH - timW) / 2, 124);
      display.print(timeBuf);

      // Volume & Danger Progress Bar (Y=162..174)
      display.fillRoundRect(20, 162, SCREEN_WIDTH - 40, 12, 4, 0x1082);
      display.drawRoundRect(20, 162, SCREEN_WIDTH - 40, 12, 4, 0x4208);
      int fillW = (int)((SCREEN_WIDTH - 44) * ratio);
      if (fillW > 0) {
        uint16_t bCol = (ratio < 0.4f) ? 0x07E0 : ((ratio < 0.75f) ? 0xFFE0 : 0xF800);
        display.fillRoundRect(22, 164, fillW, 8, 2, bCol);
      }

      // Volume Level Readout (Y=182)
      display.setTextSize(1);
      display.setTextColor(0xBDF7);
      char vBuf[32];
      snprintf(vBuf, sizeof(vBuf), "BUZZER: %d%% LOUD // PITCH: %dHz", vol, freq);
      int vbW = strlen(vBuf) * 6;
      display.setCursor((SCREEN_WIDTH - vbW) / 2, 182);
      display.print(vBuf);

      // [ DEFUSE BOMB ] Big Tactile Button (Y=206..256)
      const int dfX = 16, dfY = 206, dfW = SCREEN_WIDTH - 32, dfH = 48;
      uint16_t dfBorder = ((now / 200) % 2 == 0) ? 0xFFFF : 0x07E0;
      display.fillRoundRect(dfX, dfY, dfW, dfH, 10, 0x05E0);
      display.drawRoundRect(dfX, dfY, dfW, dfH, 10, dfBorder);
      display.drawFastHLine(dfX + 8, dfY + 4, dfW - 16, 0x07E0);

      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *dfTxt = "DEFUSE BOMB!";
      int dfW2 = strlen(dfTxt) * 12;
      display.setCursor(dfX + (dfW - dfW2) / 2, dfY + 16);
      display.print(dfTxt);

      display.setTextSize(1);
      display.setTextColor(0x632C);
      const char *fndHint = "TAP WHEN FOUND TO DISARM";
      int fhW = strlen(fndHint) * 6;
      display.setCursor((SCREEN_WIDTH - fhW) / 2, 264);
      display.print(fndHint);
    }

    // ──────────────────────────────────────────────────────────
    // STATE 4: EXPLODED (DETONATION! PLAYER IS OUT!)
    // ──────────────────────────────────────────────────────────
    else if (hsState == HS_STATE_EXPLODED) {
      gameOverActive = true;
      hsGameOver = true;

      // Tap to replay
      if (justTapped || justBtn1) {
        resetHideSeek();
        audio.playSound(SOUND_POWERUP);
        return;
      }

      // Screen shake in initial frames
      int sx = (hsExplosionFrame < 15) ? random(-4, 5) : 0;
      int sy = (hsExplosionFrame < 15) ? random(-4, 5) : 0;
      hsExplosionFrame++;

      display.fillScreen(0x1082); // Charcoal smoke background

      // Animate shockwaves and explosion particles
      for (int i = 0; i < 24; i++) {
        hsExplosionPartX[i] += hsExplosionPartVX[i];
        hsExplosionPartY[i] += hsExplosionPartVY[i];
        display.fillCircle((int)hsExplosionPartX[i] + sx, (int)hsExplosionPartY[i] + sy,
                           random(2, 4), hsExplosionPartCol[i]);
      }

      // Expanding shockwave circles
      if (hsExplosionFrame < 25) {
        display.drawCircle(120 + sx, 100 + sy, hsExplosionFrame * 4, 0xF800);
        display.drawCircle(120 + sx, 100 + sy, hsExplosionFrame * 3, 0xFFE0);
      }

      // Giant Comic-Style "BOOM!" Banner
      display.setTextSize(3);
      display.setTextColor(0x0000); // Drop shadow
      display.setCursor(62 + sx, 34 + sy);
      display.print("BOOM!");
      display.setTextColor(0xFFE0); // Bright yellow
      display.setCursor(60 + sx, 32 + sy);
      display.print("BOOM!");

      // "BOMB DETONATED!"
      display.setTextSize(2);
      display.setTextColor(0xF800);
      const char *expSub = "DETONATED!";
      int ew = strlen(expSub) * 12;
      display.setCursor((SCREEN_WIDTH - ew) / 2 + sx, 64 + sy);
      display.print(expSub);

      // Outcome Box (Y=96..178)
      display.fillRoundRect(16, 96, SCREEN_WIDTH - 32, 82, 8, 0x0841);
      display.drawRoundRect(16, 96, SCREEN_WIDTH - 32, 82, 8, 0xF800);

      display.setTextSize(2);
      display.setTextColor(0xFFE0);
      const char *out1 = "SEEKER OUT!";
      int o1w = strlen(out1) * 12;
      display.setCursor((SCREEN_WIDTH - o1w) / 2, 106);
      display.print(out1);

      display.setTextColor(0x07FF);
      const char *out2 = "HIDER WINS!";
      int o2w = strlen(out2) * 12;
      display.setCursor((SCREEN_WIDTH - o2w) / 2, 128);
      display.print(out2);

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      char durBuf[36];
      snprintf(durBuf, sizeof(durBuf), "HIDDEN FOR %d SECONDS", hsTimerSeconds);
      int dw = strlen(durBuf) * 6;
      display.setCursor((SCREEN_WIDTH - dw) / 2, 158);
      display.print(durBuf);

      // [ PLAY AGAIN > ] Button (Y=192..232)
      const int paX = 24, paY = 192, paW = SCREEN_WIDTH - 48, paH = 38;
      display.fillRoundRect(paX, paY, paW, paH, 8, 0x05E0);
      display.drawRoundRect(paX, paY, paW, paH, 8, 0x07E0);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *paTxt = "PLAY AGAIN >";
      int patW = strlen(paTxt) * 12;
      display.setCursor(paX + (paW - patW) / 2, paY + 11);
      display.print(paTxt);

      display.setTextSize(1);
      display.setTextColor(0x8410);
      const char *exTxt = "SWIPE TO EXIT ARCADE";
      int extW = strlen(exTxt) * 6;
      display.setCursor((SCREEN_WIDTH - extW) / 2, 248);
      display.print(exTxt);
    }

    // ──────────────────────────────────────────────────────────
    // STATE 5: DEFUSED (SEEKER WINS! FOUND IN TIME!)
    // ──────────────────────────────────────────────────────────
    else if (hsState == HS_STATE_DEFUSED) {
      gameOverActive = true;
      hsGameOver = true;

      // Tap to replay
      if (justTapped || justBtn1) {
        resetHideSeek();
        audio.playSound(SOUND_POWERUP);
        return;
      }

      display.fillScreen(0x0020); // Matrix dark emerald background

      // Celebration sparkle confetti
      for (int i = 0; i < 14; i++) {
        int cx = (i * 18 + (now / 20)) % (SCREEN_WIDTH - 20) + 10;
        int cy = (i * 24 + (now / 15)) % 150 + 40;
        display.fillCircle(cx, cy, 2, (i % 2 == 0) ? 0x07E0 : 0x07FF);
      }

      // Banner
      display.setTextSize(3);
      display.setTextColor(0x07E0); // Bright emerald
      const char *dfTitle = "DEFUSED!";
      int dfw = strlen(dfTitle) * 18;
      display.setCursor((SCREEN_WIDTH - dfw) / 2, 34);
      display.print(dfTitle);

      display.setTextSize(2);
      display.setTextColor(0x07FF); // Cyan
      const char *dfSub = "SEEKER WINS!";
      int dsw = strlen(dfSub) * 12;
      display.setCursor((SCREEN_WIDTH - dsw) / 2, 64);
      display.print(dfSub);

      // Victory Stats Card (Y=96..178)
      display.fillRoundRect(16, 96, SCREEN_WIDTH - 32, 82, 8, 0x0841);
      display.drawRoundRect(16, 96, SCREEN_WIDTH - 32, 82, 8, 0x07E0);

      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *fndTxt = "FOUND IN TIME!";
      int fw = strlen(fndTxt) * 12;
      display.setCursor((SCREEN_WIDTH - fw) / 2, 106);
      display.print(fndTxt);

      display.setTextColor(0x07E0);
      int remS = (int)(hsDefusedRemainingMs / 1000);
      int remT = (int)((hsDefusedRemainingMs % 1000) / 100);
      char remStr[32];
      snprintf(remStr, sizeof(remStr), "%d.%ds LEFT!", remS, remT);
      int rsw = strlen(remStr) * 12;
      display.setCursor((SCREEN_WIDTH - rsw) / 2, 128);
      display.print(remStr);

      display.setTextSize(1);
      display.setTextColor(0xCE79);
      const char *vSub = "GREAT DETECTIVE WORK!";
      int vsw = strlen(vSub) * 6;
      display.setCursor((SCREEN_WIDTH - vsw) / 2, 158);
      display.print(vSub);

      // [ PLAY AGAIN > ] Button (Y=192..232)
      const int paX = 24, paY = 192, paW = SCREEN_WIDTH - 48, paH = 38;
      display.fillRoundRect(paX, paY, paW, paH, 8, 0x001F);
      display.drawRoundRect(paX, paY, paW, paH, 8, 0x07FF);
      display.setTextSize(2);
      display.setTextColor(0xFFFF);
      const char *paTxt = "PLAY AGAIN >";
      int patW = strlen(paTxt) * 12;
      display.setCursor(paX + (paW - patW) / 2, paY + 11);
      display.print(paTxt);

      display.setTextSize(1);
      display.setTextColor(0x8410);
      const char *exTxt = "SWIPE TO EXIT ARCADE";
      int extW = strlen(exTxt) * 6;
      display.setCursor((SCREEN_WIDTH - extW) / 2, 248);
      display.print(exTxt);
    }
  }
};

#endif // GAMES_H
