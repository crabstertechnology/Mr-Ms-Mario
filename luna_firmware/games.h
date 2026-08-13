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

public:
  LunaGames() {
    resetCoinCatcher();
    resetFlappyMochy();
    lastBtn1State = false;
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

  void drawMenu(GFXcanvas16& display) {
    // Clear screen with LUNA_DARK
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF);

    // Header
    display.setTextSize(2);
    display.setTextColor(0xFDA0, 0x0821); // Orange
    display.setCursor(32, 38);
    display.print("LUNA ARCADE");
    display.drawFastHLine(12, 58, SCREEN_WIDTH - 24, 0x18E3);

    // Menu options
    display.setTextSize(2);
    int startY = 76;
    int spacing = 36;

    for (int i = 0; i < 3; i++) {
      int yPos = startY + i * spacing;
      bool isCurrent = (gameMenuOption == i);

      if (isCurrent) {
        display.fillRoundRect(12, yPos, SCREEN_WIDTH - 24, 30, 6, 0x18E3); // Highlight
        display.drawRoundRect(12, yPos, SCREEN_WIDTH - 24, 30, 6, 0x07FF);
        display.setTextColor(TFT_WHITE, 0x18E3);
      } else {
        display.setTextColor(TFT_LIGHTGREY, 0x0821);
      }

      display.setCursor(20, yPos + 7);
      if (i == 0) {
        display.print("1. COIN CATCHER");
      } else if (i == 1) {
        display.print("2. FLAPPY MOCHY");
      } else {
        display.print("3. EXIT");
      }
    }

    display.setTextSize(1);
    display.setTextColor(TFT_DARKGREY, 0x0821);
    display.setCursor(24, SCREEN_HEIGHT - 20);
    display.print("BTN1: Cycle | BTN2: Select");
  }

  void updateAndDrawCoinCatcher(GFXcanvas16& display, LunaAudio& audio) {
    // --- Physics & Collision ---
    if (!gameOver1) {
      // 1. Read Button inputs for real-time player movement
      if (digitalRead(BTN_EXPR_PIN) == LOW) {
        playerX -= 8;
        if (playerX < 12) playerX = 12;
      }
      if (digitalRead(BTN_SETTINGS_PIN) == LOW) {
        playerX += 8;
        if (playerX > SCREEN_WIDTH - 12 - playerWidth) playerX = SCREEN_WIDTH - 12 - playerWidth;
      }

      // 2. Fall coin
      coinY += (int)coinSpeed;

      // 3. Collision check
      if (coinY + 8 >= playerY && coinY - 8 <= playerY + playerHeight &&
          coinX + 8 >= playerX && coinX - 8 <= playerX + playerWidth) {
        // Catch!
        audio.playSound(SOUND_COIN);
        score1++;
        coinSpeed += 0.35f;
        if (coinSpeed > 9.0f) coinSpeed = 9.0f;
        coinY = 36;
        coinX = random(24, SCREEN_WIDTH - 24);
      }
      // 4. Miss check
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
      // If Game Over, check if BTN1 is pressed to restart
      bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);
      if (currentBtn1 && !lastBtn1State) {
        resetCoinCatcher();
      }
      lastBtn1State = currentBtn1;
    }

    // --- Rendering ---
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0x07FF); // Cyan frame

    // Score board
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SCORE:%03d", score1);

    // Draw lives (as small filled circles or simple hearts)
    int lifeX = SCREEN_WIDTH - 54;
    for (int i = 0; i < 3; i++) {
      if (i < lives1) {
        // Red heart representation: two tiny circles and a triangle
        int hx = lifeX + i * 14;
        int hy = 42;
        display.fillCircle(hx - 3, hy - 2, 3, TFT_RED);
        display.fillCircle(hx + 3, hy - 2, 3, TFT_RED);
        display.fillTriangle(hx - 6, hy, hx + 6, hy, hx, hy + 6, TFT_RED);
      } else {
        // Gray placeholder
        int hx = lifeX + i * 14;
        int hy = 42;
        display.fillCircle(hx, hy + 1, 4, TFT_DARKGREY);
      }
    }
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (!gameOver1) {
      // Draw falling coin
      display.fillCircle(coinX, coinY, 8, 0xFDA0); // Golden coin
      display.drawCircle(coinX, coinY, 8, TFT_WHITE);
      display.drawCircle(coinX, coinY, 5, TFT_YELLOW);

      // Draw player ship/basket
      display.fillRoundRect(playerX, playerY, playerWidth, playerHeight, 4, 0x07FF);
      display.fillRoundRect(playerX + 4, playerY + 3, playerWidth - 8, playerHeight - 6, 2, TFT_WHITE);
      
      // Small decoration
      display.fillRect(playerX + playerWidth/2 - 2, playerY - 3, 4, 3, 0xF8B8);
    } else {
      // Game Over Screen
      display.setTextSize(3);
      display.setTextColor(TFT_RED, 0x0821);
      display.setCursor(44, 90);
      display.print("GAME OVER");

      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, 0x0821);
      display.setCursor(56, 130);
      display.printf("FINAL SCORE: %d", score1);

      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, 0x0821);
      display.setCursor(40, 175);
      display.print("Press BTN1 to Re-Play");
      display.setCursor(40, 195);
      display.print("Hold BTN1 to return to Menu");
    }
  }

  void updateAndDrawFlappyMochy(GFXcanvas16& display, LunaAudio& audio) {
    // --- Input & Physics ---
    bool currentBtn1 = (digitalRead(BTN_EXPR_PIN) == LOW);

    if (!gameOver2) {
      // Flap triggers on edge press
      if (currentBtn1 && !lastBtn1State) {
        if (!gameStarted2) {
          gameStarted2 = true;
        }
        birdVelocity = jumpStrength;
        audio.playSound(SOUND_JUMP);
      }

      if (gameStarted2) {
        // Apply physics
        birdVelocity += gravity;
        birdY += birdVelocity;

        // Ground/ceiling collisions
        if (birdY + birdRadius >= SCREEN_HEIGHT - 12) {
          birdY = SCREEN_HEIGHT - 12 - birdRadius;
          gameOver2 = true;
          audio.playSound(SOUND_GAMEOVER);
        }
        if (birdY - birdRadius <= 32) {
          birdY = 32 + birdRadius;
          birdVelocity = 0.0f;
        }

        // Pipe scrolling
        pipeX -= (int)pipeSpeed;
        if (pipeX + pipeWidth < 12) {
          // Passed!
          pipeX = SCREEN_WIDTH - 12;
          gapY = random(75, SCREEN_HEIGHT - 85);
          score2++;
          audio.playSound(SOUND_COIN);
          pipeSpeed += 0.18f;
          if (pipeSpeed > 7.5f) pipeSpeed = 7.5f;
        }

        // Pipe collision check
        if (birdX + birdRadius >= pipeX && birdX - birdRadius <= pipeX + pipeWidth) {
          if (birdY - birdRadius <= gapY - gapHeight/2 || birdY + birdRadius >= gapY + gapHeight/2) {
            gameOver2 = true;
            audio.playSound(SOUND_GAMEOVER);
          }
        }
      }
    } else {
      // Restart on press
      if (currentBtn1 && !lastBtn1State) {
        resetFlappyMochy();
      }
    }
    lastBtn1State = currentBtn1;

    // --- Rendering ---
    display.fillRoundRect(6, 30, SCREEN_WIDTH - 12, SCREEN_HEIGHT - 36, 10, 0x0821);
    display.drawRoundRect(4, 28, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 32, 10, 0xF8B8); // Pink frame

    // Score board
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, 0x0821);
    display.setCursor(16, 36);
    display.printf("SCORE:%03d", score2);
    display.drawFastHLine(12, 54, SCREEN_WIDTH - 24, 0x18E3);

    if (gameStarted2 && !gameOver2) {
      // Draw top pipe
      display.fillRect(pipeX, 30, pipeWidth, gapY - gapHeight/2 - 30, TFT_GREEN);
      display.drawRect(pipeX, 30, pipeWidth, gapY - gapHeight/2 - 30, 0x03E0); // Dark green border
      // Pipe lips
      display.fillRect(pipeX - 2, gapY - gapHeight/2 - 8, pipeWidth + 4, 8, TFT_GREEN);
      display.drawRect(pipeX - 2, gapY - gapHeight/2 - 8, pipeWidth + 4, 8, 0x03E0);

      // Draw bottom pipe
      display.fillRect(pipeX, gapY + gapHeight/2, pipeWidth, SCREEN_HEIGHT - 8 - (gapY + gapHeight/2), TFT_GREEN);
      display.drawRect(pipeX, gapY + gapHeight/2, pipeWidth, SCREEN_HEIGHT - 8 - (gapY + gapHeight/2), 0x03E0);
      // Pipe lips
      display.fillRect(pipeX - 2, gapY + gapHeight/2, pipeWidth + 4, 8, TFT_GREEN);
      display.drawRect(pipeX - 2, gapY + gapHeight/2, pipeWidth + 4, 8, 0x03E0);
    }

    // Draw Bird (cute yellow round character)
    display.fillCircle((int)birdX, (int)birdY, birdRadius, TFT_YELLOW);
    display.drawCircle((int)birdX, (int)birdY, birdRadius, 0x7E00); // Dark yellow outline
    
    // Eye
    display.fillCircle((int)birdX + 3, (int)birdY - 3, 2, TFT_WHITE);
    display.fillCircle((int)birdX + 4, (int)birdY - 3, 1, TFT_BLACK);
    
    // Orange beak
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
      // Game Over Screen
      display.setTextSize(3);
      display.setTextColor(TFT_RED, 0x0821);
      display.setCursor(44, 90);
      display.print("GAME OVER");

      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, 0x0821);
      display.setCursor(56, 130);
      display.printf("FINAL SCORE: %d", score2);

      display.setTextSize(1);
      display.setTextColor(TFT_LIGHTGREY, 0x0821);
      display.setCursor(40, 175);
      display.print("Press BTN1 to Re-Play");
      display.setCursor(40, 195);
      display.print("Hold BTN1 to return to Menu");
    }
  }
};

#endif // GAMES_H
