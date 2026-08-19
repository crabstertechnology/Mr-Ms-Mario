#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Preferences.h>
#include "config.h"
#include "audio.h"

// External references
extern bool gamesActive;
extern bool gamePlaying;
extern int gameMenuOption;
extern int gameSelected;

class LunaGames {
private:
  // High scores
  int advHighScore;
  int racHighScore;
  int spcHighScore;

  // --- Game 1: Luna Adventure State ---
  float advPlayerX;
  float advPlayerY;
  float advPlayerVX;
  float advPlayerVY;
  bool advIsJumping;
  int advLives;
  int advScore;
  bool advGameOver;
  int advPowerState; // 0: Normal, 1: Super, 2: Fire
  float advCameraX;
  unsigned long advInvincibleTime;
  int advLevelState; // 0: Play, 1: Flag slide, 2: Walk to castle, 3: Stage Clear screen
  int advCurrentLevel; // 1, 2, 3
  unsigned long advLevelTransitionTimer;
  int advWalkFrame;
  int advMoveState; // -1 = backward, 0 = stopped, 1 = forward
  unsigned long advBtn1PressTime;
  bool advBtn1Active;
  bool advLastBtn1;

  // Level platforms
  int advNumPlats;
  float advPlatX[8];
  float advPlatY[8];
  float advPlatW[8];
  float advPlatH[8];

  // Level blocks
  float advBlockX[6];
  float advBlockY[6];
  int advBlockType[6]; // 0: Brick, 1: ? Block
  int advBlockItem[6]; // 0: Coin, 1: Mushroom, 2: Fire Flower
  bool advBlockHit[6];
  bool advBlockActive[6];

  // Coins
  float advCoinX[6];
  float advCoinY[6];
  bool advCoinActive[6];

  // Enemies
  float advEnemyX[4];
  float advEnemyY[4];
  int advEnemyType[4]; // 0: Goomba, 1: Koopa, 2: Winged Koopa
  bool advEnemyActive[4];
  int advEnemyDir[4];
  float advEnemyVY[4];

  // Mushroom / Fire Flower power-up item
  float advMushX;
  float advMushY;
  float advMushVX;
  int advMushType; // 1: Mushroom, 2: Fire Flower
  bool advMushActive;

  // Player Fireballs
  float advFireballX[3];
  float advFireballY[3];
  float advFireballVX[3];
  float advFireballVY[3];
  bool advFireballActive[3];
  unsigned long advLastFireTime;

  // Boss Bowser
  bool advBossActive;
  int advBossHP;
  float advBossX;
  float advBossY;
  int advBossDir;
  unsigned long advBossLastJump;
  unsigned long advBossLastFire;
  float advBossFireX[3];
  float advBossFireY[3];
  float advBossFireVX[3];
  bool advBossFireActive[3];
  bool advBridgeCollapsed;

  // --- Game 2: Luna Racer State ---
  float racPlayerX;
  float racPlayerY;
  float racSpeed;
  bool racNitroActive;
  float racNitroFuel;
  float racRoadScroll;
  int racScore;
  bool racGameOver;
  
  // Traffic Cars
  float racCarX[4];
  float racCarY[4];
  float racCarSpeed[4];
  int racCarColor[4];
  bool racCarActive[4];

  // --- Game 3: Luna Space State ---
  float spcPlayerX;
  float spcPlayerY;
  int spcScore;
  int spcHealth;
  bool spcGameOver;
  unsigned long spcLastShoot;
  unsigned long spcShieldTime;
  unsigned long spcSmartBombCooldown;
  
  // Lasers
  float spcLaserX[6];
  float spcLaserY[6];
  bool spcLaserActive[6];
  
  // Enemies
  float spcEnemyX[8];
  float spcEnemyY[8];
  bool spcEnemyActive[8];
  int spcEnemyHP[8];
  int spcEnemyType[8];
  unsigned long spcEnemyLastShoot[8];
  
  // Enemy lasers
  float spcEnemyLaserX[4];
  float spcEnemyLaserY[4];
  bool spcEnemyLaserActive[4];
  
  // Particles
  float spcPartX[15];
  float spcPartY[15];
  float spcPartVX[15];
  float spcPartVY[15];
  int spcPartLife[15];
  bool spcPartActive[15];
  
  // Powerups
  float spcPowerX;
  float spcPowerY;
  int spcPowerType;
  bool spcPowerActive;
  bool spcSpreadActive;
  
  // Boss Spacecraft
  bool spcBossActive;
  int spcBossHP;
  float spcBossX;
  float spcBossY;
  int spcBossDir;
  unsigned long spcBossLastShoot;

  // Buttons input helper
  bool lastBtn1;
  bool lastBtn2;

public:
  LunaGames() {
    advHighScore = 0;
    racHighScore = 0;
    spcHighScore = 0;
    advLives = 3;
    advScore = 0;
    advCurrentLevel = 1;
    resetAdventure();
    resetRacer();
    resetSpace();
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
    advHighScore = prefs.getInt("adv", 0);
    racHighScore = prefs.getInt("rac", 0);
    spcHighScore = prefs.getInt("spc", 0);
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

  void resetAdventureForLevel() {
    advPlayerX = 20;
    advPlayerY = G_B - 20;
    advPlayerVX = 0;
    advPlayerVY = 0;
    advIsJumping = false;
    advCameraX = 0;
    advLevelState = 0;
    advLastFireTime = 0;
    advBridgeCollapsed = false;
    advInvincibleTime = 0;
    advWalkFrame = 0;
    advMoveState = 1;
    advBtn1Active = false;
    advBtn1PressTime = 0;
    advLastBtn1 = false;
    
    for (int i = 0; i < 3; i++) {
      advFireballActive[i] = false;
      advBossFireActive[i] = false;
    }
    
    advMushActive = false;
    
    if (advCurrentLevel == 1) {
      // Level 1: Forest
      advNumPlats = 5;
      advPlatX[0] = 0; advPlatY[0] = G_B - 14; advPlatW[0] = 180; advPlatH[0] = 14;
      advPlatX[1] = 215; advPlatY[1] = G_B - 14; advPlatW[1] = 85; advPlatH[1] = 14;
      advPlatX[2] = 330; advPlatY[2] = G_B - 14; advPlatW[2] = 150; advPlatH[2] = 14;
      advPlatX[3] = 100; advPlatY[3] = G_B - 40; advPlatW[3] = 40; advPlatH[3] = 8;
      advPlatX[4] = 230; advPlatY[4] = G_B - 45; advPlatW[4] = 40; advPlatH[4] = 8;
      
      // Blocks
      advBlockX[0] = 90; advBlockY[0] = G_B - 40; advBlockType[0] = 1; advBlockItem[0] = 0; advBlockHit[0] = false; advBlockActive[0] = true;
      advBlockX[1] = 110; advBlockY[1] = G_B - 40; advBlockType[1] = 0; advBlockItem[1] = 0; advBlockHit[1] = false; advBlockActive[1] = true;
      advBlockX[2] = 130; advBlockY[2] = G_B - 40; advBlockType[2] = 1; advBlockItem[2] = 1; advBlockHit[2] = false; advBlockActive[2] = true; // mushroom
      advBlockX[3] = 240; advBlockY[3] = G_B - 45; advBlockType[3] = 0; advBlockItem[3] = 0; advBlockHit[3] = false; advBlockActive[3] = true;
      advBlockX[4] = 260; advBlockY[4] = G_B - 45; advBlockType[4] = 1; advBlockItem[4] = 0; advBlockHit[4] = false; advBlockActive[4] = true;
      advBlockX[5] = 280; advBlockY[5] = G_B - 45; advBlockType[5] = 0; advBlockItem[5] = 0; advBlockHit[5] = false; advBlockActive[5] = true;
      
      // Coins
      for (int i = 0; i < 4; i++) {
        advCoinX[i] = 80 + i * 90;
        advCoinY[i] = G_B - 40 - (i % 2) * 20;
        advCoinActive[i] = true;
      }
      for (int i = 4; i < 6; i++) advCoinActive[i] = false;
      
      // Enemies
      advEnemyX[0] = 140; advEnemyY[0] = G_B - 14; advEnemyType[0] = 0; advEnemyActive[0] = true; advEnemyDir[0] = -1; advEnemyVY[0] = 0;
      advEnemyX[1] = 225; advEnemyY[1] = G_B - 14; advEnemyType[1] = 0; advEnemyActive[1] = true; advEnemyDir[1] = -1; advEnemyVY[1] = 0;
      advEnemyX[2] = 270; advEnemyY[2] = G_B - 14; advEnemyType[2] = 0; advEnemyActive[2] = true; advEnemyDir[2] = -1; advEnemyVY[2] = 0;
      advEnemyX[3] = 350; advEnemyY[3] = G_B - 14; advEnemyType[3] = 0; advEnemyActive[3] = true; advEnemyDir[3] = -1; advEnemyVY[3] = 0;
      
      advBossActive = false;
    } 
    else if (advCurrentLevel == 2) {
      // Level 2: Sky
      advNumPlats = 5;
      advPlatX[0] = 0; advPlatY[0] = G_B - 20; advPlatW[0] = 60; advPlatH[0] = 8;
      advPlatX[1] = 90; advPlatY[1] = G_B - 40; advPlatW[1] = 60; advPlatH[1] = 8;
      advPlatX[2] = 180; advPlatY[2] = G_B - 60; advPlatW[2] = 60; advPlatH[2] = 8;
      advPlatX[3] = 270; advPlatY[3] = G_B - 40; advPlatW[3] = 60; advPlatH[3] = 8;
      advPlatX[4] = 350; advPlatY[4] = G_B - 20; advPlatW[4] = 100; advPlatH[4] = 14;
      
      // Blocks
      advBlockX[0] = 100; advBlockY[0] = G_B - 65; advBlockType[0] = 1; advBlockItem[0] = 2; advBlockHit[0] = false; advBlockActive[0] = true; // fire flower
      advBlockX[1] = 120; advBlockY[1] = G_B - 65; advBlockType[1] = 0; advBlockItem[1] = 0; advBlockHit[1] = false; advBlockActive[1] = true;
      advBlockX[2] = 190; advBlockY[2] = G_B - 85; advBlockType[2] = 0; advBlockItem[2] = 0; advBlockHit[2] = false; advBlockActive[2] = true;
      advBlockX[3] = 210; advBlockY[3] = G_B - 85; advBlockType[3] = 1; advBlockItem[3] = 0; advBlockHit[3] = false; advBlockActive[3] = true;
      advBlockX[4] = 280; advBlockY[4] = G_B - 65; advBlockType[4] = 0; advBlockItem[4] = 0; advBlockHit[4] = false; advBlockActive[4] = true;
      advBlockX[5] = 300; advBlockY[5] = G_B - 65; advBlockType[5] = 1; advBlockItem[5] = 0; advBlockHit[5] = false; advBlockActive[5] = true;
      
      // Coins
      for (int i = 0; i < 6; i++) {
        advCoinX[i] = 100 + i * 50;
        advCoinY[i] = G_B - 50 - (i % 2) * 15;
        advCoinActive[i] = true;
      }
      
      // Enemies
      advEnemyX[0] = 110; advEnemyY[0] = G_B - 40; advEnemyType[0] = 0; advEnemyActive[0] = true; advEnemyDir[0] = -1; advEnemyVY[0] = 0;
      advEnemyX[1] = 200; advEnemyY[1] = G_B - 60; advEnemyType[1] = 1; advEnemyActive[1] = true; advEnemyDir[1] = -1; advEnemyVY[1] = 0;
      advEnemyX[2] = 290; advEnemyY[2] = G_B - 40; advEnemyType[2] = 2; advEnemyActive[2] = true; advEnemyDir[2] = -1; advEnemyVY[2] = 0; // Winged
      advEnemyX[3] = 360; advEnemyY[3] = G_B - 20; advEnemyType[3] = 1; advEnemyActive[3] = true; advEnemyDir[3] = -1; advEnemyVY[3] = 0;
      
      advBossActive = false;
    } 
    else {
      // Level 3: Bowser's Castle
      advNumPlats = 5;
      advPlatX[0] = 0; advPlatY[0] = G_B - 14; advPlatW[0] = 120; advPlatH[0] = 14;
      advPlatX[1] = 140; advPlatY[1] = G_B - 35; advPlatW[1] = 80; advPlatH[1] = 10;
      advPlatX[2] = 240; advPlatY[2] = G_B - 14; advPlatW[2] = 110; advPlatH[2] = 14; // Bridge
      advPlatX[3] = 350; advPlatY[3] = G_B - 14; advPlatW[3] = 100; advPlatH[3] = 14; // golden axe
      advPlatX[4] = 80; advPlatY[4] = G_B - 60; advPlatW[4] = 60; advPlatH[4] = 8;
      
      // Blocks
      advBlockX[0] = 70; advBlockY[0] = G_B - 35; advBlockType[0] = 0; advBlockItem[0] = 0; advBlockHit[0] = false; advBlockActive[0] = true;
      advBlockX[1] = 160; advBlockY[1] = G_B - 55; advBlockType[1] = 1; advBlockItem[1] = 2; advBlockHit[1] = false; advBlockActive[1] = true; // fire flower
      advBlockX[2] = 200; advBlockY[2] = G_B - 55; advBlockType[2] = 0; advBlockItem[2] = 0; advBlockHit[2] = false; advBlockActive[2] = true;
      advBlockX[3] = 210; advBlockY[3] = G_B - 55; advBlockType[3] = 0; advBlockItem[3] = 0; advBlockHit[3] = false; advBlockActive[3] = true;
      advBlockX[4] = 270; advBlockY[4] = G_B - 45; advBlockType[4] = 0; advBlockItem[4] = 0; advBlockHit[4] = false; advBlockActive[4] = true;
      advBlockX[5] = 290; advBlockY[5] = G_B - 45; advBlockType[5] = 0; advBlockItem[5] = 0; advBlockHit[5] = false; advBlockActive[5] = true;
      
      // Coins
      for (int i = 0; i < 4; i++) {
        advCoinX[i] = 140 + i * 20;
        advCoinY[i] = G_B - 55;
        advCoinActive[i] = true;
      }
      for (int i = 4; i < 6; i++) advCoinActive[i] = false;
      
      // Enemies
      advEnemyX[0] = 80; advEnemyY[0] = G_B - 14; advEnemyType[0] = 1; advEnemyActive[0] = true; advEnemyDir[0] = -1; advEnemyVY[0] = 0;
      advEnemyX[1] = 150; advEnemyY[1] = G_B - 35; advEnemyType[1] = 0; advEnemyActive[1] = true; advEnemyDir[1] = -1; advEnemyVY[1] = 0;
      advEnemyX[2] = 200; advEnemyY[2] = G_B - 35; advEnemyType[2] = 0; advEnemyActive[2] = true; advEnemyDir[2] = -1; advEnemyVY[2] = 0;
      advEnemyX[3] = 250; advEnemyY[3] = G_B - 14; advEnemyType[3] = 1; advEnemyActive[3] = true; advEnemyDir[3] = -1; advEnemyVY[3] = 0;
      
      // Bowser
      advBossActive = true;
      advBossHP = 5;
      advBossX = 290;
      advBossY = G_B - 24;
      advBossDir = -1;
      advBossLastJump = millis();
      advBossLastFire = millis();
    }
  }

  void resetAdventure() {
    advGameOver = false;  // CRITICAL: clear game-over flag on every reset
    advLives = 3;
    advScore = 0;
    advCurrentLevel = 1;
    advPowerState = 0;
    resetAdventureForLevel();
  }

  void resetRacer() {
    racPlayerX = G_X + G_W / 2;
    racPlayerY = G_B - 20;
    racSpeed = 3.0f;
    racNitroActive = false;
    racNitroFuel = 100.0f;
    racRoadScroll = 0;
    racScore = 0;
    racGameOver = false;
    
    for (int i = 0; i < 4; i++) {
      racCarX[i] = G_X + 24 + random(0, 3) * (G_W - 48)/3;
      racCarY[i] = G_Y - 30 - i * 60;
      racCarSpeed[i] = 1.5f + random(0, 20) / 10.0f;
      racCarColor[i] = i % 3;
      racCarActive[i] = true;
    }
  }

  void resetSpace() {
    spcPlayerX = G_X + G_W / 2;
    spcPlayerY = G_B - 14;
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

  bool canExitActiveGame() {
    if (gameSelected == 1) return advGameOver;
    if (gameSelected == 2) return racGameOver;
    if (gameSelected == 3) return spcGameOver;
    return false;
  }

  void drawGameOverScreen(GFXcanvas16& display, int score, int highScore) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg  = TFT_WHITE;
    uint16_t themeText = 0x2104;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);

    display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
    display.setTextColor(TFT_RED);
    display.setCursor((SCREEN_WIDTH - 9 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, SCREEN_WIDTH == 240 ? 65 : 48);
    display.print("GAME OVER");

    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "SC:%d HI:%d", score, highScore);
    int scoreW = strlen(scoreBuf) * (SCREEN_WIDTH == 240 ? 12 : 6);
    display.setCursor((SCREEN_WIDTH - scoreW) / 2, SCREEN_WIDTH == 240 ? 115 : 80);
    display.print(scoreBuf);

    display.setTextColor(themeAccent);
    display.setCursor((SCREEN_WIDTH - 13 * (SCREEN_WIDTH == 240 ? 12 : 6)) / 2, SCREEN_WIDTH == 240 ? 160 : 108);
    display.print("B1:PLAY AGAIN");
    display.setCursor((SCREEN_WIDTH - 14 * (SCREEN_WIDTH == 240 ? 12 : 6)) / 2, SCREEN_WIDTH == 240 ? 190 : 124);
    display.print("B2:BACK 2 MENU");
  }

  void drawMenu(GFXcanvas16& display) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xD69A;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);

    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeAccent);
    display.setCursor((SCREEN_WIDTH - 11 * (SCREEN_WIDTH == 240 ? 12 : 6)) / 2, SCREEN_WIDTH == 240 ? 30 : 12);
    display.print("LUNA ARCADE");
    display.drawFastHLine(6, SCREEN_WIDTH == 240 ? 54 : 24, SCREEN_WIDTH - 12, themeBorder);

    const char* gameNames[] = {
      "1.LUNA ADVENTURE",
      "2.LUNA RACER",
      "3.LUNA SPACE",
      "4.EXIT ARCADE"
    };

    int spacing = SCREEN_WIDTH == 240 ? 32 : 22;
    int startY = SCREEN_WIDTH == 240 ? 66 : 32;

    for (int idx = 0; idx < 4; idx++) {
      int yPos = startY + idx * spacing;
      bool sel = (gameMenuOption == idx);
      
      if (sel) {
        display.fillRoundRect(6, yPos, SCREEN_WIDTH - 24, SCREEN_WIDTH == 240 ? 28 : 18, 4, themeCardBg);
        display.drawRoundRect(6, yPos, SCREEN_WIDTH - 24, SCREEN_WIDTH == 240 ? 28 : 18, 4, themeAccent);
        display.setTextColor(themeAccent);
      } else {
        display.setTextColor(themeText);
      }
      
      display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
      display.setCursor(14, yPos + (SCREEN_WIDTH == 240 ? 6 : 5));
      display.print(gameNames[idx]);
    }

    display.setTextSize(1);
    display.setTextColor(0x7BCF);
    display.setCursor(8, SCREEN_HEIGHT - 14);
    display.print("B1:Scroll  B2:Play");
  }

  void updateAndDrawAdventure(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    bool btn2 = (digitalRead(BTN_SETTINGS_PIN) == LOW);
    int pHeight = advPowerState > 0 ? 14 : 10;

    // Dynamic background color
    uint16_t skyColor = 0x7E5F; // Sky blue for Forest
    if (advCurrentLevel == 2) skyColor = 0x000F; // Dark blue/violet for Sky
    else if (advCurrentLevel == 3) skyColor = 0x1800; // Dark castle grey

    if (!advGameOver) {
      if (advLevelState == 0) { // Playing mode
        // Jump action (Button B / Btn 2 / BTN_SETTINGS_PIN)
        if (btn2 && !advIsJumping) {
          advPlayerVY = -6.2f;
          advIsJumping = true;
          audio.playSound(SOUND_JUMP);
        }

        // Input Handling for Button 1 (A / BTN_EXPR_PIN): Tap to Stop/Forward, Long Press to Move Left
        if (btn1) {
          if (!advLastBtn1) { // Just pressed
            advBtn1PressTime = millis();
            advBtn1Active = true;
          } else if (advBtn1Active && (millis() - advBtn1PressTime > 400)) { // Long Press threshold met (400ms)
            advMoveState = -1; // Move backward (left)
            advBtn1Active = false; // Consume long press event
          }
        } else { // Released
          if (advLastBtn1) { // Just released
            if (advBtn1Active) { // Released before 400ms threshold -> Tap event
              if (advMoveState != 0) {
                advMoveState = 0; // Stop
              } else {
                advMoveState = 1; // Move forward (right)
              }
            }
            advBtn1Active = false;
          }
        }
        advLastBtn1 = btn1;

        // Apply movement velocity based on move state
        if (advMoveState == 1) {
          advPlayerVX = 1.1f; // Moderate speed
          advWalkFrame = (millis() / 150) % 2;
        } else if (advMoveState == -1) {
          advPlayerVX = -1.1f; // Move backward
          advWalkFrame = (millis() / 150) % 2;
        } else {
          advPlayerVX = 0.0f; // Stopped
          advWalkFrame = 0;
        }

        // Auto fire fireballs if player is in Fire state and moving
        if (advPowerState == 2 && advMoveState != 0 && (millis() - advLastFireTime > 500)) {
          advLastFireTime = millis();
          for (int i = 0; i < 3; i++) {
            if (!advFireballActive[i]) {
              advFireballActive[i] = true;
              advFireballX[i] = advCameraX + advPlayerX + 6;
              advFireballY[i] = advPlayerY - (advPowerState > 0 ? 10 : 6);
              advFireballVX[i] = (advMoveState < 0) ? -4.0f : 4.0f;
              advFireballVY[i] = 1.0f;
              audio.playSound(SOUND_CHIRP);
              break;
            }
          }
        }

        // Apply player gravity
        advPlayerVY += 0.4f;
        advPlayerY += advPlayerVY;

        // Platform collisions
        float absPlayerX = advCameraX + advPlayerX;
        bool landed = false;
        
        for (int i = 0; i < advNumPlats; i++) {
          // If bridge is collapsed, skip bridge platform (index 2 in Level 3)
          if (advCurrentLevel == 3 && i == 2 && advBridgeCollapsed) continue;

          float py = advPlatY[i];
          float px1 = advPlatX[i];
          float px2 = advPlatX[i] + advPlatW[i];

          // Check if falling onto the top of platform
          if (absPlayerX + 4 >= px1 && absPlayerX - 4 <= px2) {
            if (advPlayerVY >= 0 && advPlayerY >= py && advPlayerY - advPlayerVY <= py + 4) {
              advPlayerY = py;
              advPlayerVY = 0;
              advIsJumping = false;
              landed = true;
            }
          }
        }

        // Fall in pit / lava (deadly bounds)
        if (advPlayerY > G_B + 10) {
          audio.playSound(SOUND_POWERDOWN);
          advLives--;
          if (advLives <= 0) {
            advGameOver = true;
            audio.playSound(SOUND_GAMEOVER);
            saveHighScore("adv", advHighScore, advScore);
          } else {
            resetAdventureForLevel();
          }
        }

        // Hit blocks from underneath
        if (advPlayerVY < 0) {
          for (int i = 0; i < 6; i++) {
            if (advBlockActive[i]) {
              float bx1 = advBlockX[i];
              float bx2 = advBlockX[i] + 12;
              float by = advBlockY[i];
              
              if (absPlayerX + 3 >= bx1 && absPlayerX - 3 <= bx2) {
                if (advPlayerY - pHeight <= by + 12 && advPlayerY - pHeight - advPlayerVY >= by + 6) {
                  advPlayerVY = 0.5f; // Bounce back down
                  audio.playSound(SOUND_CHIRP);

                  if (advBlockType[i] == 0) { // Brick block
                    if (advPowerState > 0) { // Super/Fire breaks it
                      advBlockActive[i] = false;
                      advScore += 50;
                    }
                  } 
                  else if (advBlockType[i] == 1 && !advBlockHit[i]) { // ? block
                    advBlockHit[i] = true;
                    if (advBlockItem[i] == 0) { // Coin
                      advScore += 10;
                      audio.playSound(SOUND_COIN);
                    } else { // Mushroom or Fire Flower
                      advMushActive = true;
                      advMushX = advBlockX[i];
                      advMushY = advBlockY[i] - 12;
                      advMushVX = 1.0f;
                      advMushType = advBlockItem[i];
                    }
                  }
                }
              }
            }
          }
        }

        // Camera follow (scrolls in both directions)
        advCameraX += advPlayerVX;
        if (advCameraX < 0) {
          advCameraX = 0;
        }
        if (advCameraX >= 400.0f) {
          advCameraX = 400.0f;
        }

        // Flagpole intersection (Level Win triggers at X = 370)
        if (absPlayerX >= 370.0f) {
          advLevelState = 1;
          advPlayerVX = 0;
          audio.playSound(SOUND_POWERUP);
        }

        // Update Mushroom/Power-up physics
        if (advMushActive) {
          if (advMushType == 1) { // Mushroom moves
            advMushX += advMushVX;
            // Check platform floor for Mushroom
            bool mushOnGround = false;
            for (int i = 0; i < advNumPlats; i++) {
              if (advMushX >= advPlatX[i] && advMushX <= advPlatX[i] + advPlatW[i]) {
                if (advMushY >= advPlatY[i] - 12 && advMushY <= advPlatY[i]) {
                  advMushY = advPlatY[i] - 12;
                  mushOnGround = true;
                }
              }
            }
            if (!mushOnGround) advMushY += 1.5f;

            // Reverse direction on block obstacle
            for (int i = 0; i < 6; i++) {
              if (advBlockActive[i] && abs(advMushX - advBlockX[i]) < 12 && abs(advMushY - advBlockY[i]) < 8) {
                advMushVX = -advMushVX;
              }
            }
          }

          // Player eats power-up item
          if (abs(absPlayerX - (advMushX + 6)) < 12 && abs(advPlayerY - (advMushY + 6)) < 14) {
            advMushActive = false;
            audio.playSound(SOUND_POWERUP);
            if (advMushType == 1) { // Mushroom
              if (advPowerState == 0) advPowerState = 1;
              advScore += 100;
            } else { // Fire Flower
              advPowerState = 2;
              advScore += 200;
            }
          }

          if (advMushX - advCameraX < G_X - 10 || advMushX - advCameraX > G_R + 10) {
            advMushActive = false;
          }
        }

        // Update Player Fireballs
        for (int i = 0; i < 3; i++) {
          if (advFireballActive[i]) {
            advFireballX[i] += advFireballVX[i];
            advFireballVY[i] += 0.3f;
            advFireballY[i] += advFireballVY[i];

            // Platform collision for fireballs
            for (int p = 0; p < advNumPlats; p++) {
              if (advFireballX[i] >= advPlatX[p] && advFireballX[i] <= advPlatX[p] + advPlatW[p]) {
                if (advFireballVY[i] >= 0 && advFireballY[i] >= advPlatY[p] - 4 && advFireballY[i] <= advPlatY[p] + 4) {
                  advFireballY[i] = advPlatY[p] - 4;
                  advFireballVY[i] = -2.5f; // bounce
                }
              }
            }

            // Fireball hits Enemy
            for (int e = 0; e < 4; e++) {
              if (advEnemyActive[e]) {
                if (abs(advFireballX[i] - advEnemyX[e]) < 12 && abs(advFireballY[i] - (advEnemyY[e] - 6)) < 10) {
                  advFireballActive[i] = false;
                  advEnemyActive[e] = false;
                  advScore += 100;
                  audio.playSound(SOUND_COIN);
                }
              }
            }

            // Fireball hits Bowser Boss
            if (advBossActive && abs(advFireballX[i] - advBossX) < 16 && abs(advFireballY[i] - advBossY) < 16) {
              advFireballActive[i] = false;
              advBossHP--;
              audio.playSound(SOUND_JUMP);
              if (advBossHP <= 0) {
                advBossActive = false;
                advScore += 500;
                advLevelState = 1; // trigger win flagpole style transition
                audio.playSound(SOUND_POWERUP);
              }
            }

            if (advFireballX[i] - advCameraX > G_R || advFireballY[i] > G_B) {
              advFireballActive[i] = false;
            }
          }
        }

        // Update Coins
        for (int i = 0; i < 6; i++) {
          if (advCoinActive[i]) {
            if (abs(absPlayerX - advCoinX[i]) < 12 && abs(advPlayerY - advCoinY[i]) < 12) {
              advCoinActive[i] = false;
              advScore += 10;
              audio.playSound(SOUND_COIN);
            }
          }
        }

        // Update Enemies
        for (int i = 0; i < 4; i++) {
          if (advEnemyActive[i]) {
            advEnemyX[i] += advEnemyDir[i] * 0.7f;
            
            // Goomba & Koopa movement boundaries
            float spawnRange = 35.0f;
            float spawnX = 140 + i * 80;
            if (advCurrentLevel == 2) spawnX = 110 + i * 80;
            else if (advCurrentLevel == 3) spawnX = 80 + i * 60;
            
            if (advEnemyX[i] < spawnX - spawnRange) advEnemyDir[i] = 1;
            if (advEnemyX[i] > spawnX + spawnRange) advEnemyDir[i] = -1;

            // Winged Koopa hopping mechanics
            if (advEnemyType[i] == 2) {
              advEnemyVY[i] += 0.2f;
              advEnemyY[i] += advEnemyVY[i];
              bool enemyOnPlat = false;
              for (int p = 0; p < advNumPlats; p++) {
                if (advEnemyX[i] >= advPlatX[p] && advEnemyX[i] <= advPlatX[p] + advPlatW[p]) {
                  if (advEnemyY[i] >= advPlatY[p]) {
                    advEnemyY[i] = advPlatY[p];
                    enemyOnPlat = true;
                  }
                }
              }
              if (enemyOnPlat) advEnemyVY[i] = -3.0f; // Hop again
            }

            // Overlap check with player
            float screenEnemyX = advEnemyX[i] - advCameraX;
            if (screenEnemyX > G_X - 10 && screenEnemyX < G_R + 10) {
              if (abs(advPlayerX - screenEnemyX) < 10 && abs(advPlayerY - advEnemyY[i]) < 10) {
                // If landing on top of the enemy
                if (advPlayerVY > 0 && advPlayerY < advEnemyY[i] - 2) {
                  advEnemyActive[i] = false;
                  advPlayerVY = -3.5f;
                  advScore += 100;
                  audio.playSound(SOUND_COIN);
                } 
                // Getting hit by enemy
                else if (millis() > advInvincibleTime) {
                  if (advPowerState > 0) {
                    advPowerState--;
                    advInvincibleTime = millis() + 1500;
                    audio.playSound(SOUND_POWERDOWN);
                  } else {
                    audio.playSound(SOUND_POWERDOWN);
                    advLives--;
                    if (advLives <= 0) {
                      advGameOver = true;
                      audio.playSound(SOUND_GAMEOVER);
                      saveHighScore("adv", advHighScore, advScore);
                    } else {
                      resetAdventureForLevel();
                    }
                  }
                }
              }
            }
          }
        }

        // Update Boss Bowser in Level 3
        if (advBossActive && advCurrentLevel == 3) {
          // Move left/right on bridge
          advBossX += advBossDir * 0.7f;
          if (advBossX < 250) advBossDir = 1;
          if (advBossX > 320) advBossDir = -1;

          // Bridge axe collapse
          if (absPlayerX >= 350.0f && !advBridgeCollapsed) {
            advBridgeCollapsed = true;
            advBossActive = false;
            advLevelState = 1; // triggers flag slide/level clear transition
            audio.playSound(SOUND_GAMEOVER);
          }

          // Bowser jumps
          if (millis() - advBossLastJump > 2200) {
            advBossLastJump = millis();
            advBossY = G_B - 45;
          }
          if (advBossY < G_B - 24) {
            advBossY += 0.8f;
          } else {
            advBossY = G_B - 24;
          }

          // Bowser fire breathing
          if (millis() - advBossLastFire > 2000) {
            advBossLastFire = millis();
            for (int f = 0; f < 3; f++) {
              if (!advBossFireActive[f]) {
                advBossFireActive[f] = true;
                advBossFireX[f] = advBossX - 10;
                advBossFireY[f] = advBossY + random(-4, 5);
                advBossFireVX[f] = -2.2f;
                break;
              }
            }
          }

          // Update Bowser Fireballs
          for (int f = 0; f < 3; f++) {
            if (advBossFireActive[f]) {
              advBossFireX[f] += advBossFireVX[f];
              if (advBossFireX[f] < advCameraX) {
                advBossFireActive[f] = false;
              }
              // Hit player check
              float screenFireX = advBossFireX[f] - advCameraX;
              if (abs(advPlayerX - screenFireX) < 10 && abs(advPlayerY - advBossFireY[f]) < 10) {
                advBossFireActive[f] = false;
                if (millis() > advInvincibleTime) {
                  if (advPowerState > 0) {
                    advPowerState--;
                    advInvincibleTime = millis() + 1500;
                    audio.playSound(SOUND_POWERDOWN);
                  } else {
                    audio.playSound(SOUND_POWERDOWN);
                    advLives--;
                    if (advLives <= 0) {
                      advGameOver = true;
                      audio.playSound(SOUND_GAMEOVER);
                      saveHighScore("adv", advHighScore, advScore);
                    } else {
                      resetAdventureForLevel();
                    }
                  }
                }
              }
            }
          }

          // Bowser physical touch player
          float screenBossX = advBossX - advCameraX;
          if (abs(advPlayerX - screenBossX) < 14 && abs(advPlayerY - advBossY) < 16) {
            if (advPlayerVY > 0 && advPlayerY < advBossY - 4) {
              advBossHP--;
              advPlayerVY = -4.0f;
              audio.playSound(SOUND_JUMP);
              if (advBossHP <= 0) {
                advBossActive = false;
                advScore += 500;
                advLevelState = 1;
                audio.playSound(SOUND_POWERUP);
              }
            } else if (millis() > advInvincibleTime) {
              if (advPowerState > 0) {
                advPowerState--;
                advInvincibleTime = millis() + 1500;
                audio.playSound(SOUND_POWERDOWN);
              } else {
                audio.playSound(SOUND_POWERDOWN);
                advLives--;
                if (advLives <= 0) {
                  advGameOver = true;
                  audio.playSound(SOUND_GAMEOVER);
                  saveHighScore("adv", advHighScore, advScore);
                } else {
                  resetAdventureForLevel();
                }
              }
            }
          }
        }
      } 
      else if (advLevelState == 1) { // Flagpole slide animation
        advPlayerY += 1.2f;
        if (advPlayerY >= G_B - 14) {
          advPlayerY = G_B - 14;
          advLevelState = 2; // Start walking to castle door
        }
      } 
      else if (advLevelState == 2) { // Walking to Castle Door
        advPlayerX += 1.0f;
        advWalkFrame = (millis() / 120) % 2;
        if (advPlayerX >= 110) { // reaches castle entrance screen right
          advLevelState = 3;
          advLevelTransitionTimer = millis();
        }
      } 
      else if (advLevelState == 3) { // Delay before Stage transition
        if (millis() - advLevelTransitionTimer > 2000) {
          if (advCurrentLevel < 3) {
            advCurrentLevel++;
            resetAdventureForLevel();
          } else {
            // Defeated Level 3: Victory!
            advLives = 99;
            advGameOver = true;
            saveHighScore("adv", advHighScore, advScore);
          }
        }
      }
    } else {
      if (btn1 && !lastBtn1) {
        resetAdventure();
      }
    }
    lastBtn1 = btn1;
    lastBtn2 = btn2;

    // RENDER GAMEPLAY
    display.fillScreen(skyColor);
    
    // UI Header
    display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
    display.setTextColor(themeText);
    display.setCursor(6, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("WORLD 1-%d", advCurrentLevel);
    display.setCursor(SCREEN_WIDTH/2 - 25, SCREEN_WIDTH == 240 ? 36 : 7);
    display.printf("PTS:%04d", advScore);
    
    int lifeX = SCREEN_WIDTH - (SCREEN_WIDTH == 240 ? 54 : 32);
    for (int i = 0; i < 3; i++) {
      int hx = lifeX + i * (SCREEN_WIDTH == 240 ? 14 : 7);
      int hy = SCREEN_WIDTH == 240 ? 42 : 10;
      display.fillCircle(hx, hy, SCREEN_WIDTH == 240 ? 3 : 2, i < advLives ? TFT_RED : themeBorder);
    }
    display.drawFastHLine(0, G_Y - 1, SCREEN_WIDTH, themeBorder);

    if (!advGameOver && advLevelState < 3) {
      // 1. Drawing parallax clouds/hills (Forest/Sky only)
      if (advCurrentLevel < 3) {
        for (int i = 0; i < 3; i++) {
          // Clouds
          float cx = (i * 120) - ((int)(advCameraX * 0.3f) % 120);
          display.fillRoundRect(cx, G_Y + 10 + i * 8, 25, 8, 3, TFT_WHITE);
          
          // Hills (only in Level 1)
          if (advCurrentLevel == 1) {
            float hx = (i * 160) - ((int)(advCameraX * 0.6f) % 160);
            display.fillTriangle(hx, G_B - 14, hx + 30, G_B - 50, hx + 60, G_B - 14, 0x96E9); // green-blue hill
          }
        }
      } else { // Castle decorations
        display.fillRect(0, G_Y, SCREEN_WIDTH, G_H, 0x1800); // darker background
        // draw lava glow details
        for (int x = 0; x < SCREEN_WIDTH; x += 30) {
          display.fillTriangle(x, G_B - 10, x + 15, G_B - 25, x + 30, G_B - 10, TFT_ORANGE);
        }
      }

      // 2. Draw platforms
      for (int i = 0; i < advNumPlats; i++) {
        if (advCurrentLevel == 3 && i == 2 && advBridgeCollapsed) continue; // collapsed bridge disappears

        float sx = advPlatX[i] - advCameraX;
        if (sx + advPlatW[i] >= 0 && sx <= G_R) {
          uint16_t platCol = (advCurrentLevel == 3) ? 0x50C6 : (advCurrentLevel == 2 ? TFT_WHITE : 0x7240); // dark grey/cloud/dirt
          display.fillRect(sx, advPlatY[i], advPlatW[i], advPlatH[i], platCol);
          display.drawRect(sx, advPlatY[i], advPlatW[i], advPlatH[i], themeText);
        }
      }

      // 3. Draw blocks
      for (int i = 0; i < 6; i++) {
        if (advBlockActive[i]) {
          float sx = advBlockX[i] - advCameraX;
          if (sx + 12 >= 0 && sx <= G_R) {
            if (advBlockType[i] == 0) { // Brick block
              display.fillRect(sx, advBlockY[i], 12, 12, 0xB269); // brown-red brick color
              display.drawRect(sx, advBlockY[i], 12, 12, themeText);
              display.drawFastHLine(sx, advBlockY[i]+4, 12, themeText);
              display.drawFastHLine(sx, advBlockY[i]+8, 12, themeText);
            } else { // Question mark block
              display.fillRect(sx, advBlockY[i], 12, 12, advBlockHit[i] ? 0x7BEF : TFT_YELLOW);
              display.drawRect(sx, advBlockY[i], 12, 12, themeText);
              if (!advBlockHit[i]) {
                display.setCursor(sx + 4, advBlockY[i] + 2);
                display.setTextColor(themeText);
                display.setTextSize(1);
                display.print("?");
              }
            }
          }
        }
      }

      // 4. Draw flagpole and castle (Level 1 and 2 end)
      float sFlagX = 370 - advCameraX;
      if (sFlagX >= -30 && sFlagX <= G_R) {
        display.drawFastVLine(sFlagX, G_Y + 10, G_H - 24, themeText); // pole
        display.fillCircle(sFlagX, G_Y + 10, 3, TFT_YELLOW); // golden ball top
        
        // sliding flag
        float flagY = G_Y + 15;
        if (advLevelState == 1) flagY = advPlayerY - pHeight;
        display.fillTriangle(sFlagX - 10, flagY, sFlagX, flagY - 5, sFlagX, flagY + 5, TFT_RED);
      }

      // Draw castle at end
      float sCastleX = 400 - advCameraX;
      if (sCastleX >= -40 && sCastleX <= G_R) {
        display.fillRect(sCastleX, G_B - 40, 40, 26, 0x50C6); // grey castle wall
        display.fillRect(sCastleX + 14, G_B - 20, 12, 20, themeText); // black castle door entrance
        display.drawRect(sCastleX, G_B - 40, 40, 26, themeText);
      }

      // 5. Draw Active Mushroom/Powerup
      if (advMushActive) {
        float smx = advMushX - advCameraX;
        if (smx >= -12 && smx <= G_R) {
          if (advMushType == 1) { // Mushroom
            display.fillCircle(smx + 6, advMushY + 6, 5, TFT_RED);
            display.fillRect(smx + 4, advMushY + 6, 4, 6, TFT_WHITE);
          } else { // Fire Flower
            uint16_t flColors[] = {TFT_RED, TFT_YELLOW, TFT_GREEN};
            display.fillCircle(smx + 6, advMushY + 6, 5, flColors[(millis()/150)%3]);
            display.drawFastVLine(smx + 6, advMushY + 9, 3, TFT_GREEN);
          }
        }
      }

      // 6. Draw Player Fireballs
      for (int i = 0; i < 3; i++) {
        if (advFireballActive[i]) {
          float sfx = advFireballX[i] - advCameraX;
          if (sfx >= -4 && sfx <= G_R) {
            display.fillCircle(sfx, advFireballY[i], 3, TFT_ORANGE);
            display.drawCircle(sfx, advFireballY[i], 3, TFT_RED);
          }
        }
      }

      // 7. Draw Coins
      for (int i = 0; i < 6; i++) {
        if (advCoinActive[i]) {
          float scx = advCoinX[i] - advCameraX;
          if (scx >= -10 && scx <= G_R) {
            // Spinner animation
            int coinFrame = (millis() / 150) % 4;
            int cw = (coinFrame == 0 || coinFrame == 2) ? 6 : ((coinFrame == 1) ? 2 : 8);
            display.fillEllipse(scx, advCoinY[i], cw/2, 4, TFT_YELLOW);
            display.drawEllipse(scx, advCoinY[i], cw/2, 4, 0xFDA0);
          }
        }
      }

      // 8. Draw Enemies
      for (int i = 0; i < 4; i++) {
        if (advEnemyActive[i]) {
          float sex = advEnemyX[i] - advCameraX;
          if (sex >= -12 && sex <= G_R) {
            if (advEnemyType[i] == 0) { // Goomba
              display.fillCircle(sex, advEnemyY[i] - 5, 5, 0x9300); // brown cap
              display.fillRect(sex - 3, advEnemyY[i] - 3, 6, 3, TFT_WHITE); // stem
              display.fillRect(sex - 4, advEnemyY[i] - 1, 8, 2, TFT_BLACK); // feet
            } 
            else if (advEnemyType[i] == 1) { // Koopa
              display.fillRect(sex - 4, advEnemyY[i] - 8, 8, 6, TFT_GREEN); // green shell
              display.fillCircle(sex, advEnemyY[i] - 10, 3, TFT_YELLOW); // head
              display.drawFastHLine(sex - 3, advEnemyY[i] - 2, 6, TFT_BLACK); // feet
            }
            else { // Winged Koopa
              display.fillRect(sex - 4, advEnemyY[i] - 8, 8, 6, TFT_GREEN);
              display.fillCircle(sex, advEnemyY[i] - 10, 3, TFT_YELLOW);
              // wing drawing
              display.fillTriangle(sex + 4, advEnemyY[i] - 12, sex + 8, advEnemyY[i] - 14, sex + 4, advEnemyY[i] - 8, TFT_WHITE);
            }
          }
        }
      }

      // 9. Draw Bowser Boss & Boss Fireballs (Level 3)
      if (advBossActive && advCurrentLevel == 3) {
        float sbx = advBossX - advCameraX;
        display.fillRect(sbx - 8, advBossY - 16, 16, 16, 0x93A0); // Bowser dark green shell
        display.fillCircle(sbx, advBossY - 18, 6, TFT_YELLOW); // yellow head/mouth
        display.fillRect(sbx - 12, advBossY - 6, 24, 6, 0x4B20); // feet/claws
        
        // draw boss red spikes
        display.fillTriangle(sbx - 8, advBossY - 14, sbx - 12, advBossY - 18, sbx - 4, advBossY - 14, TFT_RED);
        display.fillTriangle(sbx + 4, advBossY - 14, sbx, advBossY - 18, sbx + 8, advBossY - 14, TFT_RED);

        // boss health bar
        display.fillRect(sbx - 15, advBossY - 26, 30, 4, TFT_BLACK);
        display.fillRect(sbx - 15, advBossY - 26, advBossHP * 30 / 5, 4, TFT_RED);

        // draw Golden Axe victory trigger
        float sAxeX = 350 - advCameraX;
        if (sAxeX >= -10 && sAxeX <= G_R) {
          display.fillRect(sAxeX - 2, G_B - 25, 4, 11, TFT_YELLOW);
          display.fillCircle(sAxeX, G_B - 25, 4, TFT_RED);
        }

        // Bowser Fireballs
        for (int f = 0; f < 3; f++) {
          if (advBossFireActive[f]) {
            float sbfx = advBossFireX[f] - advCameraX;
            if (sbfx >= -6 && sbfx <= G_R) {
              display.fillCircle(sbfx, advBossFireY[f], 4, TFT_ORANGE);
              display.drawCircle(sbfx, advBossFireY[f], 4, TFT_RED);
            }
          }
        }
      }

      // 10. Draw Player
      bool isInvincibleFlashing = (millis() < advInvincibleTime) && ((millis() / 80) % 2 == 0);
      if (!isInvincibleFlashing) {
        // Draw body box
        uint16_t pCol = themeAccent;
        if (advPowerState == 1) pCol = 0xFDA0; // Super Orange
        else if (advPowerState == 2) pCol = TFT_WHITE; // Fire White
        
        display.fillRect(advPlayerX - 4, advPlayerY - pHeight, 8, pHeight, pCol);
        display.fillCircle(advPlayerX, advPlayerY - pHeight, 3, TFT_YELLOW); // head
        
        // draw tiny cap (red for mario style)
        display.drawFastHLine(advPlayerX - 3, advPlayerY - pHeight - 4, 6, TFT_RED);
        display.fillRect(advPlayerX - 1, advPlayerY - pHeight - 5, 3, 2, TFT_RED);

        // walking legs animation
        if (advPlayerVX > 0 && !advIsJumping) {
          if (advWalkFrame == 0) {
            display.drawFastHLine(advPlayerX - 4, advPlayerY - 1, 3, TFT_BLACK);
          } else {
            display.drawFastHLine(advPlayerX + 1, advPlayerY - 1, 3, TFT_BLACK);
          }
        }
      }
    } 
    else if (advLevelState == 3) {
      // "STAGE CLEAR" transition screen
      display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_BLACK);
      display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
      display.setTextColor(TFT_GREEN);
      display.setCursor((SCREEN_WIDTH - 11 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, SCREEN_WIDTH == 240 ? 90 : 60);
      display.print("STAGE CLEAR");
      
      display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
      display.setTextColor(TFT_WHITE);
      display.setCursor((SCREEN_WIDTH - 10 * (SCREEN_WIDTH == 240 ? 12 : 6)) / 2, SCREEN_WIDTH == 240 ? 140 : 100);
      display.printf("SCORE: %04d", advScore);
    } 
    else {
      // Game Over / Victory Screen
      if (advLives == 99) {
        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, themeBg);
        display.setTextSize(SCREEN_WIDTH == 240 ? 3 : 2);
        display.setTextColor(TFT_GREEN);
        display.setCursor((SCREEN_WIDTH - 8 * (SCREEN_WIDTH == 240 ? 18 : 12)) / 2, G_Y + 20);
        display.print("VICTORY!");
        
        display.setTextSize(SCREEN_WIDTH == 240 ? 2 : 1);
        display.setTextColor(themeText);
        display.setCursor(20, G_Y + 60);
        display.print("BOWSER DEFEATED!");
        
        display.setTextColor(themeAccent);
        display.setCursor(20, G_Y + 90);
        display.print("B1: PLAY AGAIN");

        // B1 tap = Play Again from Victory
        if (btn1 && !advLastBtn1) {
          resetAdventure();
          audio.playSound(SOUND_POWERUP);
        }
      } else {
        drawGameOverScreen(display, advScore, advHighScore);

        // B1 tap = Play Again from Game Over
        if (btn1 && !advLastBtn1) {
          resetAdventure();
          audio.playSound(SOUND_POWERUP);
        }
      }
    }
    // Always track button state so edge detection works across all game states
    advLastBtn1 = btn1;
  }

  void updateAndDrawRacer(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    bool btn2 = (digitalRead(BTN_SETTINGS_PIN) == LOW);

    if (!racGameOver) {
      if (btn1 && btn2) {
        if (racNitroFuel > 0) {
          racNitroActive = true;
          racSpeed = 8.0f;
          racNitroFuel -= 0.5f;
          if (millis() % 200 < 100) {
            audio.playSound(SOUND_JUMP);
          }
        } else {
          racNitroActive = false;
          racSpeed = 4.0f;
        }
      } else {
        racNitroActive = false;
        racSpeed = 4.0f;
      }

      if (btn1 && !btn2) {
        racPlayerX -= 3;
        if (racPlayerX < G_X + 16) racPlayerX = G_X + 16;
      } else if (btn2 && !btn1) {
        racPlayerX += 3;
        if (racPlayerX > G_R - 16 - 8) racPlayerX = G_R - 16 - 8;
      }

      racRoadScroll += racSpeed;
      if (racRoadScroll >= 40) racRoadScroll = 0;
      racScore += racSpeed / 5;

      for (int i = 0; i < 4; i++) {
        if (racCarActive[i]) {
          racCarY[i] += (racSpeed - racCarSpeed[i]);
          
          if (racCarY[i] > G_B + 10) {
            racCarY[i] = G_Y - 30;
            racCarX[i] = G_X + 20 + random(0, 3) * (G_W - 48)/3;
            racCarSpeed[i] = 1.0f + random(0, 20) / 10.0f;
            racCarActive[i] = true;
          }

          if (abs(racPlayerX - (racCarX[i] + 4)) < 8 && abs(racPlayerY - (racCarY[i] + 7)) < 14) {
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

    display.fillScreen(0x39E7);

    display.fillRect(G_X + 16, G_Y, G_W - 32, G_H, 0x4208);
    display.drawFastVLine(G_X + 16, G_Y, G_H, TFT_WHITE);
    display.drawFastVLine(G_R - 16, G_Y, G_H, TFT_WHITE);

    for (int y = G_Y - 20; y < G_B; y += 40) {
      float scrollY = y + (int)racRoadScroll;
      if (scrollY >= G_Y && scrollY < G_B - 20) {
        display.fillRect(G_X + G_W / 2 - 1, scrollY, 2, 20, TFT_YELLOW);
      }
    }

    uint16_t carColors[] = {TFT_RED, TFT_BLUE, TFT_GREEN};
    for (int i = 0; i < 4; i++) {
      if (racCarActive[i] && racCarY[i] >= G_Y && racCarY[i] < G_B) {
        display.fillRect(racCarX[i], racCarY[i], 8, 14, carColors[racCarColor[i]]);
        display.fillRect(racCarX[i] + 2, racCarY[i] + 3, 4, 3, TFT_CYAN);
        display.fillRect(racCarX[i] + 1, racCarY[i] + 11, 2, 2, TFT_RED);
        display.fillRect(racCarX[i] + 5, racCarY[i] + 11, 2, 2, TFT_RED);
      }
    }

    display.fillRect(racPlayerX, racPlayerY, 8, 14, 0xFDA0);
    display.fillRect(racPlayerX + 2, racPlayerY + 3, 4, 3, TFT_WHITE);
    display.fillRect(racPlayerX + 1, racPlayerY + 11, 2, 2, TFT_RED);
    display.fillRect(racPlayerX + 5, racPlayerY + 11, 2, 2, TFT_RED);

    if (racNitroActive && (millis() % 100 < 50)) {
      display.fillTriangle(racPlayerX + 2, racPlayerY + 14, racPlayerX + 4, racPlayerY + 20, racPlayerX + 6, racPlayerY + 14, TFT_ORANGE);
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

  void updateAndDrawSpace(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool btn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    bool btn2 = (digitalRead(BTN_SETTINGS_PIN) == LOW);

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
      else if (btn1 && !btn2) {
        spcPlayerX -= 3;
        if (spcPlayerX < G_X + 8) spcPlayerX = G_X + 8;
      }
      else if (btn2 && !btn1) {
        spcPlayerX += 3;
        if (spcPlayerX > G_R - 8) spcPlayerX = G_R - 8;
      }

      if (millis() - spcLastShoot > 350) {
        spcLastShoot = millis();
        audio.playSound(SOUND_CHIRP);
        if (spcSpreadActive) {
          float angles[] = {-1.0f, 0.0f, 1.0f};
          for (int a = 0; a < 3; a++) {
            for (int i = 0; i < 6; i++) {
              if (!spcLaserActive[i]) {
                spcLaserActive[i] = true;
                spcLaserX[i] = spcPlayerX + angles[a] * 4.0f;
                spcLaserY[i] = spcPlayerY - 8;
                break;
              }
            }
          }
        } else {
          for (int i = 0; i < 6; i++) {
            if (!spcLaserActive[i]) {
              spcLaserActive[i] = true;
              spcLaserX[i] = spcPlayerX;
              spcLaserY[i] = spcPlayerY - 8;
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

          if (abs(spcEnemyLaserX[i] - spcPlayerX) < 10 && spcEnemyLaserY[i] >= spcPlayerY - 6 && spcEnemyLaserY[i] <= spcPlayerY + 4) {
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
                spcEnemyLaserY[el] = spcEnemyY[i] + 4;
                break;
              }
            }
          }

          for (int l = 0; l < 6; l++) {
            if (spcLaserActive[l]) {
              if (abs(spcLaserX[l] - spcEnemyX[i]) < 10 && abs(spcLaserY[l] - spcEnemyY[i]) < 8) {
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
            spcEnemyX[i] = G_X + 20 + i * (G_W - 40)/4;
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
        
        if (abs(spcPowerX - spcPlayerX) < 12 && abs(spcPowerY - spcPlayerY) < 12) {
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
        if (spcBossX < G_X + 20) spcBossDir = 1;
        if (spcBossX > G_R - 20) spcBossDir = -1;

        if (millis() - spcBossLastShoot > 1800) {
          spcBossLastShoot = millis();
          for (int l = 0; l < 2; l++) {
            for (int el = 0; el < 4; el++) {
              if (!spcEnemyLaserActive[el]) {
                spcEnemyLaserActive[el] = true;
                spcEnemyLaserX[el] = spcBossX + (l == 0 ? -10 : 10);
                spcEnemyLaserY[el] = spcBossY + 6;
                break;
              }
            }
          }
        }

        for (int l = 0; l < 6; l++) {
          if (spcLaserActive[l]) {
            if (abs(spcLaserX[l] - spcBossX) < 18 && abs(spcLaserY[l] - spcBossY) < 12) {
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

    display.fillScreen(0x0002);

    for (int i = 0; i < 8; i++) {
      int sx = (i * 27) % G_W;
      int sy = (G_Y + (i * 35) + (millis() / 20)) % G_H + G_Y;
      display.drawPixel(sx, sy, 0x7BEF);
    }

    for (int i = 0; i < 6; i++) {
      if (spcLaserActive[i]) {
        display.drawFastVLine(spcLaserX[i], spcLaserY[i], 6, TFT_CYAN);
      }
    }

    for (int i = 0; i < 4; i++) {
      if (spcEnemyLaserActive[i]) {
        display.drawFastVLine(spcEnemyLaserX[i], spcEnemyLaserY[i], 5, TFT_RED);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (spcEnemyActive[i]) {
        display.fillTriangle(spcEnemyX[i], spcEnemyY[i]+6, spcEnemyX[i]-6, spcEnemyY[i]-4, spcEnemyX[i]+6, spcEnemyY[i]-4, TFT_MAGENTA);
      }
    }

    if (spcPowerActive) {
      display.fillRect(spcPowerX - 4, spcPowerY - 4, 8, 8, spcPowerType == 0 ? TFT_YELLOW : TFT_GREEN);
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
      display.fillRoundRect(spcBossX - 16, spcBossY - 8, 32, 16, 4, TFT_RED);
      display.fillRect(spcBossX - 4, spcBossY + 8, 8, 4, TFT_YELLOW);
      
      display.drawFastHLine(spcBossX - 15, spcBossY - 14, 30, TFT_BLACK);
      display.drawFastHLine(spcBossX - 15, spcBossY - 14, spcBossHP * 30 / 20, TFT_GREEN);
    }

    display.fillTriangle(spcPlayerX, spcPlayerY - 8, spcPlayerX - 6, spcPlayerY + 4, spcPlayerX + 6, spcPlayerY + 4, themeAccent);
    
    if (millis() < spcShieldTime) {
      display.drawCircle(spcPlayerX, spcPlayerY - 2, 10, TFT_GREEN);
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
};

#endif // GAMES_H
