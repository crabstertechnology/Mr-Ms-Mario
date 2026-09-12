#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Preferences.h>
#include "config.h"
#include "audio.h"
#include "imu.h"

// External references (declared once)
extern bool gamesActive;
extern bool gamePlaying;
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
                    ((((a >> 5)  & 0x3F) + ((b >> 5)  & 0x3F)) >> 1) << 5  |
                    ((( a        & 0x1F) + ( b        & 0x1F)) >> 1));
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

  // ── Game 1: Luna Racer ─────────────────────────────────────────────────────
  float racPlayerX;
  float racPlayerY;
  float racSpeed;
  bool  racNitroActive;
  float racNitroFuel;
  float racRoadScroll;
  int   racScore;
  bool  racGameOver;
  float racCarX[4];
  float racCarY[4];
  float racCarSpeed[4];
  int   racCarColor[4];
  bool  racCarActive[4];
  // Nitro alternating-tap state
  unsigned long racNitroTapTime;
  int           racLastTapSide;   // 0=none, 1=left-pressed, 2=right-pressed
  // Flame animation frame
  uint8_t       racFlameFrame;

  // ── Game 2: Luna Space ─────────────────────────────────────────────────────
  float spcPlayerX;
  float spcPlayerY;
  int   spcScore;
  int   spcHealth;
  bool  spcGameOver;
  unsigned long spcLastShoot;
  unsigned long spcShieldTime;
  unsigned long spcSmartBombCooldown;
  float spcLaserX[6];
  float spcLaserY[6];
  bool  spcLaserActive[6];
  float spcEnemyX[8];
  float spcEnemyY[8];
  bool  spcEnemyActive[8];
  int   spcEnemyHP[8];
  int   spcEnemyType[8];
  unsigned long spcEnemyLastShoot[8];
  float spcEnemyLaserX[4];
  float spcEnemyLaserY[4];
  bool  spcEnemyLaserActive[4];
  float spcPartX[15];
  float spcPartY[15];
  float spcPartVX[15];
  float spcPartVY[15];
  uint8_t spcPartLife[15];       // uint8_t saves 30 bytes vs int
  bool  spcPartActive[15];
  float spcPowerX;
  float spcPowerY;
  int   spcPowerType;
  bool  spcPowerActive;
  bool  spcSpreadActive;
  bool  spcBossActive;
  int   spcBossHP;
  float spcBossX;
  float spcBossY;
  int   spcBossDir;
  unsigned long spcBossLastShoot;
  // Parallax star positions (deterministic, computed once)
  uint8_t spcStar1X[10];
  uint8_t spcStar1Y[10];
  uint8_t spcStar2X[6];
  uint8_t spcStar2Y[6];
  bool    spcStarsInit;

  // ── Game 3: Flappy Mochy ───────────────────────────────────────────────────
  float flapPlayerY;
  float flapPlayerVY;
  float flapPipeX;
  float flapPipeGapY;
  int   flapScore;
  bool  flapGameOver;
  float flapCloud1X;
  float flapCloud2X;

  // ── Game 4: Coin Catcher ───────────────────────────────────────────────────
  float catPlayerX;
  float catCoinsX[3];
  float catCoinsY[3];
  bool  catCoinsActive[3];
  bool  catIsBomb[3];
  int   catScore;
  int   catLives;
  bool  catGameOver;

  // ── Game 5: Mochy Jump ─────────────────────────────────────────────────────
  float jumpPlayerX;
  float jumpPlayerY;
  float jumpPlayerVY;
  float jumpPlatX[5];
  float jumpPlatY[5];
  int   jumpScore;
  bool  jumpGameOver;
  bool  jumpSquash;

  // ── Game 6: Stacker ────────────────────────────────────────────────────────
  float stkBlockX;
  float stkBlockW;
  float stkBlockY;
  float stkSpeed;
  int   stkDir;
  float stkBaseX;
  int   stkScore;
  bool  stkGameOver;
  int   stkStackHeight;
  float stkStackX[12];
  float stkStackW[12];
  bool  stkFlashNew;             // flash newly placed block

  // ── Game 7: Memory Matrix ──────────────────────────────────────────────────
  int   memSeq[16];
  int   memSeqLen;
  int   memSeqStep;
  int   memPlayerStep;
  int   memSelectedTile;
  int   memState;                // 0: showing seq, 1: player input
  unsigned long memTimer;
  int   memScore;
  bool  memGameOver;

  // ── Input edge-detect ──────────────────────────────────────────────────────
  bool lastBtn1;
  bool lastBtn2;

public:
  LunaGames() {
    racHighScore  = 0; spcHighScore  = 0; flapHighScore = 0;
    catHighScore  = 0; jumpHighScore = 0; stkHighScore  = 0;
    memHighScore  = 0;
    lastBtn1 = false; lastBtn2 = false;
    spcStarsInit = false;
  }

  // ── Screen geometry (240×280 portrait) ────────────────────────────────────
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

  // ── Persistent storage ─────────────────────────────────────────────────────
  void begin() {
    Preferences prefs;
    prefs.begin("luna_scores", true);
    racHighScore  = prefs.getInt("rac",  0);
    spcHighScore  = prefs.getInt("spc",  0);
    flapHighScore = prefs.getInt("flap", 0);
    catHighScore  = prefs.getInt("cat",  0);
    jumpHighScore = prefs.getInt("jump", 0);
    stkHighScore  = prefs.getInt("stk",  0);
    memHighScore  = prefs.getInt("mem",  0);
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

  // ── IMU tilt steering ──────────────────────────────────────────────────────
  float getTiltSteer() {
    if (!imu.isInitialized()) return 0.0f;
    float ax = 0, ay = 0, az = 1, gx = 0, gy = 0, gz = 0;
    if (!imu.readMotion(ax, ay, az, gx, gy, gz)) return 0.0f;
    float lateral = -ay + (-gy * 0.0025f);
    const float dz = 0.06f;
    if (lateral > dz)  return  constrain((lateral - dz)  / 0.35f, 0.0f, 1.6f);
    if (lateral < -dz) return -constrain((-lateral - dz) / 0.35f, 0.0f, 1.6f);
    return 0.0f;
  }

  // ── Reset functions ────────────────────────────────────────────────────────
  void resetRacer() {
    const int carW = SCREEN_WIDTH == 240 ? 16 : 8;
    const int carH = SCREEN_WIDTH == 240 ? 28 : 14;
    racPlayerX       = G_X + G_W / 2 - carW / 2;
    racPlayerY       = G_B - carH - 6;
    racSpeed         = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
    racNitroActive   = false;
    racNitroFuel     = 100.0f;
    racRoadScroll    = 0;
    racScore         = 0;
    racGameOver      = false;
    racNitroTapTime  = 0;
    racLastTapSide   = 0;
    racFlameFrame    = 0;
    for (int i = 0; i < 4; i++) {
      racCarX[i]     = G_X + 24 + random(0, 3) * (G_W - 48) / 3 - carW / 2;
      racCarY[i]     = G_Y - 30 - i * 60;
      racCarSpeed[i] = 1.2f + random(0, 20) / 10.0f;
      racCarColor[i] = i % 3;
      racCarActive[i]= true;
    }
  }

  void resetSpace() {
    spcPlayerX = G_X + G_W / 2;
    spcPlayerY = G_B - (SCREEN_WIDTH == 240 ? 20 : 12);
    spcScore   = 0; spcHealth = 3; spcGameOver = false;
    spcLastShoot = 0; spcShieldTime = 0; spcSmartBombCooldown = 0;
    spcSpreadActive = false;
    for (int i = 0; i < 6;  i++) spcLaserActive[i]      = false;
    for (int i = 0; i < 8;  i++) spcEnemyActive[i]      = false;
    for (int i = 0; i < 4;  i++) spcEnemyLaserActive[i] = false;
    for (int i = 0; i < 15; i++) spcPartActive[i]       = false;
    spcPowerActive = false;
    spcBossActive  = false; spcBossHP = 20;
    spcBossX = G_X + G_W / 2; spcBossY = G_Y + 15;
    spcBossDir = 1; spcBossLastShoot = 0;
    // Init star positions once
    if (!spcStarsInit) {
      for (int i = 0; i < 10; i++) { spcStar1X[i] = random(0, G_W); spcStar1Y[i] = random(G_Y, G_B); }
      for (int i = 0; i <  6; i++) { spcStar2X[i] = random(0, G_W); spcStar2Y[i] = random(G_Y, G_B); }
      spcStarsInit = true;
    }
  }

  void resetFlappy() {
    flapPlayerY   = G_Y + G_H / 2;
    flapPlayerVY  = 0;
    flapPipeX     = G_R;
    int gapH      = SCREEN_WIDTH == 240 ? 50 : 35;
    flapPipeGapY  = G_Y + 20 + random(0, G_H - gapH - 40);
    flapScore     = 0; flapGameOver = false;
    flapCloud1X   = G_W * 0.25f;
    flapCloud2X   = G_W * 0.70f;
  }

  void resetCatcher() {
    const int basketW = SCREEN_WIDTH == 240 ? 24 : 16;
    catPlayerX = G_X + G_W / 2 - basketW / 2;
    catScore = 0; catLives = 3; catGameOver = false;
    for (int i = 0; i < 3; i++) {
      catCoinsX[i]    = G_X + 16 + random(0, G_W - 48);
      catCoinsY[i]    = G_Y - 20 - i * 50;
      catCoinsActive[i] = true;
      catIsBomb[i]    = (random(0, 10) < 3);
    }
  }

  void resetJump() {
    jumpPlayerX  = G_X + G_W / 2;
    jumpPlayerY  = G_B - 40;
    jumpPlayerVY = -5.0f;
    jumpScore    = 0; jumpGameOver = false; jumpSquash = false;
    for (int i = 0; i < 5; i++) {
      jumpPlatX[i] = G_X + 10 + random(0, G_W - 50);
      jumpPlatY[i] = G_B - 15 - i * 36;
    }
  }

  void resetStacker() {
    stkBlockW       = SCREEN_WIDTH == 240 ? 60 : 40;
    stkBlockX       = G_X + (G_W - stkBlockW) / 2;
    stkBlockY       = G_B - (SCREEN_WIDTH == 240 ? 20 : 12);
    stkSpeed        = SCREEN_WIDTH == 240 ? 3.5f : 2.0f;
    stkDir          = 1; stkBaseX = stkBlockX;
    stkStackHeight  = 0; stkScore = 0; stkGameOver = false;
    stkFlashNew     = false;
    for (int i = 0; i < 12; i++) { stkStackX[i] = 0; stkStackW[i] = 0; }
  }

  void resetMemory() {
    memSeqLen       = 1; memSeqStep    = 0;
    memPlayerStep   = 0; memSelectedTile = 4;
    memState        = 0; memSeq[0]     = random(0, 9);
    memTimer        = millis();
    memScore        = 0; memGameOver   = false;
  }

  // ── Game Over eligibility ──────────────────────────────────────────────────
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

  // ── Shared: draw a pixel-art heart at (x, y) size s ──────────────────────
  void drawHeart(GFXcanvas16& display, int x, int y, int s, uint16_t col, bool filled) {
    if (filled) {
      display.fillCircle(x,     y, s, col);
      display.fillCircle(x + s*2, y, s, col);
      display.fillTriangle(x - s, y, x + s*3, y, x + s, y + s*2, col);
    } else {
      display.drawCircle(x,     y, s, col);
      display.drawCircle(x + s*2, y, s, col);
      display.drawTriangle(x - s, y, x + s*3, y, x + s, y + s*2, col);
    }
  }

  // ── Shared: Game Over screen ───────────────────────────────────────────────
  void drawGameOverScreen(GFXcanvas16& display, int score, int highScore) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x07FF : 0xF8B8;
    const bool newRecord = (score > 0 && score >= highScore);

    // Dark card
    display.fillRect(20, 60, SCREEN_WIDTH - 40, 170, 0x0841);
    display.drawRect(20, 60, SCREEN_WIDTH - 40, 170, newRecord ? 0xFD20 : 0xF800);
    display.drawRect(21, 61, SCREEN_WIDTH - 42, 168, newRecord ? 0xFD20 : 0xF800);

    // GAME OVER title
    display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
    display.setTextColor(0xF800);
    int goW = 9 * (SCREEN_WIDTH == 240 ? 18 : 12);
    display.setCursor((SCREEN_WIDTH - goW) / 2, SCREEN_WIDTH == 240 ? 78 : 70);
    display.print("GAME OVER");

    // Score pill
    display.fillRoundRect(40, SCREEN_WIDTH == 240 ? 120 : 95, SCREEN_WIDTH - 80, SCREEN_WIDTH == 240 ? 24 : 18, 6, 0x18C3);
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
      display.setCursor((SCREEN_WIDTH - 10 * (SCREEN_WIDTH == 240 ? 12 : 6)) / 2, SCREEN_WIDTH == 240 ? 153 : 120);
      display.print("NEW RECORD!");
    } else {
      display.setTextColor(0x8410);
      char hiBuf[20];
      snprintf(hiBuf, sizeof(hiBuf), "BEST:%d", highScore);
      int hw = strlen(hiBuf) * (SCREEN_WIDTH == 240 ? 12 : 6);
      display.setCursor((SCREEN_WIDTH - hw) / 2, SCREEN_WIDTH == 240 ? 153 : 120);
      display.print(hiBuf);
    }

    // Replay / exit prompts
    display.setTextColor(themeAccent);
    const char* pAgain = "TAP TO REPLAY";
    int pW = strlen(pAgain) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - pW) / 2, SCREEN_WIDTH == 240 ? 182 : 142);
    display.print(pAgain);

    display.setTextColor(0x8410);
    const char* bMenu = "SWIPE TO EXIT";
    int bW = strlen(bMenu) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - bW) / 2, SCREEN_WIDTH == 240 ? 200 : 156);
    display.print(bMenu);
  }

  // ============================================================
  //  ARCADE MENU
  // ============================================================
  void drawMenu(GFXcanvas16& display) {
    const uint16_t themeAccent  = 0x001F; // Royal blue
    const uint16_t themeBg      = 0xFFFF;
    const uint16_t themeText    = 0x1082;
    const uint16_t themeBorder  = 0xCE79;
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

    const int opt = constrain(gameMenuOption, 0, 7);

    const char* titlesL1[] = {"LUNA","LUNA","FLAPPY","COIN","MOCHY","STACKER","MEMORY","EXIT"};
    const char* titlesL2[] = {"RACER","SPACE","MOCHY","CATCHER","JUMP","TOWER","MATRIX","ARCADE"};
    int highScores[] = {
      racHighScore, spcHighScore, flapHighScore, catHighScore,
      jumpHighScore, stkHighScore, memHighScore, 0
    };

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
    if (opt < 7) {
      char buf[48];
      if (opt == 0 || opt == 1 || opt == 3 || opt == 4)
        snprintf(buf, sizeof(buf), "HI:%d // GYRO TILT ACTIVE", highScores[opt]);
      else
        snprintf(buf, sizeof(buf), "SIMULATION // HI-SCORE: %d", highScores[opt]);
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
        display.fillRoundRect(cx - 9,  cy - 10 + wave, 18, 14, 2, 0xFFFF); // windshield
        // Wheels
        display.fillCircle(cx - 10, cy + 20 + wave, 4, themeText);
        display.fillCircle(cx + 10, cy + 20 + wave, 4, themeText);
        display.fillCircle(cx - 10, cy - 14 + wave, 4, themeText);
        display.fillCircle(cx + 10, cy - 14 + wave, 4, themeText);
        // Nitro flame (flicker)
        display.fillTriangle(cx - 6, cy + 20 + wave, cx, cy + 28 + wave, cx + 6, cy + 20 + wave, 0xFB80);
      } break;

      case 1: { // LUNA SPACE
        display.drawCircle(cx, cy + wave, 32, themeBorder);
        display.drawFastHLine(cx - 38, cy + wave, 76, 0xCE79);
        display.drawFastVLine(cx, cy - 38 + wave, 76, 0xCE79);
        display.fillTriangle(cx, cy - 22 + wave, cx - 20, cy + 16 + wave, cx + 20, cy + 16 + wave, themeAccent);
        display.fillTriangle(cx, cy - 14 + wave, cx - 12, cy + 10 + wave, cx + 12, cy + 10 + wave, 0xF7BE);
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
        display.fillTriangle(cx + 12, cy + wave, cx + 12, cy + 4 + wave, cx + 20, cy + 2 + wave, 0xFD20);
        // Wing
        display.drawLine(cx - 8, cy + wave, cx - 18, cy - 10 + wave, themeText);
        display.drawLine(cx - 18, cy - 10 + wave, cx - 4, cy - 4 + wave, themeText);
      } break;

      case 3: { // COIN CATCHER
        // Spinning coin illusion
        int coinW = (int)(cosf(t / 300.0f) * 12);
        if (abs(coinW) < 2) coinW = 2;
        display.fillRoundRect(cx - abs(coinW), cy - 12 + wave, abs(coinW)*2, 24, 4, 0xFD20);
        display.drawCircle(cx, cy + wave, 26, themeBorder);
        // Basket
        display.fillRoundRect(cx - 24, cy + 28 + wave, 48, 8, 3, 0xC3A5);
        display.drawLine(cx - 24, cy + 28 + wave, cx - 28, cy + 22 + wave, 0x8410);
        display.drawLine(cx + 24, cy + 28 + wave, cx + 28, cy + 22 + wave, 0x8410);
      } break;

      case 4: { // MOCHY JUMP
        display.fillRoundRect(cx - 36, cy + 18 + wave, 30, 4, 2, themeAccent);
        display.fillRoundRect(cx - 6,  cy + 2  + wave, 32, 4, 2, themeAccent);
        display.fillRoundRect(cx + 12, cy - 16 + wave, 28, 4, 2, themeAccent);
        // Character with squash
        display.fillRoundRect(cx + 4, cy - 28 + wave, 12, 14, 2, TFT_YELLOW);
        display.fillCircle(cx + 7,  cy - 24 + wave, 1, TFT_BLACK);
        display.fillCircle(cx + 13, cy - 24 + wave, 1, TFT_BLACK);
        // Jump trail
        display.drawLine(cx + 10, cy - 14 + wave, cx + 10, cy - 10 + wave, 0x07E0);
      } break;

      case 5: { // STACKER
        display.drawFastVLine(cx, cy - 28, 56, 0xCE79);
        uint16_t colors[] = {themeAccent, 0x07FF, 0xF7BE, themeAccent};
        int widths[] = {48, 36, 24, 14};
        for (int i = 0; i < 4; i++) {
          int by = cy + 12 - i * 12 + wave;
          display.fillRect(cx - widths[i]/2, by, widths[i], 10, colors[i]);
          display.drawRect(cx - widths[i]/2, by, widths[i], 10, TFT_WHITE);
        }
      } break;

      case 6: { // MEMORY MATRIX
        // Pulsing tile grid
        for (int r = 0; r < 3; r++) {
          for (int c = 0; c < 3; c++) {
            int tx = cx - 30 + c * 22;
            int ty = cy - 22 + r * 22 + wave;
            uint16_t col = (r == (t/400)%3 && c == (t/300)%3) ? TFT_YELLOW : 0x0182;
            display.fillRoundRect(tx, ty, 18, 18, 3, col);
            display.drawRoundRect(tx, ty, 18, 18, 3, TFT_WHITE);
          }
        }
      } break;

      case 7: { // EXIT
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
    const uint16_t btnColor = (opt == 7) ? 0xF800 : themeAccent;
    // Subtle shadow
    display.fillRoundRect(btnX + 2, btnY + 2, btnW, btnH, 8, 0x8410);
    display.fillRoundRect(btnX, btnY, btnW, btnH, 8, btnColor);
    // Inner highlight strip
    display.drawFastHLine(btnX + 8, btnY + 4, btnW - 16, blend565(btnColor, TFT_WHITE));

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    const char* btnTxt = (opt == 7) ? "EXIT <" : "PLAY >";
    int bW = strlen(btnTxt) * 12;
    display.setCursor(btnX + (btnW - bW) / 2, btnY + 9);
    display.print(btnTxt);

    // ── Carousel indicator ─────────────────────────────────────────────────
    const int dotStartX = 72;
    for (int d = 0; d < 8; d++) {
      int dx = dotStartX + d * 14;
      if (d == opt) {
        display.fillRoundRect(dx - 4, 238, 12, 4, 2, btnColor);
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
  void updateAndDrawRacer(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent  = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    const uint16_t themeBg      = 0x0841; // Dark HUD bar
    const uint16_t themeText    = TFT_WHITE;
    const uint16_t themeBorder  = 0x4208;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;
    const bool justBtn1 = btn1 && !lastBtn1;
    const bool justBtn2 = btn2 && !lastBtn2;

    const int carW = SCREEN_WIDTH == 240 ? 18 : 9;
    const int carH = SCREEN_WIDTH == 240 ? 30 : 15;

    if (!racGameOver) {
      // ── Nitro: alternating rapid tap (fixed single-touch limitation) ──────
      if (justBtn1) {
        if (racLastTapSide == 2 && (millis() - racNitroTapTime) < 450) {
          if (racNitroFuel > 5.0f) racNitroActive = true;
        }
        racLastTapSide  = 1;
        racNitroTapTime = millis();
      } else if (justBtn2) {
        if (racLastTapSide == 1 && (millis() - racNitroTapTime) < 450) {
          if (racNitroFuel > 5.0f) racNitroActive = true;
        }
        racLastTapSide  = 2;
        racNitroTapTime = millis();
      }

      // Nitro burn / extinguish
      if (racNitroActive) {
        racSpeed        = SCREEN_WIDTH == 240 ? 8.5f : 5.5f;
        racNitroFuel   -= 0.7f;
        racFlameFrame   = (racFlameFrame + 1) % 6;
        if (millis() % 400 < 200) audio.playSound(SOUND_JUMP);
        if (racNitroFuel <= 0) { racNitroFuel = 0; racNitroActive = false; }
      } else {
        // Passive recharge
        racNitroFuel = min(100.0f, racNitroFuel + 0.10f);
        racSpeed = SCREEN_WIDTH == 240 ? 4.0f : 2.5f;
        racFlameFrame = 0;
      }

      // ── Steering ──────────────────────────────────────────────────────────
      float steerDelta = 0.0f;
      if (btn1 && !btn2) steerDelta -= 3.5f;
      else if (btn2 && !btn1) steerDelta += 3.5f;
      float tilt = getTiltSteer();
      if (fabsf(tilt) > 0.01f) steerDelta += tilt * 5.2f;
      racPlayerX = constrain(racPlayerX + steerDelta, (float)(G_X + 18), (float)(G_R - 18 - carW));

      // ── Road physics ──────────────────────────────────────────────────────
      racRoadScroll += racSpeed;
      if (racRoadScroll >= 40) racRoadScroll = 0;
      racScore += (int)(racSpeed / 5) + (racNitroActive ? 1 : 0);

      // ── Traffic cars ──────────────────────────────────────────────────────
      for (int i = 0; i < 4; i++) {
        if (racCarActive[i]) {
          racCarY[i] += (racSpeed - racCarSpeed[i]);
          if (racCarY[i] > G_B + 10) {
            racCarY[i]    = G_Y - 30;
            racCarX[i]    = G_X + 24 + random(0, 3) * (G_W - 48) / 3 - carW / 2;
            racCarSpeed[i] = 1.0f + random(0, 20) / 10.0f;
            racCarActive[i] = true;
          }
          // Collision
          if (abs((int)racPlayerX - (int)racCarX[i]) < (SCREEN_WIDTH == 240 ? 14 : 7) &&
              abs((int)racPlayerY - (int)racCarY[i]) < (SCREEN_WIDTH == 240 ? 26 : 13)) {
            racGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("rac", racHighScore, racScore);
          }
        }
      }

    } else {
      if (justBtn1) resetRacer();
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
        display.fillRect(G_X + G_W / 2 - 1, sy, 2, SCREEN_WIDTH == 240 ? 20 : 10, 0x07FF);
    }

    // Grass detail strips
    for (int y = G_Y + 10; y < G_B; y += 25) {
      display.fillRect(G_X + 4,  y, 6, 3, 0x03E0);
      display.fillRect(G_R - 10, y, 6, 3, 0x03E0);
    }

    // ── Draw traffic cars (with proper front detail) ──────────────────────
    const uint16_t carBodyColors[] = {0xF800, 0x001F, 0x03E0};   // red, blue, green
    const uint16_t carRoofColors[] = {0xC000, 0x000C, 0x0280};
    for (int i = 0; i < 4; i++) {
      if (racCarActive[i] && racCarY[i] >= G_Y && racCarY[i] < G_B) {
        int cx = (int)racCarX[i], cy = (int)racCarY[i];
        uint16_t bodyCol = carBodyColors[racCarColor[i]];
        uint16_t roofCol = carRoofColors[racCarColor[i]];
        // Body
        display.fillRoundRect(cx, cy, carW, carH, 2, bodyCol);
        // Roof
        display.fillRect(cx + carW/4, cy + carH/5, carW/2, carH*2/5, roofCol);
        // Windshield
        display.fillRect(cx + carW/4 + 1, cy + carH/5 + 1, carW/2 - 2, carH/5, 0x07FF);
        // Headlights
        display.fillRect(cx + 1,          cy, carW/4, carH/8, 0xFFE0);
        display.fillRect(cx + carW*3/4-1, cy, carW/4, carH/8, 0xFFE0);
        // Tail lights
        display.fillRect(cx + 1,          cy + carH - carH/8, carW/4, carH/8, 0xF800);
        display.fillRect(cx + carW*3/4-1, cy + carH - carH/8, carW/4, carH/8, 0xF800);
      }
    }

    // ── Draw player car ──────────────────────────────────────────────────
    {
      int px = (int)racPlayerX, py = (int)racPlayerY;
      // Body
      display.fillRoundRect(px, py, carW, carH, 2, 0xFB40);
      // Roof / cabin
      display.fillRect(px + carW/4, py + carH/5, carW/2, carH*2/5, 0xC840);
      // Windshield (white)
      display.fillRect(px + carW/4 + 1, py + carH/5 + 1, carW/2 - 2, carH/5, TFT_WHITE);
      // Front headlights (white-yellow)
      display.fillRect(px + 1,          py, carW/4, carH/8, 0xFFFF);
      display.fillRect(px + carW*3/4-1, py, carW/4, carH/8, 0xFFFF);
      // Tail lights
      display.fillRect(px + 1,          py + carH - carH/8, carW/4, carH/8, 0xF800);
      display.fillRect(px + carW*3/4-1, py + carH - carH/8, carW/4, carH/8, 0xF800);
      // Wheels
      display.fillCircle(px + carW/5,      py + carH - 2, carW/5, 0x18C3);
      display.fillCircle(px + carW*4/5,    py + carH - 2, carW/5, 0x18C3);
      display.fillCircle(px + carW/5,      py + 4,        carW/5, 0x18C3);
      display.fillCircle(px + carW*4/5,    py + 4,        carW/5, 0x18C3);
    }

    // ── Nitro flame FX ───────────────────────────────────────────────────
    if (racNitroActive) {
      int px = (int)racPlayerX, py = (int)racPlayerY;
      const uint16_t flameColors[] = {0xFB40, 0xFD20, 0xFFE0, 0xFB40, 0xF800, 0xFD60};
      uint16_t fc = flameColors[racFlameFrame];
      int flameH = 8 + (racFlameFrame % 3) * 4;
      display.fillTriangle(px + carW/4,     py + carH,
                           px + carW/2,     py + carH + flameH,
                           px + 3*carW/4,   py + carH, fc);
      display.fillTriangle(px + carW/2 - 3, py + carH,
                           px + carW/2,     py + carH + flameH + 4,
                           px + carW/2 + 3, py + carH, TFT_WHITE);
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
    uint16_t fuelCol = racNitroActive ? 0xFD20 : (racNitroFuel > 50 ? 0x07E0 : 0xF800);
    display.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 1, fuelCol);

    // Tap hint
    display.setTextSize(1);
    display.setTextColor(0x4A49);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 52 : 18);
    display.print("L>R TAP=NITRO");

    if (racGameOver) drawGameOverScreen(display, racScore, racHighScore);
  }

  // ============================================================
  //  GAME 2: LUNA SPACE
  // ============================================================
  void updateAndDrawSpace(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;
    const bool justBtn1 = btn1 && !lastBtn1;
    const bool justBtn2 = btn2 && !lastBtn2;

    const int pw = SCREEN_WIDTH == 240 ? 12 : 6;
    const int ph = SCREEN_WIDTH == 240 ? 16 : 8;
    const int ew = SCREEN_WIDTH == 240 ? 12 : 6;
    const int eh = SCREEN_WIDTH == 240 ? 8  : 4;
    const int playerColW = SCREEN_WIDTH == 240 ? 16 : 10;
    const int playerColH = SCREEN_WIDTH == 240 ? 12 :  6;
    const int enemyColW  = SCREEN_WIDTH == 240 ? 18 : 10;
    const int enemyColH  = SCREEN_WIDTH == 240 ? 14 :  8;

    if (!spcGameOver) {
      // Smart bomb (btn1+btn2 within 200ms of each other — use edge detect trick)
      if (justBtn1 && btn2 && (millis() - spcSmartBombCooldown > 8000)) {
        spcSmartBombCooldown = millis();
        audio.playSound(SOUND_GAMEOVER);
        for (int i = 0; i < 4;  i++) spcEnemyLaserActive[i] = false;
        for (int i = 0; i < 8;  i++) {
          if (spcEnemyActive[i]) {
            spcEnemyHP[i]--;
            if (spcEnemyHP[i] <= 0) { spcEnemyActive[i] = false; spcScore += 20; }
          }
        }
        if (spcBossActive) spcBossHP -= 3;
        for (int p = 0; p < 15; p++) {
          spcPartActive[p] = true;
          spcPartX[p] = G_X + G_W / 2; spcPartY[p] = G_Y + G_H / 2;
          spcPartVX[p] = random(-40, 41) / 10.0f;
          spcPartVY[p] = random(-40, 41) / 10.0f;
          spcPartLife[p] = 14;
        }
      }

      // Steering
      float steerDelta = 0.0f;
      if (btn1 && !btn2) steerDelta -= 3.5f;
      else if (btn2 && !btn1) steerDelta += 3.5f;
      float tilt = getTiltSteer();
      if (fabsf(tilt) > 0.01f) steerDelta += tilt * 5.5f;
      spcPlayerX = constrain(spcPlayerX + steerDelta, (float)(G_X + pw + 4), (float)(G_R - pw - 4));

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
          if (spcLaserY[i] < G_Y) spcLaserActive[i] = false;
        }
      }

      // Move enemy lasers + hit player
      for (int i = 0; i < 4; i++) {
        if (spcEnemyLaserActive[i]) {
          spcEnemyLaserY[i] += 2.5f;
          if (spcEnemyLaserY[i] > G_B) { spcEnemyLaserActive[i] = false; continue; }
          if (abs((int)spcEnemyLaserX[i] - (int)spcPlayerX) < playerColW &&
              spcEnemyLaserY[i] >= spcPlayerY - playerColH &&
              spcEnemyLaserY[i] <= spcPlayerY + playerColH / 2) {
            spcEnemyLaserActive[i] = false;
            if (millis() > spcShieldTime) {
              audio.playSound(SOUND_POWERDOWN);
              if (--spcHealth <= 0) { spcGameOver = true; audio.playSound(SOUND_GAMEOVER); saveHighScore("spc", spcHighScore, spcScore); }
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
            if (--spcHealth <= 0) { spcGameOver = true; audio.playSound(SOUND_GAMEOVER); saveHighScore("spc", spcHighScore, spcScore); }
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
                  spcPowerX = spcEnemyX[i]; spcPowerY = spcEnemyY[i];
                  spcPowerType = random(0, 2);
                }
                for (int p = 0; p < 4; p++) {
                  int pIdx = random(0, 15);
                  spcPartActive[pIdx] = true;
                  spcPartX[pIdx] = spcEnemyX[i]; spcPartY[pIdx] = spcEnemyY[i];
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
          spcBossActive = true; spcBossHP = 20;
        } else {
          for (int i = 0; i < 4; i++) {
            spcEnemyActive[i] = true;
            spcEnemyX[i] = G_X + 24 + i * (G_W - 48) / 4;
            spcEnemyY[i] = G_Y - 20;
            spcEnemyHP[i] = 1; spcEnemyType[i] = i % 2;
            spcEnemyLastShoot[i] = millis();
          }
        }
      }

      // Power-up movement
      if (spcPowerActive) {
        spcPowerY += 1.2f;
        if (spcPowerY > G_B) spcPowerActive = false;
        if (abs((int)spcPowerX - (int)spcPlayerX) < 16 && abs((int)spcPowerY - (int)spcPlayerY) < 16) {
          spcPowerActive = false;
          audio.playSound(SOUND_POWERUP);
          if (spcPowerType == 0) spcSpreadActive = true;
          else spcShieldTime = millis() + 5000;
        }
      }

      // Particles
      for (int i = 0; i < 15; i++) {
        if (spcPartActive[i]) {
          spcPartX[i] += spcPartVX[i]; spcPartY[i] += spcPartVY[i];
          if (--spcPartLife[i] <= 0) spcPartActive[i] = false;
        }
      }

      // Boss update
      if (spcBossActive) {
        spcBossX += spcBossDir * 0.8f;
        if (spcBossX < G_X + 32) spcBossDir = 1;
        if (spcBossX > G_R - 32) spcBossDir = -1;
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
        const int bh = SCREEN_WIDTH == 240 ? 16 :  8;
        for (int l = 0; l < 6; l++) {
          if (spcLaserActive[l] &&
              abs((int)spcLaserX[l] - (int)spcBossX) < bw + 4 &&
              abs((int)spcLaserY[l] - (int)spcBossY) < bh + 4) {
            spcLaserActive[l] = false;
            audio.playSound(SOUND_JUMP);
            if (--spcBossHP <= 0) {
              spcBossActive = false; spcScore += 200;
              spcGameOver = true; spcHealth = 99;
              audio.playSound(SOUND_POWERUP);
              saveHighScore("spc", spcHighScore, spcScore);
            }
          }
        }
      }
    } else {
      if (justBtn1) resetSpace();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

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
        display.fillRect((int)spcLaserX[i], (int)spcLaserY[i], 2, SCREEN_WIDTH==240?8:4, 0x07FF);
        display.drawPixel((int)spcLaserX[i], (int)spcLaserY[i], TFT_WHITE);
      }
    }
    for (int i = 0; i < 4; i++) {
      if (spcEnemyLaserActive[i]) {
        display.fillRect((int)spcEnemyLaserX[i], (int)spcEnemyLaserY[i], 2, SCREEN_WIDTH==240?8:4, 0xF800);
        display.drawPixel((int)spcEnemyLaserX[i], (int)spcEnemyLaserY[i], 0xFD20);
      }
    }

    // Particles (orange/yellow sparks)
    const uint16_t partColors[] = {0xFD20, 0xFB40, 0xF800, 0xFFE0};
    for (int i = 0; i < 15; i++) {
      if (spcPartActive[i]) {
        uint16_t pc = partColors[spcPartLife[i] % 4];
        display.drawPixel((int)spcPartX[i], (int)spcPartY[i], pc);
        if (spcPartLife[i] > 4)
          display.drawPixel((int)spcPartX[i]+1, (int)spcPartY[i], pc);
      }
    }

    // Power-up
    if (spcPowerActive) {
      const int pwSize = SCREEN_WIDTH == 240 ? 12 : 8;
      uint16_t pwCol = spcPowerType == 0 ? 0xFFE0 : 0x07E0;
      display.fillRoundRect((int)spcPowerX - pwSize/2, (int)spcPowerY - pwSize/2, pwSize, pwSize, 3, pwCol);
      display.setTextColor(TFT_BLACK); display.setTextSize(1);
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
          display.fillCircle(ex, ey, ew/3, TFT_WHITE);
        } else {
          // X-fighter
          display.fillRect(ex - ew, ey - 2, ew*2, 4, 0xFC00);
          display.fillRect(ex - 2, ey - eh, 4, eh*2, 0xFC00);
          display.fillCircle(ex, ey, 3, 0xFFFF);
        }
      }
    }

    // Boss
    if (spcBossActive) {
      const int bw = SCREEN_WIDTH==240 ? 32 : 16;
      const int bh = SCREEN_WIDTH==240 ? 16 :  8;
      int bx = (int)spcBossX, by = (int)spcBossY;
      // Main body
      display.fillRoundRect(bx - bw, by - bh, bw*2, bh*2, 4, 0xA000);
      // Wings
      display.fillTriangle(bx - bw, by, bx - bw - 12, by - bh, bx - bw, by - bh, 0xF800);
      display.fillTriangle(bx + bw, by, bx + bw + 12, by - bh, bx + bw, by - bh, 0xF800);
      // Core
      display.fillCircle(bx, by, bh/2, 0xFFE0);
      // Engine cannons
      display.fillRect(bx - bw/3 - 2, by + bh, 4, bh/2, 0xFD20);
      display.fillRect(bx + bw/3 - 2, by + bh, 4, bh/2, 0xFD20);
      // HP bar
      int hpBarW = bw * 2 - 4;
      display.drawFastHLine(bx - bw + 2, by - bh - 6, hpBarW, 0x4208);
      display.drawFastHLine(bx - bw + 2, by - bh - 6, spcBossHP * hpBarW / 20, 0x07E0);
    }

    // Player ship — body + wings + canopy + engine glow
    {
      int px = (int)spcPlayerX, py = (int)spcPlayerY;
      // Main thrust engine glow
      display.fillCircle(px, py + ph/2, 3, 0x07E0);
      // Body
      display.fillTriangle(px, py - ph, px - pw, py + ph/2, px + pw, py + ph/2, themeAccent);
      // Wings
      display.fillTriangle(px - pw, py, px - pw - 6, py + ph/2, px - pw/2, py + ph/2, 0x4208);
      display.fillTriangle(px + pw, py, px + pw + 6, py + ph/2, px + pw/2, py + ph/2, 0x4208);
      // Canopy
      display.fillRect(px - 2, py - ph/2, 4, ph/3, 0x07FF);
      // Side mini-engines
      display.fillCircle(px - pw - 2, py + ph/2, 2, 0xFD20);
      display.fillCircle(px + pw + 2, py + ph/2, 2, 0xFD20);
      // Shield ring
      if (millis() < spcShieldTime)
        display.drawCircle(px, py - ph/4, SCREEN_WIDTH==240?20:10, 0x07E0);
    }

    // ── HUD ──────────────────────────────────────────────────────────────
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("SC:%03d", spcScore);

    // Hearts for health
    for (int i = 0; i < 3; i++) {
      int hx = SCREEN_WIDTH - (SCREEN_WIDTH==240?54:32) + i*(SCREEN_WIDTH==240?18:10);
      int hy = SCREEN_WIDTH==240?38:9;
      drawHeart(display, hx, hy, 3, i < spcHealth ? 0xF800 : 0x4208, i < spcHealth);
    }

    if (spcGameOver) {
      if (spcHealth == 99) {
        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0841);
        display.setTextSize(SCREEN_WIDTH==240?3:2);
        display.setTextColor(0x07E0);
        display.setCursor((SCREEN_WIDTH - 8*(SCREEN_WIDTH==240?18:12))/2, G_Y+20);
        display.print("VICTORY!");
        display.setTextSize(SCREEN_WIDTH==240?2:1);
        display.setTextColor(TFT_WHITE);
        display.setCursor(20, G_Y+60);
        display.print("SPACE BOSS DESTROYED!");
        display.setTextColor(themeAccent);
        display.setCursor(20, G_Y+90);
        display.print("TAP TO PLAY AGAIN");
      } else {
        drawGameOverScreen(display, spcScore, spcHighScore);
      }
    }
  }

  // ============================================================
  //  GAME 3: FLAPPY MOCHY
  // ============================================================
  void updateAndDrawFlappy(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    const uint16_t themeBorder = 0xCE79;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int mSize = SCREEN_WIDTH==240? 9 : 5;
    const int pW    = SCREEN_WIDTH==240?24 :16;
    const int gapH  = SCREEN_WIDTH==240?52 :36;

    if (!flapGameOver) {
      if (btn2 && !lastBtn2) {
        flapPlayerVY = SCREEN_WIDTH==240?-4.0f:-3.0f;
        audio.playSound(SOUND_JUMP);
      }
      flapPlayerVY += SCREEN_WIDTH==240?0.22f:0.18f;
      flapPlayerY  += flapPlayerVY;

      // Animate clouds
      flapCloud1X -= 0.4f; if (flapCloud1X < -30) flapCloud1X = G_R + 10;
      flapCloud2X -= 0.7f; if (flapCloud2X < -30) flapCloud2X = G_R + 10;

      flapPipeX -= SCREEN_WIDTH==240?2.2f:1.5f;
      if (flapPipeX < G_X - pW) {
        flapPipeX = G_R;
        flapPipeGapY = G_Y + 20 + random(0, G_H - gapH - 40);
        flapScore++;
        audio.playSound(SOUND_CHIRP);
      }

      if (flapPlayerY < G_Y || flapPlayerY > G_B - mSize*2) {
        flapGameOver = true; audio.playSound(SOUND_GAMEOVER);
        saveHighScore("flap", flapHighScore, flapScore);
      }
      if (flapPipeX > G_X + G_W/2 - pW && flapPipeX < G_X + G_W/2 + mSize) {
        if (flapPlayerY < flapPipeGapY || flapPlayerY > flapPipeGapY + gapH - mSize*2) {
          flapGameOver = true; audio.playSound(SOUND_GAMEOVER);
          saveHighScore("flap", flapHighScore, flapScore);
        }
      }
    } else {
      if (btn1 && !lastBtn1) resetFlappy();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

    // ── Sky gradient (3 bands) ─────────────────────────────────────────────
    display.fillRect(G_X, G_Y,         G_W, G_H/3,   0xAEFF); // top light sky
    display.fillRect(G_X, G_Y+G_H/3,   G_W, G_H/3,   0x7E5F); // mid sky
    display.fillRect(G_X, G_Y+2*G_H/3, G_W, G_H/3+1, 0x5D1F); // bottom darker

    // Animated clouds
    auto drawCloud = [&](int cx, int cy) {
      display.fillCircle(cx,    cy, 7,  TFT_WHITE);
      display.fillCircle(cx+10, cy-2, 9, TFT_WHITE);
      display.fillCircle(cx+20, cy,  7,  TFT_WHITE);
    };
    drawCloud((int)flapCloud1X, G_Y + G_H/6);
    drawCloud((int)flapCloud2X, G_Y + G_H/4 + 10);

    // Ground
    display.fillRect(G_X, G_B - 10, G_W, 10, 0x03E0);
    display.fillRect(G_X, G_B - 10, G_W, 3,  0x07E0); // highlight
    for (int gx = G_X; gx < G_R; gx += 12)
      display.drawFastVLine(gx, G_B - 10, 10, 0x02A0);

    // Pipes
    if (flapPipeX >= G_X && flapPipeX < G_R) {
      const uint16_t pipeCol  = 0x1AE2;
      const uint16_t pipeHL   = 0x37E7;
      // Top pipe
      display.fillRect((int)flapPipeX, G_Y, pW, (int)(flapPipeGapY - G_Y), pipeCol);
      display.fillRect((int)flapPipeX, G_Y, 3,  (int)(flapPipeGapY - G_Y), pipeHL);
      display.fillRect((int)flapPipeX - 2, (int)flapPipeGapY - 9, pW+4, 9, pipeCol);
      display.drawRect ((int)flapPipeX - 2, (int)flapPipeGapY - 9, pW+4, 9, TFT_BLACK);
      // Bottom pipe
      int bPipeTop = (int)flapPipeGapY + gapH;
      display.fillRect((int)flapPipeX, bPipeTop, pW, G_B - bPipeTop, pipeCol);
      display.fillRect((int)flapPipeX, bPipeTop, 3,  G_B - bPipeTop, pipeHL);
      display.fillRect((int)flapPipeX - 2, bPipeTop, pW+4, 9, pipeCol);
      display.drawRect ((int)flapPipeX - 2, bPipeTop, pW+4, 9, TFT_BLACK);
    }

    // Mochy character — 2-frame wing animation based on VY
    {
      int px = G_X + G_W/2 - mSize;
      int py = (int)flapPlayerY;
      // Body
      display.fillCircle(px + mSize, py + mSize, mSize, TFT_YELLOW);
      display.drawCircle(px + mSize, py + mSize, mSize, 0xFDA0);
      // Eye
      display.fillCircle(px + mSize + mSize/2, py + mSize/2, mSize/3+1, TFT_WHITE);
      display.fillCircle(px + mSize + mSize/2 + 1, py + mSize/2, mSize/4, TFT_BLACK);
      // Beak
      display.fillTriangle(px + mSize*2 - 2, py + mSize, px + mSize*2 + 3, py + mSize + 2, px + mSize*2 - 2, py + mSize + 4, 0xFD20);
      // Wing (flap based on VY)
      if (flapPlayerVY < 0) {
        display.fillTriangle(px, py + mSize, px - mSize - 2, py + mSize/2, px, py + mSize/2, 0xFDA0);
      } else {
        display.fillTriangle(px, py + mSize, px - mSize - 2, py + mSize + mSize/2, px, py + mSize + 2, 0xFDA0);
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("SC:%03d", flapScore);
    display.setCursor(SCREEN_WIDTH-(SCREEN_WIDTH==240?90:54), SCREEN_WIDTH==240?36:7);
    display.printf("HI:%03d", flapHighScore);

    if (flapGameOver) drawGameOverScreen(display, flapScore, flapHighScore);
  }

  // ============================================================
  //  GAME 4: COIN CATCHER
  // ============================================================
  void updateAndDrawCatcher(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int basketW = SCREEN_WIDTH==240?26:17;
    const int coinR   = SCREEN_WIDTH==240? 6: 3;

    if (!catGameOver) {
      float tilt = getTiltSteer();
      float steerDelta = 0.0f;
      if (btn1 && !btn2) steerDelta -= 4.0f;
      else if (btn2 && !btn1) steerDelta += 4.0f;
      if (fabsf(tilt) > 0.01f) steerDelta += tilt * 6.0f;
      catPlayerX = constrain(catPlayerX + steerDelta, (float)(G_X + 8), (float)(G_R - basketW - 8));

      float fallSpeed = SCREEN_WIDTH==240?(2.0f + catScore/80.0f):(1.4f + catScore/80.0f);
      for (int i = 0; i < 3; i++) {
        if (catCoinsActive[i]) {
          catCoinsY[i] += fallSpeed;
          if (catCoinsY[i] > G_B - 6) {
            if (!catIsBomb[i]) {
              if (--catLives <= 0) { catGameOver = true; audio.playSound(SOUND_GAMEOVER); saveHighScore("cat", catHighScore, catScore); }
              else audio.playSound(SOUND_POWERDOWN);
            }
            catCoinsX[i] = G_X + 16 + random(0, G_W - 48);
            catCoinsY[i] = G_Y - 20;
            catIsBomb[i] = (random(0, 10) < 3);
          }
          if (catCoinsY[i] >= G_B - 22 && catCoinsY[i] <= G_B - 10) {
            if (catCoinsX[i] + coinR >= catPlayerX && catCoinsX[i] <= catPlayerX + basketW) {
              if (catIsBomb[i]) {
                if (--catLives <= 0) { catGameOver = true; audio.playSound(SOUND_GAMEOVER); saveHighScore("cat", catHighScore, catScore); }
                else audio.playSound(SOUND_POWERDOWN);
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
      if (btn1 && !lastBtn1) resetCatcher();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

    // ── Background: night sky ────────────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H, 0x0001);
    // Static stars
    unsigned long t = millis();
    for (int i = 0; i < 16; i++) {
      int sx = (i * 43 + 7)  % G_W + G_X;
      int sy = (i * 31 + 11) % G_H + G_Y;
      if ((t/600 + i) % 2 == 0) display.drawPixel(sx, sy, TFT_WHITE);
    }

    // Ground
    display.fillRect(G_X, G_B - 8, G_W, 8, 0x0262);
    display.drawFastHLine(G_X, G_B - 8, G_W, 0x07E0);

    // Basket — trapezoid via rects
    {
      int bx = (int)catPlayerX, by = G_B - 16;
      display.fillRoundRect(bx, by, basketW, 7, 2, 0xC3A5);  // rim
      display.fillRect(bx + 3, by + 7, basketW - 6, 3, 0x8200); // net shadow
      // Handles
      display.drawLine(bx - 2, by, bx - 2, by - 3, 0x8410);
      display.drawLine(bx + basketW + 2, by, bx + basketW + 2, by - 3, 0x8410);
    }

    // Coins & bombs
    for (int i = 0; i < 3; i++) {
      if (catCoinsActive[i]) {
        int cx = (int)catCoinsX[i], cy = (int)catCoinsY[i];
        if (catIsBomb[i]) {
          // Bomb body
          display.fillCircle(cx + coinR, cy + coinR, coinR, 0x2104);
          display.fillCircle(cx + coinR, cy + coinR, coinR - 1, 0x39E7);
          // Fuse (flicker)
          bool fuseOn = (t / 150) % 2;
          display.fillRect(cx + coinR - 1, cy - 1, 2, 3, TFT_ORANGE);
          if (fuseOn) display.fillCircle(cx + coinR, cy - 2, 1, TFT_RED);
        } else {
          // Coin with shimmer
          display.fillCircle(cx + coinR, cy + coinR, coinR, 0xFDA0);
          display.drawCircle(cx + coinR, cy + coinR, coinR, 0xFFE0);
          // Shimmer pixel
          if ((t / 200 + i) % 3 == 0)
            display.fillCircle(cx + coinR - 1, cy + coinR - 1, 1, TFT_WHITE);
          // Dollar symbol
          display.setTextSize(1); display.setTextColor(0xC840);
          display.setCursor(cx + coinR - 2, cy + coinR - 3);
          display.print("$");
        }
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("SC:%03d", catScore);

    // Heart lives
    for (int i = 0; i < 3; i++) {
      int hx = SCREEN_WIDTH - (SCREEN_WIDTH==240?54:32) + i*(SCREEN_WIDTH==240?18:10);
      int hy = SCREEN_WIDTH==240?38:9;
      drawHeart(display, hx, hy, 3, i < catLives ? 0xF800 : 0x4208, i < catLives);
    }

    if (catGameOver) drawGameOverScreen(display, catScore, catHighScore);
  }

  // ============================================================
  //  GAME 5: MOCHY JUMP
  // ============================================================
  void updateAndDrawJump(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int platW = SCREEN_WIDTH==240?36:24;
    const int pW    = SCREEN_WIDTH==240?14: 9;
    const int pH    = SCREEN_WIDTH==240?16:10;

    float steerDelta = 0.0f; // hoisted to function scope for draw section

    if (!jumpGameOver) {
      float tilt = getTiltSteer();
      if (btn1 && !btn2) steerDelta -= 3.2f;
      else if (btn2 && !btn1) steerDelta += 3.2f;
      if (fabsf(tilt) > 0.01f) steerDelta += tilt * 5.2f;
      jumpPlayerX += steerDelta;

      // Wrap around
      if (jumpPlayerX < G_X - pW) jumpPlayerX = G_R - pW;
      if (jumpPlayerX > G_R + pW) jumpPlayerX = G_X - pW;

      jumpPlayerVY += 0.18f;
      jumpPlayerY  += jumpPlayerVY;

      jumpSquash = false;

      if (jumpPlayerY < G_Y + G_H / 2) {
        float diff = (G_Y + G_H / 2) - jumpPlayerY;
        jumpPlayerY = G_Y + G_H / 2;
        jumpScore  += (int)(diff / 2);
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
          if (jumpPlayerX + pW >= jumpPlatX[i] && jumpPlayerX <= jumpPlatX[i] + platW &&
              jumpPlayerY + pH >= jumpPlatY[i] && jumpPlayerY + pH - 4 <= jumpPlatY[i] + 4) {
            jumpPlayerVY = SCREEN_WIDTH==240?-5.8f:-4.8f;
            jumpSquash   = true;
            audio.playSound(SOUND_JUMP);
            break;
          }
        }
      }

      if (jumpPlayerY > G_B) {
        jumpGameOver = true; audio.playSound(SOUND_GAMEOVER);
        saveHighScore("jump", jumpHighScore, jumpScore);
      }
    } else {
      if (btn1 && !lastBtn1) resetJump();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

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
      display.drawPixel(shimX+1, py, TFT_WHITE);
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
      display.fillCircle(px + eyeOff,     drawY + drawH/4, 1, TFT_BLACK);
      display.fillCircle(px + eyeOff + 4, drawY + drawH/4, 1, TFT_BLACK);
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
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("ALT:%04d", jumpScore);
    display.setCursor(SCREEN_WIDTH-(SCREEN_WIDTH==240?90:54), SCREEN_WIDTH==240?36:7);
    display.printf("HI:%04d", jumpHighScore);

    if (jumpGameOver) drawGameOverScreen(display, jumpScore, jumpHighScore);
  }

  // ============================================================
  //  GAME 6: STACKER
  // ============================================================
  void updateAndDrawStacker(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int blockH = SCREEN_WIDTH==240?12:8;

    if (!stkGameOver) {
      stkBlockX += stkDir * stkSpeed;
      if (stkBlockX < G_X + 6)           { stkBlockX = G_X + 6;             stkDir = 1; }
      if (stkBlockX + stkBlockW > G_R-6) { stkBlockX = G_R - 6 - stkBlockW; stkDir = -1; }

      if (btn2 && !lastBtn2) {
        stkFlashNew = false;
        if (stkStackHeight == 0) {
          stkBaseX = stkBlockX;
          stkStackX[0] = stkBlockX; stkStackW[0] = stkBlockW;
          stkStackHeight = 1; stkScore += 10;
          audio.playSound(SOUND_COIN);
          stkFlashNew = true;
        } else {
          float prevX = stkStackX[stkStackHeight - 1];
          float prevW = stkStackW[stkStackHeight - 1];
          float oL    = max(stkBlockX, prevX);
          float oR    = min(stkBlockX + stkBlockW, prevX + prevW);
          float oW    = oR - oL;
          if (oW <= 0) {
            stkGameOver = true; audio.playSound(SOUND_GAMEOVER);
            saveHighScore("stk", stkHighScore, stkScore);
          } else {
            stkBlockW = oW; stkBlockX = oL;
            if (stkStackHeight < 10) {
              stkStackX[stkStackHeight] = oL;
              stkStackW[stkStackHeight] = oW;
              stkStackHeight++;
            } else {
              for (int i = 0; i < 9; i++) { stkStackX[i] = stkStackX[i+1]; stkStackW[i] = stkStackW[i+1]; }
              stkStackX[9] = oL; stkStackW[9] = oW;
            }
            stkScore  += 10;
            stkSpeed  += 0.25f;
            stkFlashNew = true;
            audio.playSound(SOUND_COIN);
          }
        }
      } else {
        stkFlashNew = false;
      }
    } else {
      if (btn1 && !lastBtn1) resetStacker();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

    // ── Background: neon violet grid ─────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H, 0x1804);
    for (int y = G_Y; y < G_B; y += blockH + 2)
      display.drawFastHLine(G_X, y, G_W, 0x2104);
    // Vertical guide line
    display.drawFastVLine(G_X + G_W/2, G_Y, G_H, 0x2905);

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
      uint16_t topCol = (i == stkStackHeight-1 && stkFlashNew) ? TFT_WHITE : themeAccent;
      uint16_t botCol = (i == stkStackHeight-1 && stkFlashNew) ? 0xC000   : blend565(themeAccent, 0x0000);
      display.fillRoundRect((int)stkStackX[i], blockY, (int)stkStackW[i], blockH, 2, topCol);
      display.fillRect     ((int)stkStackX[i], blockY + blockH/2, (int)stkStackW[i], blockH/2, botCol);
      display.drawRoundRect((int)stkStackX[i], blockY, (int)stkStackW[i], blockH, 2, TFT_WHITE);
      // Top highlight strip
      display.drawFastHLine((int)stkStackX[i]+2, blockY+1, (int)stkStackW[i]-4, blend565(topCol, TFT_WHITE));
    }

    // Moving active block
    if (!stkGameOver) {
      int curY = G_B - (blockH + 2) - stkStackHeight * (blockH + 2);
      unsigned long t = millis();
      uint16_t blockCol = (t/300)%2 ? TFT_YELLOW : 0xFFE0;
      display.fillRoundRect((int)stkBlockX, curY, (int)stkBlockW, blockH, 2, blockCol);
      display.drawRoundRect((int)stkBlockX, curY, (int)stkBlockW, blockH, 2, TFT_WHITE);
      display.drawFastHLine((int)stkBlockX+2, curY+1, (int)stkBlockW-4, TFT_WHITE);
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("STCK:%03d", stkScore);
    display.setCursor(SCREEN_WIDTH-(SCREEN_WIDTH==240?90:54), SCREEN_WIDTH==240?36:7);
    display.printf("HI:%03d", stkHighScore);

    if (stkGameOver) drawGameOverScreen(display, stkScore, stkHighScore);
  }

  // ============================================================
  //  GAME 7: MEMORY MATRIX
  // ============================================================
  void updateAndDrawMemory(GFXcanvas16& display, LunaAudio& audio) {
    const uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;

    const bool btn1 = virtualBtn1;
    const bool btn2 = virtualBtn2;

    const int gridSpacing = SCREEN_WIDTH==240?42:28;
    const int gridStartX  = G_X + (G_W - 3*gridSpacing)/2;
    const int gridStartY  = G_Y + (G_H - 3*gridSpacing)/2;

    if (!memGameOver) {
      if (memState == 0) {
        unsigned long elapsed = millis() - memTimer;
        const int stepTime  = 600;
        const int gapTime   = 200;
        const int cycleTime = stepTime + gapTime;
        int currentStep = elapsed / cycleTime;
        if (currentStep < memSeqLen) {
          memSeqStep = ((elapsed % cycleTime) < (unsigned long)stepTime) ? memSeq[currentStep] : -1;
        } else {
          memState = 1; memPlayerStep = 0; memSeqStep = -1;
        }
      } else if (memState == 1) {
        if (btn1 && !lastBtn1) { memSelectedTile = (memSelectedTile + 1) % 9; audio.playSound(SOUND_CHIRP); }
        if (btn2 && !lastBtn2) {
          if (memSelectedTile == memSeq[memPlayerStep]) {
            audio.playSound(SOUND_JUMP);
            if (++memPlayerStep == memSeqLen) {
              memScore += 10; memSeqLen++;
              if (memSeqLen > 15) memSeqLen = 15;
              memSeq[memSeqLen - 1] = random(0, 9);
              memState = 0; memTimer = millis();
            }
          } else {
            memGameOver = true; audio.playSound(SOUND_GAMEOVER);
            saveHighScore("mem", memHighScore, memScore);
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) resetMemory();
    }
    lastBtn1 = btn1; lastBtn2 = btn2;

    // ── Background: deep cyber blue ───────────────────────────────────────
    display.fillRect(G_X, G_Y, G_W, G_H, 0x000F);
    // Subtle grid lines
    for (int gx = gridStartX - 4; gx <= gridStartX + 3*gridSpacing; gx += gridSpacing)
      display.drawFastVLine(gx, gridStartY, 3*gridSpacing, 0x0841);
    for (int gy = gridStartY - 4; gy <= gridStartY + 3*gridSpacing; gy += gridSpacing)
      display.drawFastHLine(gridStartX, gy, 3*gridSpacing, 0x0841);

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
          tileColor = TFT_YELLOW; isActive = true;
        } else if (memState == 1 && memSelectedTile == tileIdx) {
          tileColor = themeAccent; isActive = true;
        }

        display.fillRoundRect(tx, ty, tw, th, 5, tileColor);
        display.drawRoundRect(tx, ty, tw, th, 5, isActive ? TFT_WHITE : 0x18C3);
        // Extra glow ring for active tile
        if (isActive) {
          display.drawRoundRect(tx - 1, ty - 1, tw + 2, th + 2, 6, blend565(tileColor, TFT_WHITE));
        }
        // Tile number dimly
        if (!isActive) {
          display.setTextSize(1); display.setTextColor(0x18C3);
          display.setCursor(tx + tw/2 - 3, ty + th/2 - 4);
          display.print(tileIdx + 1);
        }
      }
    }

    // Progress dots below grid (show how far into input sequence)
    if (memState == 1 && memSeqLen > 0) {
      int dotY = gridStartY + 3*gridSpacing + 8;
      int dotW = min(memSeqLen, 15) * 8;
      int dotX = G_X + (G_W - dotW)/2;
      for (int d = 0; d < min(memSeqLen, 15); d++) {
        uint16_t dc = (d < memPlayerStep) ? 0x07E0 : (d == memPlayerStep ? TFT_YELLOW : 0x4208);
        display.fillCircle(dotX + d*8 + 3, dotY + 3, 3, dc);
      }
    }

    // HUD
    display.fillRect(0, 0, SCREEN_WIDTH, G_Y, 0x0841);
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeAccent);
    display.setTextSize(SCREEN_WIDTH==240?2:1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(6, SCREEN_WIDTH==240?36:7);
    display.printf("SC:%03d", memScore);

    // Phase badge
    {
      int badgeX = SCREEN_WIDTH/2 - 30, badgeY = SCREEN_WIDTH==240?30:4;
      uint16_t badgeCol = (memState == 0) ? 0xF800 : 0x03E0;
      display.fillRoundRect(badgeX, badgeY, 60, SCREEN_WIDTH==240?16:12, 4, badgeCol);
      display.setTextSize(1); display.setTextColor(TFT_WHITE);
      display.setCursor(badgeX + 4, badgeY + (SCREEN_WIDTH==240?4:2));
      display.print(memState == 0 ? " WATCH  " : "  INPUT ");
    }

    if (memState == 1 && !memGameOver) {
      display.setTextSize(1); display.setTextColor(0x4A49);
      display.setCursor(SCREEN_WIDTH-(SCREEN_WIDTH==240?100:60), SCREEN_WIDTH==240?36:7);
      display.printf("S%d/%d", memPlayerStep+1, memSeqLen);
    }

    if (memGameOver) drawGameOverScreen(display, memScore, memHighScore);
  }
};

#endif // GAMES_H
