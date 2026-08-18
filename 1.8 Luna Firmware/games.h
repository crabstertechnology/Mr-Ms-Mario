#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include "audio.h"

// External references
extern bool gamesActive;
extern bool gamePlaying;
extern int gameMenuOption;
extern int gameSelected;

class LunaGames {
private:
  // --- Game 1 (Coin Catcher) State ---
  int playerX;
  int playerY;
  const int playerWidth = 36;
  const int playerHeight = 14;
  int coinX;
  int coinY;
  float coinSpeed;
  int score1;
  int lives1;
  bool gameOver1;

  // --- Game 2 (Flappy Mochy) State ---
  float birdY;
  float birdVelocity;
  const float gravity = 0.45f;
  const float jumpStrength = -5.8f;
  const int birdX = 60;
  const int birdRadius = 9;
  int pipeX;
  const int pipeWidth = 36;
  int gapY;
  const int gapHeight = 70;
  float pipeSpeed;
  int score2;
  bool gameOver2;
  bool gameStarted2;
  bool lastBtn1State;

  // --- Game 3 (Retro Snake) State ---
  int snakeX[30];
  int snakeY[30];
  int snakeLength;
  int snakeDir; // 0: Up, 1: Right, 2: Down, 3: Left
  int foodX;
  int foodY;
  int score3;
  bool gameOver3;
  unsigned long lastSnakeUpdate;
  bool lastBtn2State;

  // --- Game 4 (Space Invaders) State ---
  int invaderX[10];
  int invaderY[10];
  bool invaderAlive[10];
  int invaderDir;
  int invaderCount;
  int laserX;
  int laserY;
  bool laserActive;
  int playerInvX;
  int score4;
  bool gameOver4;
  bool gameWon4;
  unsigned long lastInvaderUpdate;
  unsigned long lastLaserUpdate;

  // --- Game 5 (Pong Challenge) State ---
  float ballX;
  float ballY;
  float ballVX;
  float ballVY;
  int paddlePlayerY;
  int paddleCpuY;
  int scorePlayer;
  int scoreCpu;
  bool gameOver5;
  bool gameWon5;

  // --- Game 6 (Breakout) State ---
  float breakBallX;
  float breakBallY;
  float breakBallVX;
  float breakBallVY;
  int breakPaddleX;
  bool brickActive[15];
  int score6;
  int lives6;
  bool gameOver6;
  bool gameWon6;

  // --- Game 7 (Memory Match) State ---
  int cards[8];
  int cardState[8];
  int cursorIndex;
  int firstSelected;
  unsigned long matchTimer;
  bool matchingInProgress;
  int score7;
  bool gameOver7;
  bool lastBtn1StateMM;
  bool lastBtn2StateMM;

public:
  LunaGames() {
    resetCoinCatcher();
    resetFlappyMochy();
    resetSnake();
    resetSpaceInvaders();
    resetPong();
    resetBreakout();
    resetMemoryMatch();
    lastBtn1State = false;
    lastBtn2State = false;
  }

  // â”€â”€ Layout constants for 128Ã—160 display â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
  // Status bar occupies y=0..26 (drawn by drawStatusBar externally)
  // Game outer border: x=2, y=28, w=124, h=130  â†’  bottom = y=158
  // Score bar: y=30..54  (24 px tall)
  // Divider:   y=54
  // Play area: x=4, y=56, w=120, h=98  â†’  bottom = y=154
  // â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
  static const int G_X = 4;     // play area left
  static const int G_Y = 56;    // play area top
  static const int G_W = 120;   // play area width
  static const int G_H = 98;    // play area height
  static const int G_B = 154;   // play area bottom (G_Y + G_H)
  static const int G_R = 124;   // play area right  (G_X + G_W)

  void resetCoinCatcher() {
    playerX = (SCREEN_WIDTH - playerWidth) / 2;
    playerY = G_B - 20;                      // 10 px above play area bottom
    coinX   = random(G_X + 8, G_R - 8);
    coinY   = G_Y + 10;
    coinSpeed = 2.0f;
    score1 = 0;
    lives1 = 3;
    gameOver1 = false;
  }

  void resetFlappyMochy() {
    birdY        = G_Y + G_H / 2.0f;
    birdVelocity = 0.0f;
    pipeX        = SCREEN_WIDTH + 10;
    gapY         = G_Y + 30 + random(0, G_H - 60);
    pipeSpeed    = 2.0f;
    score2       = 0;
    gameOver2    = false;
    gameStarted2 = false;
  }

  void resetSnake() {
    snakeLength = 4;
    snakeDir = 1;
    // Grid: 19 cols Ã— 14 rows of 6 px cells, origin (G_X+2, G_Y+2)
    for (int i = 0; i < snakeLength; i++) {
      snakeX[i] = 5 - i;
      snakeY[i] = 4;
    }
    foodX = random(0, 19);
    foodY = random(0, 14);
    score3 = 0;
    gameOver3 = false;
    lastSnakeUpdate = millis();
    lastBtn2State   = false;
  }

  void resetSpaceInvaders() {
    playerInvX   = G_X + G_W / 2 - 8;
    invaderDir   = 1;
    invaderCount = 10;
    laserActive  = false;
    score4       = 0;
    gameOver4    = false;
    gameWon4     = false;
    // 2 rows of 5 invaders, each 12 px wide, 8 px gap â†’ step=20
    for (int i = 0; i < 5; i++) {
      invaderX[i]    = G_X + 4 + i * 22;
      invaderY[i]    = G_Y + 8;
      invaderAlive[i] = true;
    }
    for (int i = 5; i < 10; i++) {
      invaderX[i]    = G_X + 4 + (i - 5) * 22;
      invaderY[i]    = G_Y + 22;
      invaderAlive[i] = true;
    }
    lastInvaderUpdate = millis();
    lastLaserUpdate   = millis();
  }

  void resetPong() {
    ballX        = G_X + G_W / 2.0f;
    ballY        = G_Y + G_H / 2.0f;
    ballVX       = random(0, 2) == 0 ? -2.5f : 2.5f;
    ballVY       = random(-15, 16) / 10.0f;
    paddlePlayerY = G_Y + G_H / 2 - 12;
    paddleCpuY    = G_Y + G_H / 2 - 12;
    scorePlayer  = 0;
    scoreCpu     = 0;
    gameOver5    = false;
    gameWon5     = false;
  }

  void resetBreakout() {
    // Bricks: 5 cols Ã— 3 rows, each 20Ã—8, gap 2 â†’ step 22
    breakPaddleX = G_X + G_W / 2 - 14;
    breakBallX   = G_X + G_W / 2.0f;
    breakBallY   = G_B - 20.0f;
    breakBallVX  = 2.0f;
    breakBallVY  = -2.5f;
    score6       = 0;
    lives6       = 3;
    gameOver6    = false;
    gameWon6     = false;
    for (int i = 0; i < 15; i++) brickActive[i] = true;
  }

  void resetMemoryMatch() {
    int tempCards[] = {0, 0, 1, 1, 2, 2, 3, 3};
    for (int i = 0; i < 8; i++) {
      int r = random(i, 8);
      int temp = tempCards[i];
      tempCards[i] = tempCards[r];
      tempCards[r] = temp;
    }
    for (int i = 0; i < 8; i++) {
      cards[i] = tempCards[i];
      cardState[i] = 0;
    }
    cursorIndex = 0;
    firstSelected = -1;
    matchingInProgress = false;
    score7 = 0;
    gameOver7 = false;
    lastBtn1StateMM = false;
    lastBtn2StateMM = false;
  }

  bool canExitActiveGame() {
    if (gameSelected == 1) return gameOver1;
    if (gameSelected == 2) return (!gameStarted2 || gameOver2);
    if (gameSelected == 3) return gameOver3;
    if (gameSelected == 4) return gameOver4;
    if (gameSelected == 5) return gameOver5;
    if (gameSelected == 6) return gameOver6;
    if (gameSelected == 7) return gameOver7;
    return false;
  }

  void drawGameOverScreen(GFXcanvas16& display, int score) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg  = TFT_WHITE;
    uint16_t themeText = 0x2104;

    // Fill screen area
    display.fillRect(0, 22, 128, 138, themeBg);

    // "GAME OVER"
    display.setTextSize(2);
    display.setTextColor(TFT_RED, themeBg);
    display.setCursor((128 - 9*12) / 2, 48);
    display.print("GAME OVER");

    // Score line
    display.setTextSize(1);
    display.setTextColor(themeText, themeBg);
    char scoreBuf[24];
    snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %d", score);
    int scoreW = strlen(scoreBuf) * 6;
    display.setCursor((128 - scoreW) / 2, 80);
    display.print(scoreBuf);

    // Navigation buttons
    display.setTextColor(themeAccent, themeBg);
    display.setCursor((128 - 13*6) / 2, 108);
    display.print("B1:PLAY AGAIN");
    display.setCursor((128 - 14*6) / 2, 124);
    display.print("B2:BACK 2 MENU");
  }

  void drawMenu(GFXcanvas16& display) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeCardBg = (robotVariant == "mr_luna") ? 0xE7FC : 0xFDF2; // light pastel bg
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xD69A;

    // Fill screen area y=22..159 with pure white
    display.fillRect(0, 22, 128, 138, themeBg);

    const char* gameNames[] = {
      "1.COIN CATCH",
      "2.FLAPPY MOCHY",
      "3.RETRO SNAKE",
      "4.SPACE INVAD",
      "5.PONG CHALL",
      "6.BREAKOUT",
      "7.MEMO MATCH",
      "8.EXIT ARCADE"
    };

    // Calculate scroll offset (visible items)
    int startVisible = gameMenuOption - 2;
    if (startVisible < 0) startVisible = 0;
    if (startVisible > 3) startVisible = 3;

    // Draw 5 menu items starting at y=28 with 22px spacing
    for (int i = 0; i < 5; i++) {
      int idx = startVisible + i;
      if (idx >= 8) break;
      int yPos = 28 + i * 22;
      bool sel = (gameMenuOption == idx);
      
      if (sel) {
        // Subtle clean select pill
        display.fillRoundRect(6, yPos, 104, 18, 4, themeCardBg);
        display.drawRoundRect(6, yPos, 104, 18, 4, themeAccent);
        display.setTextColor(themeAccent);
      } else {
        display.setTextColor(themeText);
      }
      
      display.setTextSize(1);
      display.setCursor(14, yPos + 5);
      display.print(gameNames[idx]);
    }

    // Scrollbar on right
    display.fillRect(118, 28, 2, 106, 0xEF5C);
    int thumbY = 28 + (gameMenuOption * (106 - 18)) / 7;
    display.fillRect(118, thumbY, 2, 18, themeAccent);

    // Clean footer
    display.setTextSize(1);
    display.setTextColor(0x7BCF);
    display.setCursor(8, 146);
    display.print("B1:Scroll  B2:Play");
  }

  void updateAndDrawCoinCatcher(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    if (!gameOver1) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        playerX -= 6;
        if (playerX < G_X) playerX = G_X;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        playerX += 6;
        if (playerX > G_R - playerWidth) playerX = G_R - playerWidth;
      }
      coinY += (int)coinSpeed;
      if (coinY + 5 >= playerY && coinY - 5 <= playerY + playerHeight &&
          coinX + 5 >= playerX && coinX - 5 <= playerX + playerWidth) {
        audio.playSound(SOUND_COIN);
        score1++;
        coinSpeed += 0.25f;
        if (coinSpeed > 7.0f) coinSpeed = 7.0f;
        coinY = G_Y + 8;
        coinX = random(G_X + 6, G_R - 6);
      } else if (coinY > G_B) {
        audio.playSound(SOUND_POWERDOWN);
        if (--lives1 <= 0) { gameOver1 = true; audio.playSound(SOUND_GAMEOVER); }
        else { coinY = G_Y + 8; coinX = random(G_X + 6, G_R - 6); }
      }
    } else {
      bool b = (digitalRead(BTN_EXPR_PIN) == LOW);
      if (b && !lastBtn1State) resetCoinCatcher();
      lastBtn1State = b;
    }

    // Clean & Borderless frame
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);

    // Score bar (y=31..54)
    display.setTextSize(1);
    display.setTextColor(themeText);
    display.setCursor(8, 36);
    display.printf("SCORE:%03d", score1);
    // Lives (hearts) right side
    for (int i = 0; i < 3; i++) {
      int hx = 84 + i * 13;
      if (i < lives1) {
        display.fillCircle(hx-2, 36, 2, TFT_RED);
        display.fillCircle(hx+2, 36, 2, TFT_RED);
        display.fillTriangle(hx-4, 37, hx+4, 37, hx, 42, TFT_RED);
      } else {
        display.fillCircle(hx, 38, 3, themeBorder);
      }
    }
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver1) {
      // Coin: radius 5
      display.fillCircle(coinX, coinY, 5, 0xFDA0);
      display.drawCircle(coinX, coinY, 5, themeText);
      display.fillCircle(coinX, coinY, 2, TFT_YELLOW);
      // Paddle
      display.fillRoundRect(playerX, playerY, playerWidth, playerHeight, 3, themeAccent);
      display.fillRoundRect(playerX+3, playerY+3, playerWidth-6, playerHeight-6, 2, themeBg);
    } else {
      drawGameOverScreen(display, score1);
    }
  }

  void updateAndDrawFlappyMochy(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg     = TFT_WHITE;
    uint16_t themeText   = 0x2104;
    uint16_t themeBorder = 0xCE79;

    // birdX is const=60; gap half = 22 px; pipe width=16
    const int PW = 16;
    const int GH = 44;  // gap half

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    if (!gameOver2) {
      if (currentBtn1 && !lastBtn1State) {
        if (!gameStarted2) gameStarted2 = true;
        birdVelocity = jumpStrength;
        audio.playSound(SOUND_JUMP);
      }
      if (gameStarted2) {
        birdVelocity += gravity;
        birdY += birdVelocity;
        if (birdY + 5 >= G_B)  { birdY = G_B - 5; gameOver2 = true; audio.playSound(SOUND_GAMEOVER); }
        if (birdY - 5 <= G_Y)  { birdY = G_Y + 5; birdVelocity = 0; }
        pipeX -= (int)pipeSpeed;
        if (pipeX + PW < G_X) {
          pipeX = G_R + 4;
          gapY  = G_Y + GH + random(0, G_H - GH*2);
          score2++;
          audio.playSound(SOUND_COIN);
          pipeSpeed += 0.15f;
          if (pipeSpeed > 5.0f) pipeSpeed = 5.0f;
        }
        if (birdX + 5 >= pipeX && birdX - 5 <= pipeX + PW) {
          if ((int)birdY - 5 <= gapY - GH || (int)birdY + 5 >= gapY + GH) {
            gameOver2 = true; audio.playSound(SOUND_GAMEOVER);
          }
        }
      }
    } else {
      if (currentBtn1 && !lastBtn1State) resetFlappyMochy();
    }
    lastBtn1State = currentBtn1;

    // Clean & Borderless frame
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    // Score bar
    display.setTextSize(1);
    display.setTextColor(themeText);
    display.setCursor(8, 36);
    display.printf("SCORE:%03d", score2);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (gameStarted2 && !gameOver2) {
      // Top pipe
      display.fillRect(pipeX, G_Y, PW, gapY - GH - G_Y, TFT_GREEN);
      display.drawRect(pipeX, G_Y, PW, gapY - GH - G_Y, 0x03E0);
      // Bottom pipe
      display.fillRect(pipeX, gapY + GH, PW, G_B - (gapY + GH), TFT_GREEN);
      display.drawRect(pipeX, gapY + GH, PW, G_B - (gapY + GH), 0x03E0);
    }
    // Bird
    display.fillCircle(birdX, (int)birdY, 5, TFT_YELLOW);
    display.drawCircle(birdX, (int)birdY, 5, 0x7E00);
    display.fillCircle(birdX+2, (int)birdY-2, 1, themeText);
    display.fillTriangle(birdX+5, (int)birdY-1, birdX+9, (int)birdY, birdX+5, (int)birdY+1, 0xFD20);

    if (!gameStarted2 && !gameOver2) {
      display.setTextSize(1);
      display.setTextColor(themeText);
      display.setCursor((128-12*6)/2, 90);
      display.print("FLAPPY MOCHY");
      display.setTextColor(themeAccent);
      display.setCursor((128-9*6)/2, 108);
      display.print("B1: Flap!");
      display.setCursor(20, 135);
      display.print("B2: Exit");
    }

    if (gameOver2) {
      drawGameOverScreen(display, score2);
    }
  }

  void updateAndDrawSnake(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    bool currentBtn2 = (digitalRead(BTN_SETTINGS_PIN) == LOW);

    if (!gameOver3) {
      if (currentBtn1 && !lastBtn1State) {
        snakeDir = (snakeDir + 3) % 4;
        audio.playSound(SOUND_CHIRP);
      }
      if (currentBtn2 && !lastBtn2State) {
        snakeDir = (snakeDir + 1) % 4;
        audio.playSound(SOUND_CHIRP);
      }

      if (millis() - lastSnakeUpdate > 250) {
        lastSnakeUpdate = millis();

        int nextX = snakeX[0];
        int nextY = snakeY[0];
        if (snakeDir == 0) nextY--;
        else if (snakeDir == 1) nextX++;
        else if (snakeDir == 2) nextY++;
        else if (snakeDir == 3) nextX--;

        if (nextX < 0 || nextX >= 18 || nextY < 0 || nextY >= 15) {
          gameOver3 = true;
          audio.playSound(SOUND_GAMEOVER);
        }
        for (int i = 0; i < snakeLength; i++) {
          if (snakeX[i] == nextX && snakeY[i] == nextY) {
            gameOver3 = true;
            audio.playSound(SOUND_GAMEOVER);
          }
        }

        if (!gameOver3) {
          if (nextX == foodX && nextY == foodY) {
            audio.playSound(SOUND_COIN);
            score3++;
            if (snakeLength < 30) {
              snakeLength++;
            }
            for (int i = snakeLength - 1; i > 0; i--) {
              snakeX[i] = snakeX[i-1];
              snakeY[i] = snakeY[i-1];
            }
            snakeX[0] = nextX;
            snakeY[0] = nextY;
            foodX = random(0, 18);
            foodY = random(0, 15);
          } else {
            for (int i = snakeLength - 1; i > 0; i--) {
              snakeX[i] = snakeX[i-1];
              snakeY[i] = snakeY[i-1];
            }
            snakeX[0] = nextX;
            snakeY[0] = nextY;
          }
        }
      }
    } else {
      if (currentBtn1 && !lastBtn1State) {
        resetSnake();
      }
    }
    lastBtn1State = currentBtn1;
    lastBtn2State = currentBtn2;

    // Clean & Borderless frame
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    // Score bar
    display.setTextSize(1); display.setTextColor(themeText);
    display.setCursor(8, 36);
    display.printf("SNAKE:%03d", score3);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver3) {
      // Grid: 19 cols Ã— 14 rows Ã— 6 px cells, origin G_X+1, G_Y+1
      display.drawRect(G_X, G_Y, G_W, G_H, themeBorder);
      // Food
      display.fillCircle(G_X + 1 + foodX*6 + 3, G_Y + 1 + foodY*6 + 3, 2, TFT_RED);
      // Snake segments
      for (int i = 0; i < snakeLength; i++) {
        display.fillRoundRect(G_X + 1 + snakeX[i]*6, G_Y + 1 + snakeY[i]*6, 5, 5, 1,
                              (i==0) ? themeAccent : 0x03E0);
      }
    } else {
      drawGameOverScreen(display, score3);
    }
  }

  void updateAndDrawSpaceInvaders(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver4) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        playerInvX -= 4;
      if (playerInvX < G_X)         playerInvX = G_X;
      if (playerInvX > G_R - 16)  playerInvX = G_R - 16;
      }

      if (millis() - lastInvaderUpdate > 400) {
        lastInvaderUpdate = millis();
        bool hitWall = false;
        for (int i = 0; i < 10; i++) {
          if (invaderAlive[i]) {
            invaderX[i] += invaderDir * 6;
            if (invaderX[i] < 12 || invaderX[i] > SCREEN_WIDTH - 28) {
              hitWall = true;
            }
          }
        }
        if (hitWall) {
          invaderDir = -invaderDir;
          for (int i = 0; i < 10; i++) {
            if (invaderAlive[i]) {
              invaderY[i] += 6;
              if (invaderY[i] >= SCREEN_HEIGHT - 32) {
                gameOver4 = true;
                audio.playSound(SOUND_GAMEOVER);
              }
            }
          }
        }
      }

      if (!laserActive) {
        laserX = playerInvX + 10;
        laserY = SCREEN_HEIGHT - 22;
        laserActive = true;
        audio.playSound(SOUND_CHIRP);
      } else {
        if (millis() - lastLaserUpdate > 30) {
          lastLaserUpdate = millis();
          laserY -= 6;
          for (int i = 0; i < 10; i++) {
            if (invaderAlive[i]) {
              if (laserX >= invaderX[i] && laserX <= invaderX[i] + 16 &&
                  laserY >= invaderY[i] && laserY <= invaderY[i] + 10) {
                invaderAlive[i] = false;
                laserActive = false;
                score4 += 10;
                invaderCount--;
                audio.playSound(SOUND_COIN);
                if (invaderCount == 0) {
                  gameWon4 = true;
                  gameOver4 = true;
                  audio.playSound(SOUND_POWERUP);
                }
                break;
              }
            }
          }
          if (laserY < 58) {
            laserActive = false;
          }
        }
      }
    } else {
      if (currentBtn1 && !lastBtn1State) {
        resetSpaceInvaders();
      }
    }
    lastBtn1State = currentBtn1;

    // Clean & Borderless frame
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    display.setTextSize(1); display.setTextColor(themeText);
    display.setCursor(8, 36); display.printf("SPACE:%03d", score4);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver4) {
      // Player ship at bottom
      display.fillTriangle(playerInvX+6, G_B-2, playerInvX, G_B+6, playerInvX+12, G_B+6, TFT_GREEN);
      if (laserActive) display.drawFastVLine(laserX, laserY, 5, 0xFDA0);
      for (int i = 0; i < 10; i++) {
        if (invaderAlive[i]) {
          display.fillRect(invaderX[i], invaderY[i], 12, 8, TFT_RED);
          display.fillRect(invaderX[i]+3, invaderY[i]+2, 2, 2, themeBg);
          display.fillRect(invaderX[i]+7, invaderY[i]+2, 2, 2, themeBg);
        }
      }
    } else {
      if (gameWon4) {
        display.setTextSize(2); display.setTextColor(TFT_GREEN, themeBg);
        display.setCursor((128-8*12)/2, 70); display.print("VICTORY!");
        display.setTextSize(1); display.setTextColor(themeText, themeBg);
        display.setCursor((128-10*6)/2, 100); display.printf("SCORE:%d", score4);
        display.setTextColor(themeAccent, themeBg);
        display.setCursor((128-11*6)/2, 118); display.print("B1:Re-Play");
        display.setCursor((128-7*6)/2, 134);  display.print("B2:Exit");
      } else { drawGameOverScreen(display, score4); }
    }
  }

  void updateAndDrawPong(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver5) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        paddlePlayerY -= 4;
        if (paddlePlayerY < 58) paddlePlayerY = 58;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        paddlePlayerY += 4;
        if (paddlePlayerY > SCREEN_HEIGHT - 42) paddlePlayerY = SCREEN_HEIGHT - 42;
      }

      ballX += ballVX;
      ballY += ballVY;

      if (ballY <= 58) {
        ballY = 58;
        ballVY = -ballVY;
        audio.playSound(SOUND_CHIRP);
      } else if (ballY >= SCREEN_HEIGHT - 12) {
        ballY = SCREEN_HEIGHT - 12;
        ballVY = -ballVY;
        audio.playSound(SOUND_CHIRP);
      }

      if (ballVX < 0 && ballX <= 24 && ballX >= 18 && ballY >= paddlePlayerY && ballY <= paddlePlayerY + 20) {
        ballVX = -ballVX * 1.05f;
        ballVY += ((ballY - (paddlePlayerY + 10)) / 10.0f) * 2.0f;
        ballX = 25;
        audio.playSound(SOUND_JUMP);
      }

      if (ballVX > 0 && ballX >= SCREEN_WIDTH - 28 && ballX <= SCREEN_WIDTH - 22 && ballY >= paddleCpuY && ballY <= paddleCpuY + 20) {
        ballVX = -ballVX * 1.05f;
        ballVY += ((ballY - (paddleCpuY + 10)) / 10.0f) * 2.0f;
        ballX = SCREEN_WIDTH - 29;
        audio.playSound(SOUND_JUMP);
      }

      if (ballY > paddleCpuY + 10) {
        paddleCpuY += 2;
      } else if (ballY < paddleCpuY + 10) {
        paddleCpuY -= 2;
      }
      paddleCpuY = constrain(paddleCpuY, 58, SCREEN_HEIGHT - 42);

      if (ballX < 12) {
        scoreCpu++;
        audio.playSound(SOUND_POWERDOWN);
        if (scoreCpu >= 5) {
          gameOver5 = true;
          gameWon5 = false;
          audio.playSound(SOUND_GAMEOVER);
        } else {
          ballX = SCREEN_WIDTH / 2;
          ballY = (SCREEN_HEIGHT - 36) / 2 + 30;
          ballVX = 3.0f;
          ballVY = random(-15, 16) / 10.0f;
        }
      } else if (ballX > SCREEN_WIDTH - 12) {
        scorePlayer++;
        audio.playSound(SOUND_COIN);
        if (scorePlayer >= 5) {
          gameOver5 = true;
          gameWon5 = true;
          audio.playSound(SOUND_POWERUP);
        } else {
          ballX = SCREEN_WIDTH / 2;
          ballY = (SCREEN_HEIGHT - 36) / 2 + 30;
          ballVX = -3.0f;
          ballVY = random(-15, 16) / 10.0f;
        }
      }
    } else {
      if (currentBtn1 && !lastBtn1State) {
        resetPong();
      }
    }
    lastBtn1State = currentBtn1;

    // Pong: horizontal play (ball bounces top/bottom, paddles on left/right)
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    display.setTextSize(1); display.setTextColor(themeText);
    display.setCursor(8, 36); display.printf("Y:%d CPU:%d", scorePlayer, scoreCpu);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver5) {
      // Centre dashed line
      for (int y = G_Y; y < G_B; y += 8)
        display.drawFastVLine(64, y, 4, themeBorder);
      // Paddles
      display.fillRect(G_X,    paddlePlayerY, 4, 24, themeAccent);
      display.fillRect(G_R-4,  paddleCpuY,    4, 24, TFT_RED);
      // Ball
      display.fillCircle((int)ballX, (int)ballY, 3, 0xFDA0);
    } else {
      if (gameWon5) {
        display.setTextSize(2); display.setTextColor(TFT_GREEN, themeBg);
        display.setCursor((128-8*12)/2, 70); display.print("VICTORY!");
        display.setTextSize(1); display.setTextColor(themeText, themeBg);
        display.setCursor((128-10*6)/2, 100); display.printf("YOU: %d", scorePlayer);
        display.setTextColor(themeAccent, themeBg);
        display.setCursor((128-11*6)/2, 118); display.print("B1:Re-Play");
        display.setCursor((128-7*6)/2, 134);  display.print("B2:Exit");
      } else { drawGameOverScreen(display, scorePlayer); }
    }
  }

  void updateAndDrawBreakout(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver6) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        breakPaddleX -= 5;
        if (breakPaddleX < 12) breakPaddleX = 12;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        breakPaddleX += 5;
        if (breakPaddleX > SCREEN_WIDTH - 12 - 28) breakPaddleX = SCREEN_WIDTH - 12 - 28;
      }

      breakBallX += breakBallVX;
      breakBallY += breakBallVY;

      if (breakBallX <= 14) {
        breakBallX = 14;
        breakBallVX = -breakBallVX;
        audio.playSound(SOUND_CHIRP);
      } else if (breakBallX >= SCREEN_WIDTH - 14) {
        breakBallX = SCREEN_WIDTH - 14;
        breakBallVX = -breakBallVX;
        audio.playSound(SOUND_CHIRP);
      }

      if (breakBallY <= 58) {
        breakBallY = 58;
        breakBallVY = -breakBallVY;
        audio.playSound(SOUND_CHIRP);
      }

      if (breakBallVY > 0 && breakBallY >= SCREEN_HEIGHT - 20 && breakBallY <= SCREEN_HEIGHT - 14 &&
          breakBallX >= breakPaddleX && breakBallX <= breakPaddleX + 28) {
        breakBallVY = -breakBallVY;
        breakBallVX = ((breakBallX - (breakPaddleX + 14)) / 14.0f) * 3.5f;
        audio.playSound(SOUND_JUMP);
      }

      if (breakBallY > SCREEN_HEIGHT - 12) {
        lives6--;
        audio.playSound(SOUND_POWERDOWN);
        if (lives6 <= 0) {
          gameOver6 = true;
          gameWon6 = false;
          audio.playSound(SOUND_GAMEOVER);
        } else {
          breakBallX = breakPaddleX + 14;
          breakBallY = SCREEN_HEIGHT - 36;
          breakBallVX = 2.5f;
          breakBallVY = -3.0f;
        }
      }

      bool anyBrickRemaining = false;
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
          int idx = c + r * 5;
          if (brickActive[idx]) {
            anyBrickRemaining = true;
            int bx = 16 + c * 20;
            int by = 70 + r * 12;
            if (breakBallX + 3 >= bx && breakBallX - 3 <= bx + 18 &&
                breakBallY + 3 >= by && breakBallY - 3 <= by + 8) {
              brickActive[idx] = false;
              breakBallVY = -breakBallVY;
              score6 += 10;
              audio.playSound(SOUND_COIN);
              break;
            }
          }
        }
      }

      if (!anyBrickRemaining) {
        gameWon6 = true;
        gameOver6 = true;
        audio.playSound(SOUND_POWERUP);
      }
    } else {
      if (currentBtn1 && !lastBtn1State) {
        resetBreakout();
      }
    }
    lastBtn1State = currentBtn1;

    // Breakout draw
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    display.setTextSize(1); display.setTextColor(themeText);
    display.setCursor(8, 36); display.printf("BRK:%03d", score6);
    // Lives dots
    for (int i = 0; i < 3; i++)
      display.fillCircle(88 + i*12, 38, 3, i < lives6 ? TFT_RED : themeBorder);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver6) {
      // Paddle
      display.fillRoundRect(breakPaddleX, G_B - 6, 28, 5, 2, themeAccent);
      // Ball
      display.fillCircle((int)breakBallX, (int)breakBallY, 3, 0xFDA0);
      // Bricks: 5c Ã— 3r, bx=G_X+2, w=20, gap=2â†’step22; by=G_Y+4, h=8, gap=2â†’step10
      uint16_t rowColors[] = {TFT_RED, TFT_ORANGE, TFT_GREEN};
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
          int idx = c + r*5;
          if (brickActive[idx]) {
            int bx = G_X + 2 + c*22;
            int by = G_Y + 4 + r*10;
            display.fillRect(bx, by, 20, 8, rowColors[r]);
            display.drawRect(bx, by, 20, 8, themeBg);
          }
        }
      }
    } else {
      if (gameWon6) {
        display.setTextSize(2); display.setTextColor(TFT_GREEN, themeBg);
        display.setCursor((128-8*12)/2, 70); display.print("VICTORY!");
        display.setTextSize(1); display.setTextColor(themeText, themeBg);
        display.setCursor((128-10*6)/2, 100); display.printf("SCORE:%d", score6);
        display.setTextColor(themeAccent, themeBg);
        display.setCursor((128-11*6)/2, 118); display.print("B1:Re-Play");
        display.setCursor((128-7*6)/2, 134);  display.print("B2:Exit");
      } else { drawGameOverScreen(display, score6); }
    }
  }

  void updateAndDrawMemoryMatch(GFXcanvas16& display, LunaAudio& audio) {
    extern String robotVariant;
    uint16_t themeAccent = (robotVariant == "mr_luna") ? 0x001F : 0xF8B8;
    uint16_t themeBg = TFT_WHITE;
    uint16_t themeText = 0x2104;
    uint16_t themeBorder = 0xCE79;

    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
    bool currentBtn2 = (digitalRead(BTN_SETTINGS_PIN) == LOW);

    if (!gameOver7) {
      if (currentBtn1 && !lastBtn1StateMM) {
        if (!matchingInProgress) {
          cursorIndex = (cursorIndex + 1) % 8;
          audio.playSound(SOUND_CHIRP);
        }
      }

      if (currentBtn2 && !lastBtn2StateMM) {
        if (!matchingInProgress && cardState[cursorIndex] == 0) {
          cardState[cursorIndex] = 1;
          audio.playSound(SOUND_JUMP);
          if (firstSelected == -1) {
            firstSelected = cursorIndex;
          } else {
            matchingInProgress = true;
            matchTimer = millis();
          }
        }
      }

      if (matchingInProgress && millis() - matchTimer > 1000) {
        matchingInProgress = false;
        int secondSelected = -1;
        for (int i = 0; i < 8; i++) {
          if (cardState[i] == 1 && i != firstSelected) {
            secondSelected = i;
            break;
          }
        }
        if (secondSelected != -1) {
          score7++;
          if (cards[firstSelected] == cards[secondSelected]) {
            cardState[firstSelected] = 2;
            cardState[secondSelected] = 2;
            audio.playSound(SOUND_COIN);
            bool allSolved = true;
            for (int i = 0; i < 8; i++) {
              if (cardState[i] != 2) allSolved = false;
            }
            if (allSolved) {
              gameOver7 = true;
              audio.playSound(SOUND_POWERUP);
            }
          } else {
            cardState[firstSelected] = 0;
            cardState[secondSelected] = 0;
            audio.playSound(SOUND_POWERDOWN);
          }
        }
        firstSelected = -1;
      }
    } else {
      if (currentBtn1 && !lastBtn1StateMM) {
        resetMemoryMatch();
      }
    }
    lastBtn1StateMM = currentBtn1;
    lastBtn2StateMM = currentBtn2;

    // Memory Match draw
    display.fillRect(0, 22, 128, 138, themeBg);
    display.drawRect(G_X - 1, G_Y - 1, G_W + 2, G_H + 2, themeBorder);
    display.setTextSize(1); display.setTextColor(themeText);
    display.setCursor(8, 36); display.printf("TRIES:%03d", score7);
    display.drawFastHLine(4, 54, 120, themeBorder);

    if (!gameOver7) {
      // 4 cols Ã— 2 rows. Cell 26Ã—34 px. Grid starts at x=6, y=58. Gap=2
      for (int i = 0; i < 8; i++) {
        int col = i % 4, row = i / 4;
        int cx = 6  + col * 30;
        int cy = 58 + row * 38;
        int cw = 26, ch = 34;
        if (cardState[i] == 0) {
          display.fillRoundRect(cx, cy, cw, ch, 3, 0xF7BE);
          display.drawRoundRect(cx, cy, cw, ch, 3, themeText);
          display.drawCircle(cx+cw/2, cy+ch/2, 4, themeAccent);
        } else {
          display.fillRoundRect(cx, cy, cw, ch, 3, themeBg);
          display.drawRoundRect(cx, cy, cw, ch, 3, cardState[i]==2 ? TFT_GREEN : themeAccent);
          int sx = cx+cw/2, sy = cy+ch/2, v = cards[i];
          if      (v==0) { display.fillCircle(sx-3,sy-3,3,TFT_RED); display.fillCircle(sx+3,sy-3,3,TFT_RED); display.fillTriangle(sx-5,sy,sx+5,sy,sx,sy+6,TFT_RED); }
          else if (v==1) { display.fillTriangle(sx,sy-5,sx-4,sy+3,sx+4,sy+3,TFT_YELLOW); display.fillTriangle(sx,sy+5,sx-4,sy-3,sx+4,sy-3,TFT_YELLOW); }
          else if (v==2) { display.fillTriangle(sx,sy-5,sx-4,sy,sx+4,sy,TFT_CYAN);   display.fillTriangle(sx,sy+5,sx-4,sy,sx+4,sy,TFT_CYAN); }
          else            { display.fillRect(sx-4,sy-4,8,8,TFT_ORANGE); }
        }
        if (i == cursorIndex && !matchingInProgress)
          display.drawRoundRect(cx-2, cy-2, cw+4, ch+4, 5, 0xFDA0);
      }
    } else {
      display.setTextSize(2); display.setTextColor(TFT_GREEN, themeBg);
      display.setCursor((128-8*12)/2, 70); display.print("VICTORY!");
      display.setTextSize(1); display.setTextColor(themeText, themeBg);
      display.setCursor((128-10*6)/2, 100); display.printf("TRIES:%d", score7);
      display.setTextColor(themeAccent, themeBg);
      display.setCursor((128-11*6)/2, 118); display.print("B1:Re-Play");
      display.setCursor((128-7*6)/2, 134);  display.print("B2:Exit");
    }
  }
};

#endif // GAMES_H
