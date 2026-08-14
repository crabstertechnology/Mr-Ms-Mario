#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
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

  void resetCoinCatcher() {
    playerX = (SCREEN_WIDTH - playerWidth) / 2;
    playerY = SCREEN_HEIGHT - 36;
    coinX = random(24, SCREEN_WIDTH - 24);
    coinY = 36;
    coinSpeed = 3.2f;
    score1 = 0;
    lives1 = 3;
    gameOver1 = false;
  }

  void resetFlappyMochy() {
    birdY = SCREEN_HEIGHT / 2.0f;
    birdVelocity = 0.0f;
    pipeX = SCREEN_WIDTH;
    gapY = random(75, SCREEN_HEIGHT - 85);
    pipeSpeed = 2.8f;
    score2 = 0;
    gameOver2 = false;
    gameStarted2 = false;
  }

  void resetSnake() {
    snakeLength = 4;
    snakeDir = 1; // Start moving right
    for (int i = 0; i < snakeLength; i++) {
      snakeX[i] = 5 - i;
      snakeY[i] = 8;
    }
    foodX = random(2, 19);
    foodY = random(2, 14);
    score3 = 0;
    gameOver3 = false;
    lastSnakeUpdate = millis();
    lastBtn2State = false;
  }

  void resetSpaceInvaders() {
    playerInvX = 110;
    invaderDir = 1;
    invaderCount = 10;
    laserActive = false;
    score4 = 0;
    gameOver4 = false;
    gameWon4 = false;
    for (int i = 0; i < 5; i++) {
      invaderX[i] = 30 + i * 35;
      invaderY[i] = 70;
      invaderAlive[i] = true;
    }
    for (int i = 5; i < 10; i++) {
      invaderX[i] = 30 + (i - 5) * 35;
      invaderY[i] = 90;
      invaderAlive[i] = true;
    }
    lastInvaderUpdate = millis();
    lastLaserUpdate = millis();
  }

  void resetPong() {
    ballX = 120;
    ballY = 140;
    ballVX = random(0, 2) == 0 ? -3.0f : 3.0f;
    ballVY = random(-20, 21) / 10.0f;
    paddlePlayerY = 120;
    paddleCpuY = 120;
    scorePlayer = 0;
    scoreCpu = 0;
    gameOver5 = false;
    gameWon5 = false;
  }

  void resetBreakout() {
    breakBallX = 120;
    breakBallY = 180;
    breakBallVX = 2.5f;
    breakBallVY = -3.0f;
    breakPaddleX = 102;
    score6 = 0;
    lives6 = 3;
    gameOver6 = false;
    gameWon6 = false;
    for (int i = 0; i < 15; i++) {
      brickActive[i] = true;
    }
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
    display.setTextSize(3);
    display.setTextColor(TFT_RED, 0x0821);
    display.setCursor(39, 65);
    display.print("GAME OVER");

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(36, 115);
    display.printf("FINAL SCORE:%d", score);

    display.setTextSize(2);
    display.setTextColor(TFT_LIGHTGREY, 0x0821);
    display.setCursor(42, 160);
    display.print("BTN1:PLAY AGAIN");
    display.setCursor(12, 190);
    display.print("BTN2:BACK TO MENU");
  }

  void drawMenu(GFXcanvas16& display) {
    // Clear screen with LUNA_DARK
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    // Header
    display.setTextSize(2);
    display.setTextColor(0xFDA0, 0x0821); // Orange
    display.setCursor(54, 38);
    display.print("LUNA ARCADE");
    display.drawFastHLine(12, 58, SCREEN_WIDTH - 24, 0x18E3);

    // Menu options (larger text, neat scroll window)
    const char* gameNames[] = {
      "1. COIN CATCHER",
      "2. FLAPPY MOCHY",
      "3. RETRO SNAKE",
      "4. SPACE INVADERS",
      "5. PONG CHALLENGE",
      "6. BRICK BREAKER",
      "7. MEMORY MATCH",
      "8. EXIT ARCADE"
    };

    int startVisible = gameMenuOption - 1;
    if (startVisible < 0) startVisible = 0;
    if (startVisible > 4) startVisible = 4; // 8 items total, so index 4 to 7 is 4 items.
    int startY = 66;
    int spacing = 37;

    for (int i = 0; i < 4; i++) {
      int itemIdx = startVisible + i;
      if (itemIdx >= 8) break;

      int yPos = startY + i * spacing;
      bool isCurrent = (gameMenuOption == itemIdx);

      if (isCurrent) {
        display.fillRoundRect(12, yPos, SCREEN_WIDTH - 24, 30, 4, 0x18E3); // Highlight
        display.drawRoundRect(12, yPos, SCREEN_WIDTH - 24, 30, 4, 0x07FF);
        display.setTextColor(TFT_WHITE, 0x18E3);
        
        // Selected indicator triangle
        display.fillTriangle(18, yPos + 10, 18, yPos + 20, 24, yPos + 15, TFT_YELLOW);
        display.setCursor(30, yPos + 7);
      } else {
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
        display.setCursor(20, yPos + 7);
      }
      
      display.setTextSize(2);
      display.print(gameNames[itemIdx]);
    }

    // Scrollbar indicator
    int scrollbarHeight = 142;
    int thumbHeight = scrollbarHeight / 8;
    int scrollbarY = 66 + (gameMenuOption * (scrollbarHeight - thumbHeight)) / 7;
    display.fillRect(SCREEN_WIDTH - 10, 66, 3, scrollbarHeight, 0x0842);
    display.fillRect(SCREEN_WIDTH - 10, scrollbarY, 3, thumbHeight, 0x07FF);

    display.setTextSize(1);
    display.setTextColor(TFT_DARKGREY, 0x0821);
    display.setCursor(39, SCREEN_HEIGHT - 18);
    display.print("BTN1: Cycle | BTN2: Select");
  }

  void updateAndDrawCoinCatcher(GFXcanvas16& display, LunaAudio& audio) {
    if (!gameOver1) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        playerX -= 8;
        if (playerX < 12) playerX = 12;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        playerX += 8;
        if (playerX > SCREEN_WIDTH - 12 - playerWidth) playerX = SCREEN_WIDTH - 12 - playerWidth;
      }

      coinY += (int)coinSpeed;

      if (coinY + 8 >= playerY && coinY - 8 <= playerY + playerHeight &&
          coinX + 8 >= playerX && coinX - 8 <= playerX + playerWidth) {
        audio.playSound(SOUND_COIN);
        score1++;
        coinSpeed += 0.35f;
        if (coinSpeed > 9.0f) coinSpeed = 9.0f;
        coinY = 36;
        coinX = random(24, SCREEN_WIDTH - 24);
      }
      else if (coinY > SCREEN_HEIGHT - 16) {
        audio.playSound(SOUND_POWERDOWN);
        lives1--;
        if (lives1 <= 0) {
          gameOver1 = true;
          audio.playSound(SOUND_GAMEOVER);
        } else {
          coinY = 36;
          coinX = random(24, SCREEN_WIDTH - 24);
        }
      }
    } else {
      bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
      if (currentBtn1 && !lastBtn1State) {
        resetCoinCatcher();
      }
      lastBtn1State = currentBtn1;
    }

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SCORE:%03d", score1);

    int lifeX = SCREEN_WIDTH - 54;
    for (int i = 0; i < 3; i++) {
      int hx = lifeX + i * 14;
      int hy = 42;
      if (i < lives1) {
        display.fillCircle(hx - 3, hy - 2, 3, TFT_RED);
        display.fillCircle(hx + 3, hy - 2, 3, TFT_RED);
        display.fillTriangle(hx - 6, hy, hx + 6, hy, hx, hy + 6, TFT_RED);
      } else {
        display.fillCircle(hx, hy + 1, 4, TFT_DARKGREY);
      }
    }
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver1) {
      display.fillCircle(coinX, coinY, 8, 0xFDA0);
      display.drawCircle(coinX, coinY, 8, TFT_WHITE);
      display.drawCircle(coinX, coinY, 5, TFT_YELLOW);

      display.fillRoundRect(playerX, playerY, playerWidth, playerHeight, 4, 0x07FF);
      display.fillRoundRect(playerX + 4, playerY + 3, playerWidth - 8, playerHeight - 6, 2, TFT_WHITE);
      display.fillRect(playerX + playerWidth/2 - 2, playerY - 3, 4, 3, 0xF8B8);
    } else {
      drawGameOverScreen(display, score1);
    }
  }

  void updateAndDrawFlappyMochy(GFXcanvas16& display, LunaAudio& audio) {
    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver2) {
      if (currentBtn1 && !lastBtn1State) {
        if (!gameStarted2) {
          gameStarted2 = true;
        }
        birdVelocity = jumpStrength;
        audio.playSound(SOUND_JUMP);
      }

      if (gameStarted2) {
        birdVelocity += gravity;
        birdY += birdVelocity;

        if (birdY + birdRadius >= SCREEN_HEIGHT - 12) {
          birdY = SCREEN_HEIGHT - 12 - birdRadius;
          gameOver2 = true;
          audio.playSound(SOUND_GAMEOVER);
        }
        if (birdY - birdRadius <= 32) {
          birdY = 32 + birdRadius;
          birdVelocity = 0.0f;
        }

        pipeX -= (int)pipeSpeed;
        if (pipeX + pipeWidth < 12) {
          pipeX = SCREEN_WIDTH - 12;
          gapY = random(75, SCREEN_HEIGHT - 85);
          score2++;
          audio.playSound(SOUND_COIN);
          pipeSpeed += 0.18f;
          if (pipeSpeed > 7.5f) pipeSpeed = 7.5f;
        }

        if (birdX + birdRadius >= pipeX && birdX - birdRadius <= pipeX + pipeWidth) {
          if (birdY - birdRadius <= gapY - gapHeight/2 || birdY + birdRadius >= gapY + gapHeight/2) {
            gameOver2 = true;
            audio.playSound(SOUND_GAMEOVER);
          }
        }
      }
    } else {
      if (currentBtn1 && !lastBtn1State) {
        resetFlappyMochy();
      }
    }
    lastBtn1State = currentBtn1;

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0xF8B8);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SCORE:%03d", score2);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (gameStarted2 && !gameOver2) {
      display.fillRect(pipeX, 30, pipeWidth, gapY - gapHeight/2 - 30, TFT_GREEN);
      display.drawRect(pipeX, 30, pipeWidth, gapY - gapHeight/2 - 30, 0x03E0);
      display.fillRect(pipeX - 2, gapY - gapHeight/2 - 8, pipeWidth + 4, 8, TFT_GREEN);
      display.drawRect(pipeX - 2, gapY - gapHeight/2 - 8, pipeWidth + 4, 8, 0x03E0);

      display.fillRect(pipeX, gapY + gapHeight/2, pipeWidth, SCREEN_HEIGHT - 8 - (gapY + gapHeight/2), TFT_GREEN);
      display.drawRect(pipeX, gapY + gapHeight/2, pipeWidth, SCREEN_HEIGHT - 8 - (gapY + gapHeight/2), 0x03E0);
      display.fillRect(pipeX - 2, gapY + gapHeight/2, pipeWidth + 4, 8, TFT_GREEN);
      display.drawRect(pipeX - 2, gapY + gapHeight/2, pipeWidth + 4, 8, 0x03E0);
    }

    display.fillCircle((int)birdX, (int)birdY, birdRadius, TFT_YELLOW);
    display.drawCircle((int)birdX, (int)birdY, birdRadius, 0x7E00);
    display.fillCircle((int)birdX + 3, (int)birdY - 3, 2, TFT_WHITE);
    display.fillCircle((int)birdX + 4, (int)birdY - 3, 1, TFT_BLACK);
    display.fillTriangle((int)birdX + birdRadius - 2, (int)birdY - 2, 
                         (int)birdX + birdRadius + 5, (int)birdY, 
                         (int)birdX + birdRadius - 2, (int)birdY + 2, 0xFD20);

    if (!gameStarted2 && !gameOver2) {
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, 0x0821);
      display.setCursor(28, 100);
      display.print("FLAPPY MOCHY");

      display.setTextSize(1);
      display.setTextColor(TFT_YELLOW, 0x0821);
      display.setCursor(44, 140);
      display.print("Press BTN1 to FLAP!");
      display.setCursor(44, 160);
      display.print("BTN2: Exit to Arcade");
    }

    if (gameOver2) {
      drawGameOverScreen(display, score2);
    }
  }

  void updateAndDrawSnake(GFXcanvas16& display, LunaAudio& audio) {
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

        if (nextX < 0 || nextX >= 21 || nextY < 0 || nextY >= 16) {
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
            foodX = random(0, 21);
            foodY = random(0, 16);
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

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SNAKE:%03d", score3);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver3) {
      display.drawRect(10, 58, 220, 170, 0x10A2);
      display.fillCircle(12 + foodX * 10 + 5, 60 + foodY * 10 + 5, 4, TFT_RED);

      for (int i = 0; i < snakeLength; i++) {
        uint16_t color = (i == 0) ? TFT_GREEN : 0x7E0;
        display.fillRoundRect(12 + snakeX[i] * 10 + 1, 60 + snakeY[i] * 10 + 1, 8, 8, 2, color);
      }
    } else {
      drawGameOverScreen(display, score3);
    }
  }

  void updateAndDrawSpaceInvaders(GFXcanvas16& display, LunaAudio& audio) {
    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver4) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        playerInvX -= 4;
        if (playerInvX < 12) playerInvX = 12;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        playerInvX += 4;
        if (playerInvX > 208) playerInvX = 208;
      }

      if (millis() - lastInvaderUpdate > 400) {
        lastInvaderUpdate = millis();
        bool hitWall = false;
        for (int i = 0; i < 10; i++) {
          if (invaderAlive[i]) {
            invaderX[i] += invaderDir * 6;
            if (invaderX[i] < 12 || invaderX[i] > 212) {
              hitWall = true;
            }
          }
        }
        if (hitWall) {
          invaderDir = -invaderDir;
          for (int i = 0; i < 10; i++) {
            if (invaderAlive[i]) {
              invaderY[i] += 8;
              if (invaderY[i] >= 195) {
                gameOver4 = true;
                audio.playSound(SOUND_GAMEOVER);
              }
            }
          }
        }
      }

      if (!laserActive) {
        laserX = playerInvX + 10;
        laserY = 200;
        laserActive = true;
        audio.playSound(SOUND_CHIRP);
      } else {
        if (millis() - lastLaserUpdate > 30) {
          lastLaserUpdate = millis();
          laserY -= 6;
          for (int i = 0; i < 10; i++) {
            if (invaderAlive[i]) {
              if (laserX >= invaderX[i] && laserX <= invaderX[i] + 16 &&
                  laserY >= invaderY[i] && laserY <= invaderY[i] + 12) {
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

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SPACE:%03d", score4);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver4) {
      display.fillRect(playerInvX, 204, 20, 8, TFT_GREEN);
      display.fillRect(playerInvX + 8, 200, 4, 4, TFT_GREEN);

      if (laserActive) {
        display.drawFastVLine(laserX, laserY, 6, TFT_YELLOW);
      }

      for (int i = 0; i < 10; i++) {
        if (invaderAlive[i]) {
          display.fillRect(invaderX[i], invaderY[i], 16, 10, TFT_RED);
          display.fillRect(invaderX[i] + 4, invaderY[i] + 3, 2, 2, TFT_WHITE);
          display.fillRect(invaderX[i] + 10, invaderY[i] + 3, 2, 2, TFT_WHITE);
        }
      }
    } else {
      if (gameWon4) {
        display.setTextSize(3);
        display.setTextColor(TFT_GREEN, 0x0821);
        display.setCursor(44, 90);
        display.print("VICTORY!");

        display.setTextSize(2);
        display.setTextColor(TFT_WHITE, 0x0821);
        display.setCursor(56, 130);
        display.printf("FINAL SCORE: %d", score4);

        display.setTextSize(1);
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
        display.setCursor(40, 175);
        display.print("Press BTN1 to Re-Play");
        display.setCursor(40, 195);
        display.print("Press BTN2 to return to Menu");
      } else {
        drawGameOverScreen(display, score4);
      }
    }
  }

  void updateAndDrawPong(GFXcanvas16& display, LunaAudio& audio) {
    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver5) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        paddlePlayerY -= 4;
        if (paddlePlayerY < 58) paddlePlayerY = 58;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        paddlePlayerY += 4;
        if (paddlePlayerY > 192) paddlePlayerY = 192;
      }

      ballX += ballVX;
      ballY += ballVY;

      if (ballY <= 58) {
        ballY = 58;
        ballVY = -ballVY;
        audio.playSound(SOUND_CHIRP);
      } else if (ballY >= 218) {
        ballY = 218;
        ballVY = -ballVY;
        audio.playSound(SOUND_CHIRP);
      }

      if (ballVX < 0 && ballX <= 26 && ballX >= 20 && ballY >= paddlePlayerY && ballY <= paddlePlayerY + 30) {
        ballVX = -ballVX * 1.05f;
        ballVY += ((ballY - (paddlePlayerY + 15)) / 15.0f) * 2.0f;
        ballX = 27;
        audio.playSound(SOUND_JUMP);
      }

      if (ballVX > 0 && ballX >= 208 && ballX <= 214 && ballY >= paddleCpuY && ballY <= paddleCpuY + 30) {
        ballVX = -ballVX * 1.05f;
        ballVY += ((ballY - (paddleCpuY + 15)) / 15.0f) * 2.0f;
        ballX = 207;
        audio.playSound(SOUND_JUMP);
      }

      if (ballY > paddleCpuY + 15) {
        paddleCpuY += 2;
      } else if (ballY < paddleCpuY + 15) {
        paddleCpuY -= 2;
      }
      paddleCpuY = constrain(paddleCpuY, 58, 192);

      if (ballX < 12) {
        scoreCpu++;
        audio.playSound(SOUND_POWERDOWN);
        if (scoreCpu >= 5) {
          gameOver5 = true;
          gameWon5 = false;
          audio.playSound(SOUND_GAMEOVER);
        } else {
          ballX = 120;
          ballY = 140;
          ballVX = 3.0f;
          ballVY = random(-15, 16) / 10.0f;
        }
      } else if (ballX > 228) {
        scorePlayer++;
        audio.playSound(SOUND_COIN);
        if (scorePlayer >= 5) {
          gameOver5 = true;
          gameWon5 = true;
          audio.playSound(SOUND_POWERUP);
        } else {
          ballX = 120;
          ballY = 140;
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

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("YOU:%d  CPU:%d", scorePlayer, scoreCpu);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver5) {
      for (int i = 58; i < 224; i += 10) {
        display.drawFastVLine(120, i, 5, 0x10A2);
      }

      display.fillRect(20, paddlePlayerY, 6, 30, TFT_CYAN);
      display.fillRect(214, paddleCpuY, 6, 30, TFT_RED);

      display.fillCircle((int)ballX, (int)ballY, 3, TFT_YELLOW);
    } else {
      if (gameWon5) {
        display.setTextSize(3);
        display.setTextColor(TFT_GREEN, 0x0821);
        display.setCursor(44, 90);
        display.print("VICTORY!");

        display.setTextSize(2);
        display.setTextColor(TFT_WHITE, 0x0821);
        display.setCursor(56, 130);
        display.printf("FINAL SCORE: %d", scorePlayer);

        display.setTextSize(1);
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
        display.setCursor(40, 175);
        display.print("Press BTN1 to Re-Play");
        display.setCursor(40, 195);
        display.print("Press BTN2 to return to Menu");
      } else {
        drawGameOverScreen(display, scorePlayer);
      }
    }
  }

  void updateAndDrawBreakout(GFXcanvas16& display, LunaAudio& audio) {
    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver6) {
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        breakPaddleX -= 5;
        if (breakPaddleX < 12) breakPaddleX = 12;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        breakPaddleX += 5;
        if (breakPaddleX > 192) breakPaddleX = 192;
      }

      breakBallX += breakBallVX;
      breakBallY += breakBallVY;

      if (breakBallX <= 14) {
        breakBallX = 14;
        breakBallVX = -breakBallVX;
        audio.playSound(SOUND_CHIRP);
      } else if (breakBallX >= 226) {
        breakBallX = 226;
        breakBallVX = -breakBallVX;
        audio.playSound(SOUND_CHIRP);
      }

      if (breakBallY <= 58) {
        breakBallY = 58;
        breakBallVY = -breakBallVY;
        audio.playSound(SOUND_CHIRP);
      }

      if (breakBallVY > 0 && breakBallY >= 200 && breakBallY <= 206 &&
          breakBallX >= breakPaddleX && breakBallX <= breakPaddleX + 36) {
        breakBallVY = -breakBallVY;
        breakBallVX = ((breakBallX - (breakPaddleX + 18)) / 18.0f) * 3.5f;
        audio.playSound(SOUND_JUMP);
      }

      if (breakBallY > 224) {
        lives6--;
        audio.playSound(SOUND_POWERDOWN);
        if (lives6 <= 0) {
          gameOver6 = true;
          gameWon6 = false;
          audio.playSound(SOUND_GAMEOVER);
        } else {
          breakBallX = breakPaddleX + 18;
          breakBallY = 180;
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
            int bx = 20 + c * 40;
            int by = 70 + r * 14;
            if (breakBallX + 3 >= bx && breakBallX - 3 <= bx + 36 &&
                breakBallY + 3 >= by && breakBallY - 3 <= by + 10) {
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

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("BRK:%03d", score6);

    int lifeX = SCREEN_WIDTH - 54;
    for (int i = 0; i < 3; i++) {
      if (i < lives6) {
        display.fillCircle(lifeX + i * 14, 44, 4, TFT_RED);
      } else {
        display.fillCircle(lifeX + i * 14, 44, 4, TFT_DARKGREY);
      }
    }
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver6) {
      display.fillRect(breakPaddleX, 204, 36, 6, TFT_CYAN);
      display.fillCircle((int)breakBallX, (int)breakBallY, 3, TFT_YELLOW);

      uint16_t rowColors[] = {TFT_RED, TFT_ORANGE, TFT_GREEN};
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
          int idx = c + r * 5;
          if (brickActive[idx]) {
            int bx = 20 + c * 40;
            int by = 70 + r * 14;
            display.fillRect(bx, by, 36, 10, rowColors[r]);
            display.drawRect(bx, by, 36, 10, TFT_BLACK);
          }
        }
      }
    } else {
      if (gameWon6) {
        display.setTextSize(3);
        display.setTextColor(TFT_GREEN, 0x0821);
        display.setCursor(44, 90);
        display.print("VICTORY!");

        display.setTextSize(2);
        display.setTextColor(TFT_WHITE, 0x0821);
        display.setCursor(56, 130);
        display.printf("FINAL SCORE: %d", score6);

        display.setTextSize(1);
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
        display.setCursor(40, 175);
        display.print("Press BTN1 to Re-Play");
        display.setCursor(40, 195);
        display.print("Press BTN2 to return to Menu");
      } else {
        drawGameOverScreen(display, score6);
      }
    }
  }

  void updateAndDrawMemoryMatch(GFXcanvas16& display, LunaAudio& audio) {
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

    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("TRIES:%03d", score7);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver7) {
      for (int i = 0; i < 8; i++) {
        int col = i % 4;
        int row = i / 4;
        int cx = 22 + col * 50;
        int cy = 66 + row * 74;
        int cw = 42;
        int ch = 64;

        if (cardState[i] == 0) {
          display.fillRoundRect(cx, cy, cw, ch, 4, 0x10A2);
          display.drawRoundRect(cx, cy, cw, ch, 4, TFT_LIGHTGREY);
          display.drawCircle(cx + cw/2, cy + ch/2, 6, 0x07FF);
        } else {
          display.fillRoundRect(cx, cy, cw, ch, 4, TFT_BLACK);
          display.drawRoundRect(cx, cy, cw, ch, 4, cardState[i] == 2 ? TFT_GREEN : TFT_WHITE);
          int sx = cx + cw/2;
          int sy = cy + ch/2;
          int val = cards[i];
          if (val == 0) {
            display.fillCircle(sx - 4, sy - 4, 4, TFT_RED);
            display.fillCircle(sx + 4, sy - 4, 4, TFT_RED);
            display.fillTriangle(sx - 8, sy - 2, sx + 8, sy - 2, sx, sy + 7, TFT_RED);
          } else if (val == 1) {
            display.fillTriangle(sx, sy - 8, sx - 6, sy + 4, sx + 6, sy + 4, TFT_YELLOW);
            display.fillTriangle(sx, sy + 8, sx - 6, sy - 4, sx + 6, sy - 4, TFT_YELLOW);
          } else if (val == 2) {
            display.fillTriangle(sx, sy - 8, sx - 6, sy, sx + 6, sy, TFT_CYAN);
            display.fillTriangle(sx, sy + 8, sx - 6, sy, sx + 6, sy, TFT_CYAN);
          } else if (val == 3) {
            display.fillRect(sx - 6, sy - 6, 12, 12, TFT_ORANGE);
          }
        }

        if (i == cursorIndex && !matchingInProgress) {
          display.drawRoundRect(cx - 2, cy - 2, cw + 4, ch + 4, 6, TFT_YELLOW);
        }
      }
    } else {
      display.setTextSize(3);
      display.setTextColor(TFT_GREEN, 0x0821);
      display.setCursor(48, 70);
      display.print("VICTORY!");

      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, 0x0821);
      display.setCursor(56, 120);
      display.printf("TRIES:%d", score7);

      display.setTextSize(2);
      display.setTextColor(TFT_LIGHTGREY, 0x0821);
      display.setCursor(42, 160);
      display.print("BTN1:PLAY AGAIN");
      display.setCursor(12, 190);
      display.print("BTN2:BACK TO MENU");
    }
  }
};

#endif // GAMES_H
