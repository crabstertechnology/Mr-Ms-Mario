// =============================================================================
// quests.h  —  Real-World Activity & Quest Library Engine with Casino Roulette
// =============================================================================
#ifndef QUESTS_H
#define QUESTS_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Preferences.h>
#include "config.h"
#include "audio.h"

#ifndef TFT_WHITE
#define TFT_WHITE 0xFFFF
#endif

extern LunaAudio audio;
extern uint32_t lunaXP;
extern int lunaLevel;
extern uint32_t lunaFeedCount;
extern void saveLunaPetStats();
extern void sendLunaStatsToBLE();
extern void notifyScreenAndExprSync();
extern SmartwatchScreen currentScreen;

// ── Quest Item Definition ───────────────────────────────────────────────────
struct QuestItem {
  uint8_t catIndex;     // Category Index 0..8
  const char* title;    // Crisp action title
  const char* mission;  // Luna formula real-world mini mission
  const char* duration; // e.g. "2 MIN", "5 MIN", "60 SEC"
};

// ── Category Definition ─────────────────────────────────────────────────────
struct QuestCategory {
  const char* name;
  const char* tag;
  uint16_t color;
  uint8_t count;
};

// ── 9 Real-World Categories ─────────────────────────────────────────────────
static const QuestCategory QUEST_CATEGORIES[] = {
  { "Be Social",       "SOCIAL",   0x07FF, 12 }, // Cyan
  { "Look Around",     "OBSERVE",  0xFFE0, 12 }, // Yellow
  { "Quick Creative",  "CREATE",   0xFD20, 12 }, // Coral Orange
  { "Brain Teasers",   "BRAIN",    0xB81F, 12 }, // Neon Purple
  { "Body & Move",     "MOVE",     0x07E0, 12 }, // Spring Green
  { "Calm & Relax",    "CALM",     0x5EFA, 12 }, // Sky Blue
  { "Kind Deeds",      "KIND",     0xFDA0, 12 }, // Amber Gold
  { "Fun Chats",       "CHAT",     0x2695, 12 }, // Emerald
  { "Focus & Tidy",    "FOCUS",    0xF9A4, 12 }  // Crimson
};
static const int TOTAL_QUEST_CATEGORIES = 9;

// ── 108 Real-World Activities (Simple English, Distinct, General Audience) ───
static const QuestItem PROGMEM QUEST_ITEMS[] = {
  // ── Category 0: Be Social (12) ───────────────────────────────────────────
  { 0, "Say Hello and Smile", "Smile and say a friendly hello to the very next person you walk past.", "1 MIN" },
  { 0, "Give a Nice Compliment", "Tell someone nearby one nice thing you like about their outfit or smile.", "1 MIN" },
  { 0, "Say a Real Thank You", "Look someone in the eye and thank them sincerely for something they did.", "1 MIN" },
  { 0, "High-Five or Fist Bump", "Give a fun high-five or fist bump to a friend, family member, or coworker.", "30 SEC" },
  { 0, "Ask About Their Pet", "Ask a friend or neighbor if they have a pet, and what kind of animal it is.", "3 MIN" },
  { 0, "Wave to a Neighbor", "Give a friendly wave to a neighbor across the street or down the hall.", "30 SEC" },
  { 0, "Share a Snack or Drink", "Offer a piece of fruit, snack, or bottle of water to someone near you.", "2 MIN" },
  { 0, "Ask an Elder for Advice", "Ask an older person for their single best piece of advice for living well.", "5 MIN" },
  { 0, "3-Min Phone-Free Talk", "Put your phone in your pocket and talk with someone with zero distractions.", "3 MIN" },
  { 0, "Tell a Funny Real Story", "Share a silly or funny thing that happened to you recently to make them smile.", "3 MIN" },
  { 0, "Send a Friendly Text", "Send a quick message to a friend saying you hope they have a wonderful day.", "1 MIN" },
  { 0, "Introduce Two Friends", "Introduce two friends or coworkers who have not met each other yet.", "2 MIN" },

  // ── Category 1: Look Around (12) ──────────────────────────────────────────
  { 1, "Find 3 Bright Red Items", "Look around your space right now and point out 3 bright red objects.", "1 MIN" },
  { 1, "Hear 3 Distinct Sounds", "Close your eyes for 30 seconds. Listen closely and identify 3 distinct sounds.", "1 MIN" },
  { 1, "Gaze at the Clouds", "Look up at the sky or out a window. Watch the clouds drift for two minutes.", "2 MIN" },
  { 1, "Touch 3 Textures", "Feel something smooth like glass, something rough, and something cold metal.", "2 MIN" },
  { 1, "Spot a Bird or Insect", "Look through a window or outside until you spot a flying bird or tiny insect.", "2 MIN" },
  { 1, "Find the Oldest Item", "Look around the room and find the oldest item. Guess how many years old it is.", "2 MIN" },
  { 1, "Read an Overlooked Sign", "Find a poster, label, or warning sign you pass every day and read every word.", "2 MIN" },
  { 1, "Guess Time Before Looking", "Guess the exact hour and minute in your head, then check your watch to see!", "30 SEC" },
  { 1, "Count 10 Passing Vehicles", "Look outside at the street and count 10 cars, bikes, or buses passing by.", "3 MIN" },
  { 1, "Inspect a Green Leaf", "Look closely at a leaf on a plant or tree. Trace the tiny veins with your eye.", "2 MIN" },
  { 1, "Find a Star or Triangle", "Search your room for any pattern or object shaped like a star or triangle.", "2 MIN" },
  { 1, "Count People in View", "Count how many people are in the room or street in front of you right now.", "1 MIN" },

  // ── Category 2: Quick Creative (12) ───────────────────────────────────────
  { 2, "Fold a Paper Airplane", "Take a piece of scrap paper, fold a quick airplane, and test flight it!", "2 MIN" },
  { 2, "Doodle a Silly Face", "Grab any pen and scrap paper. Draw a funny laughing face with big ears.", "2 MIN" },
  { 2, "Stack 5 Coins High", "Find 5 coins or bottle caps. Balance them into a tall tower without dropping.", "2 MIN" },
  { 2, "Hum Your Favorite Song", "Hum or whistle the chorus of your favorite song for 30 seconds.", "1 MIN" },
  { 2, "Write a 5-Word Story", "Write down a complete, exciting story that uses strictly five words.", "2 MIN" },
  { 2, "Draw with Other Hand", "Hold a pen in your non-dominant hand and try drawing a cat or a car.", "2 MIN" },
  { 2, "Give an Object a Hero Name", "Look at your water bottle or stapler and give it a hilarious superhero name.", "1 MIN" },
  { 2, "Tap a Catchy Drum Beat", "Use your fingertips on a desk or your knees to tap a cool 4-beat rhythm.", "1 MIN" },
  { 2, "Balance a Pen on Finger", "Balance a pen or pencil horizontally on your index finger for 10 seconds.", "1 MIN" },
  { 2, "Write 3 Good Things", "Jot down 3 simple things you feel happy and grateful for right this minute.", "2 MIN" },
  { 2, "Origami Paper Cup or Boat", "Fold a square paper napkin or receipt into a tiny boat or triangle hat.", "3 MIN" },
  { 2, "Leave a Hidden Cheerful Note", "Write 'Keep smiling!' on a sticky note and leave it for a family member.", "2 MIN" },

  // ── Category 3: Brain Teasers (12) ────────────────────────────────────────
  { 3, "Remember 5 Items on Desk", "Look at 5 items on your table. Close your eyes and recite them in reverse!", "2 MIN" },
  { 3, "Alphabet from Z to A", "Recite the alphabet backwards starting from Z down to A without peaking.", "2 MIN" },
  { 3, "Count by 7s up to 70", "Count up by 7s in your head: 7, 14, 21, 28, 35, 42, 49, 56, 63, 70!", "1 MIN" },
  { 3, "Name 5 Animals with 'B'", "Name 5 different animals that start with the letter B as fast as you can.", "1 MIN" },
  { 3, "Recall Yesterday's Meals", "Remember exactly what you ate for breakfast, lunch, and dinner yesterday.", "2 MIN" },
  { 3, "Say 'Thank You' in 3 Ways", "Say 'Thank you' in Spanish (Gracias), French (Merci), or Japanese (Arigato)!", "1 MIN" },
  { 3, "Spell 3 Words Backwards", "Pick 3 everyday words around you and spell each of them backwards out loud.", "2 MIN" },
  { 3, "Count Down from 50 by 3s", "Count backwards: 50, 47, 44, 41, 38... see if you can reach 2 without error!", "2 MIN" },
  { 3, "Name 5 Capital Cities", "Name 5 capital cities around the world without looking at your phone.", "2 MIN" },
  { 3, "Remember 3 School Friends", "Picture 3 friends from your school days and recall one fun memory together.", "2 MIN" },
  { 3, "Picture Your Bedroom", "Close your eyes and visualize where every single item in your bedroom sits.", "2 MIN" },
  { 3, "Add Up a Plate Number", "Look at any license plate or door number and add all the digits together.", "1 MIN" },

  // ── Category 4: Body & Move (12) ──────────────────────────────────────────
  { 4, "Reach for the Sky Stretch", "Stand up tall, interlock fingers, and push both hands up toward the ceiling.", "1 MIN" },
  { 4, "Do 10 Easy Knee Bends", "Stand with feet apart and do 10 gentle squats to wake up your leg muscles.", "2 MIN" },
  { 4, "Roll Shoulders 10 Times", "Roll both shoulders backward 10 times to melt away tightness in your neck.", "1 MIN" },
  { 4, "Balance on One Foot", "Stand on your left foot for 15 seconds, then switch and balance on the right.", "1 MIN" },
  { 4, "Take 100 Brisk Steps", "Get up and walk 100 brisk steps around your room, hallway, or garden.", "2 MIN" },
  { 4, "Climb One Flight of Stairs", "Skip the elevator and walk up or down one full flight of stairs.", "2 MIN" },
  { 4, "Gentle Torso Twists", "Stand relaxed and gently swing your arms side to side, twisting your waist.", "1 MIN" },
  { 4, "Bend and Touch Your Shins", "Keep your legs straight and slowly bend forward to stretch your hamstrings.", "1 MIN" },
  { 4, "Shake Out Hands & Wrists", "Shake both hands and wrists loosely for 20 seconds to release tension.", "30 SEC" },
  { 4, "Do 15 Tiptoe Calf Lifts", "Stand tall and lift your heels off the floor 15 times to pump circulation.", "1 MIN" },
  { 4, "Step Out for 10 Breaths", "Open the front door or step on a balcony. Fill your lungs with fresh air.", "2 MIN" },
  { 4, "Pace Around While Talking", "Stand on your feet and walk around next time you speak with someone.", "3 MIN" },

  // ── Category 5: Calm & Relax (12) ─────────────────────────────────────────
  { 5, "5 Slow Box Breaths", "Breathe in for 4 seconds, hold for 4 seconds, and exhale slowly for 4 seconds.", "2 MIN" },
  { 5, "Drink Cold Water Slowly", "Drink a full glass of cool water slowly, savoring every single sip.", "2 MIN" },
  { 5, "Rest Eyes for 60 Seconds", "Close both eyes completely. Let your face relax and stay in total dark.", "1 MIN" },
  { 5, "Gaze at Furthest Point", "Look out a window at the furthest building or tree to rest your eye muscles.", "2 MIN" },
  { 5, "Sit in Complete Silence", "Rest your hands on your lap and sit in tranquil silence for two minutes.", "2 MIN" },
  { 5, "Splash Cold Water on Face", "Go to the sink and splash cool refreshing water on your face and eyes.", "2 MIN" },
  { 5, "Drop Jaw and Shoulders", "Notice if your teeth are clenched. Let your jaw drop and drop your shoulders.", "1 MIN" },
  { 5, "3-Minute Screen Holiday", "Turn off your phone and computer screen. Look at the real world for 3 mins.", "3 MIN" },
  { 5, "Feel Sunlight on Skin", "Stand near a sunny window or step outside to feel warm sunlight on your arms.", "2 MIN" },
  { 5, "Clear 1 Foot of Your Desk", "Wipe down and tidy one square foot of your desk so it is completely empty.", "2 MIN" },
  { 5, "Listen to Running Water", "Turn on the tap gently or listen to rain sounds. Let all busy thoughts fade.", "2 MIN" },
  { 5, "Lie Flat on Floor or Bed", "Lie flat on your back on a mat, bed, or rug and let your whole body relax.", "3 MIN" },

  // ── Category 6: Kind Deeds (12) ───────────────────────────────────────────
  { 6, "Hold the Door Open", "Wait a few seconds and hold the door open for the person coming behind you.", "1 MIN" },
  { 6, "Let Someone Go Ahead", "Let someone step in front of you in line, at an elevator, or in traffic.", "2 MIN" },
  { 6, "Pick Up One Piece of Trash", "Spot a piece of stray paper or plastic on the floor and put it in the bin.", "1 MIN" },
  { 6, "Refill the Water Pitcher", "Top up the water pitcher, kettle, or bottle so it is full for the next person.", "2 MIN" },
  { 6, "Praise Someone's Effort", "Tell someone you notice how hard they work and that they do a fantastic job.", "1 MIN" },
  { 6, "Wipe Down a Shared Sink", "Take a paper towel and wipe down the bathroom or kitchen sink tidy and dry.", "2 MIN" },
  { 6, "Feed a Bird or Family Pet", "Toss a few seeds or breadcrumbs for birds outside, or give a pet a treat.", "2 MIN" },
  { 6, "Message Your Family", "Send a loving message to your parents or family asking how their day is going.", "2 MIN" },
  { 6, "Put an Item Back in Place", "Find one object left lying around in the house and return it where it belongs.", "2 MIN" },
  { 6, "Thank a Hardworking Worker", "Warmly thank a delivery driver, security guard, cashier, or cleaning staff.", "1 MIN" },
  { 6, "Hold Lift Button for Someone", "Press and hold the elevator door open button if someone is walking toward it.", "1 MIN" },
  { 6, "Do a Secret Good Turn", "Do a helpful chore for someone at home or work without telling anyone you did it.", "3 MIN" },

  // ── Category 7: Fun Chats (12) ────────────────────────────────────────────
  { 7, "What Made You Smile Today?", "Ask a friend or coworker what was the brightest or funniest moment today.", "3 MIN" },
  { 7, "Recommend a Great Movie", "Tell someone about an awesome movie, show, or song you think they would love.", "3 MIN" },
  { 7, "Ask About Their First Job", "Discover something cool about a peer by asking what their very first job was.", "4 MIN" },
  { 7, "Share a Favorite Food Spot", "Tell someone about the tastiest local restaurant, cafe, or bakery you know.", "3 MIN" },
  { 7, "Ask About Their Dream Trip", "Ask a friend: 'If you could fly anywhere tomorrow, where would you go?'", "4 MIN" },
  { 7, "Trade a Handy Life Hack", "Share a neat trick you use on your phone or in daily life that saves time.", "3 MIN" },
  { 7, "Ask What Video They Loved", "Ask someone what fascinating video, book, or podcast they enjoyed lately.", "3 MIN" },
  { 7, "Say 'Good Morning' to 3", "Greet 3 different people with genuine energy and warmth before noon.", "2 MIN" },
  { 7, "Send a Warm Voice Note", "Record a warm 15-second voice note to a good friend instead of typing a text.", "1 MIN" },
  { 7, "Favorite Childhood Game?", "Ask someone what game or cartoon they were obsessed with when growing up.", "4 MIN" },
  { 7, "Learn a Local Secret Spot", "Ask someone: 'What is your favorite quiet corner or park in this city?'", "3 MIN" },
  { 7, "Give a Sincere Shout-Out", "Tell a friend or coworker one specific quality about them you really admire.", "2 MIN" },

  // ── Category 8: Focus & Tidy (12) ─────────────────────────────────────────
  { 8, "Phone in Drawer for 5 Min", "Tuck your phone out of sight in a drawer and focus on real life for 5 mins.", "5 MIN" },
  { 8, "Throw Away 3 Old Scraps", "Find 3 old receipts, used wrappers, or unwanted flyers and bin them.", "2 MIN" },
  { 8, "Unsubscribe from 2 Spams", "Open your email inbox and unsubscribe from 2 annoying newsletters you never read.", "2 MIN" },
  { 8, "Tidy a Messy Cable", "Roll up and tidy a messy phone charger or power cord lying on your desk.", "2 MIN" },
  { 8, "Write Today's Single Goal", "Write on a piece of paper the #1 most important task you must finish today.", "2 MIN" },
  { 8, "Focus on 1 Tab for 10 Min", "Close all other browser tabs and work on just one single document or task.", "10 MIN" },
  { 8, "Make Your Bed Neatly", "Pull up your sheets and straighten the pillows so your bed looks hotel-crisp.", "2 MIN" },
  { 8, "Delete 10 Blurry Photos", "Open your phone photo gallery and delete 10 blurry or useless screenshots.", "2 MIN" },
  { 8, "Line Up Shoes by Door", "Straighten all shoes and slippers by the doorway into a neat, tidy row.", "2 MIN" },
  { 8, "Knock Out a 2-Minute Chore", "Do that tiny chore you have been putting off all day and finish it right now!", "2 MIN" },
  { 8, "Clear Out Old Receipts", "Open your wallet or bag and toss out crumpled slips, wrappers, or old papers.", "2 MIN" },
  { 8, "Read 3 Pages of a Book", "Pick up a physical printed book and read 3 pages with your full attention.", "5 MIN" }
};
static const int TOTAL_QUEST_ITEMS = 108;

// ── UI States ───────────────────────────────────────────────────────────────
enum QuestUIState {
  QUEST_STATE_CATEGORIES = 0,
  QUEST_STATE_SPINNING,
  QUEST_STATE_CARD_VIEW,
  QUEST_STATE_COMPLETED
};

// =============================================================================
// LunaQuests Class
// =============================================================================
class LunaQuests {
private:
  QuestUIState state;
  int selectedCategory;
  int currentQuestIndex;
  float scrollY;
  float scrollVel;
  int prevTouchY;
  bool wasScrolling;

  // Casino Roulette Spin State
  unsigned long spinStartTime;
  unsigned long spinDuration;
  unsigned long lastSpinTickMs;
  int spinDisplayItem;
  int spinTargetItem;
  int spinTickDelay;
  
  // Completed Celebration State
  unsigned long completedStartTime;

  // Sequential Progression Tracking (Persisted in NVS Preferences)
  uint8_t categorySeqIndex[TOTAL_QUEST_CATEGORIES];
  bool prefsLoaded;

  void ensurePrefsLoaded() {
    if (prefsLoaded) return;
    prefsLoaded = true;
    Preferences prefs;
    if (prefs.begin("luna_quest", true)) {
      for (int i = 0; i < TOTAL_QUEST_CATEGORIES; i++) {
        char k[8];
        snprintf(k, sizeof(k), "s%d", i);
        categorySeqIndex[i] = prefs.getUChar(k, 0);
        if (categorySeqIndex[i] >= QUEST_CATEGORIES[i].count) {
          categorySeqIndex[i] = 0;
        }
      }
      prefs.end();
    } else {
      for (int i = 0; i < TOTAL_QUEST_CATEGORIES; i++) {
        categorySeqIndex[i] = 0;
      }
    }
  }

  void saveCategoryProgress(int catIdx) {
    if (catIdx < 0 || catIdx >= TOTAL_QUEST_CATEGORIES) return;
    Preferences prefs;
    if (prefs.begin("luna_quest", false)) {
      char k[8];
      snprintf(k, sizeof(k), "s%d", catIdx);
      prefs.putUChar(k, categorySeqIndex[catIdx]);
      prefs.end();
    }
  }

  // ── Typography Word-Wrap Helper using FreeSans (Smooth Vector Canvas Font) ──
  void wrapFreeFont(GFXcanvas16& display, const String& text, int x, int startBaselineY, int maxWidth, int maxLines, int lineSpacing, uint16_t color) {
    display.setTextColor(color);
    int line = 0;
    int startIdx = 0;
    int len = text.length();

    while (startIdx < len && line < maxLines) {
      while (startIdx < len && text.charAt(startIdx) == ' ') startIdx++;
      if (startIdx >= len) break;

      int endIdx = startIdx;
      int lastSpace = -1;
      while (endIdx <= len) {
        String sub = text.substring(startIdx, endIdx);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(sub.c_str(), 0, 0, &x1, &y1, &w, &h);
        if (w > (uint16_t)maxWidth) {
          break;
        }
        if (endIdx < len && text.charAt(endIdx) == ' ') {
          lastSpace = endIdx;
        }
        endIdx++;
      }

      int printEnd = (lastSpace > startIdx && endIdx <= len) ? lastSpace : (endIdx - 1);
      if (printEnd <= startIdx) printEnd = startIdx + 1;

      String lineStr = text.substring(startIdx, printEnd);
      display.setCursor(x, startBaselineY + (line * lineSpacing));
      display.print(lineStr);

      startIdx = (lastSpace > startIdx && printEnd == lastSpace) ? lastSpace + 1 : printEnd;
      line++;
    }
  }

public:
  LunaQuests() 
    : state(QUEST_STATE_CATEGORIES), selectedCategory(0), currentQuestIndex(0),
      scrollY(0), scrollVel(0), prevTouchY(0), wasScrolling(false),
      spinStartTime(0), spinDuration(1800), lastSpinTickMs(0),
      spinDisplayItem(0), spinTargetItem(0), spinTickDelay(40),
      completedStartTime(0), prefsLoaded(false) {
    for (int i = 0; i < TOTAL_QUEST_CATEGORIES; i++) {
      categorySeqIndex[i] = 0;
    }
  }

  void openCategoryBrowser() {
    state = QUEST_STATE_CATEGORIES;
    scrollY = 0;
    scrollVel = 0;
    wasScrolling = false;
  }

  QuestUIState getState() const { return state; }

  // ── Start Casino Roulette Spin for a Category (Sequential With Reel Effect) ──
  void startRoulette(int categoryIdx) {
    if (categoryIdx < 0 || categoryIdx >= TOTAL_QUEST_CATEGORIES) categoryIdx = 0;
    selectedCategory = categoryIdx;
    ensurePrefsLoaded();

    int catCount = QUEST_CATEGORIES[categoryIdx].count;
    int firstIdx = 0;
    for (int i = 0; i < categoryIdx; i++) {
      firstIdx += QUEST_CATEGORIES[i].count;
    }

    // Pick the next sequential activity in this category (0 -> 1 -> 2 -> ... -> 11 -> 0)
    int targetOffset = categorySeqIndex[categoryIdx];
    spinTargetItem = firstIdx + targetOffset;

    // Advance sequence for next roll or reroll and persist to NVS
    categorySeqIndex[categoryIdx] = (targetOffset + 1) % catCount;
    saveCategoryProgress(categoryIdx);

    // Start reel cycling from an offset position so it spins dynamically across items
    spinDisplayItem = firstIdx + ((targetOffset + catCount - 3) % catCount);
    
    state = QUEST_STATE_SPINNING;
    spinStartTime = millis();
    spinDuration = 1800; // 1.8s exciting spin
    lastSpinTickMs = millis();
    spinTickDelay = 35;  // rapid initial reel tick
    audio.playSound(SOUND_CHIRP);
  }

  void triggerReroll() {
    startRoulette(selectedCategory);
  }

  // ── Update Spin Animation / Transitions ───────────────────────────────────
  void update() {
    if (state == QUEST_STATE_SPINNING) {
      unsigned long now = millis();
      unsigned long elapsed = now - spinStartTime;

      if (elapsed >= spinDuration) {
        spinDisplayItem = spinTargetItem;
        currentQuestIndex = spinTargetItem;
        state = QUEST_STATE_CARD_VIEW;
        audio.playSound(SOUND_POWERUP); // Winning fanfare!
        Serial.printf("[QUEST] Roulette landed on #%d: %s\n", 
                      categorySeqIndex[selectedCategory], 
                      QUEST_ITEMS[currentQuestIndex].title);
      } else {
        float progress = (float)elapsed / (float)spinDuration;
        spinTickDelay = 35 + (int)(progress * progress * 260); // Easing deceleration

        if (now - lastSpinTickMs >= (unsigned long)spinTickDelay) {
          lastSpinTickMs = now;
          int firstIdx = 0;
          for (int i = 0; i < selectedCategory; i++) {
            firstIdx += QUEST_CATEGORIES[i].count;
          }
          int catCount = QUEST_CATEGORIES[selectedCategory].count;

          if (progress > 0.85f) {
            // Smooth final landing onto the target item in the center window!
            spinDisplayItem = spinTargetItem;
          } else {
            spinDisplayItem = firstIdx + ((spinDisplayItem - firstIdx + 1) % catCount);
          }
          audio.playSound(SOUND_CHIRP);
        }
      }
    } else if (state == QUEST_STATE_COMPLETED) {
      if (millis() - completedStartTime > 2200) {
        currentScreen = SCREEN_FACE;
        state = QUEST_STATE_CATEGORIES;
        notifyScreenAndExprSync();
      }
    }
  }

  // ── Touch and Scroll Input Handling ───────────────────────────────────────
  void handleTouch(int touchX, int touchY, int ev = 0) {
    if (state == QUEST_STATE_CATEGORIES) {
      // Top Bar: Exit button at top-left (X < 70, Y < 38)
      if (touchY < 38 && touchX < 70) {
        currentScreen = SCREEN_FACE;
        audio.playSound(SOUND_POWERDOWN);
        notifyScreenAndExprSync();
        return;
      }

      // Tapping on large category cards (Y between 40 and 275)
      if (touchY >= 40 && touchY <= 275) {
        float effectiveY = (float)touchY - 40.0f + scrollY;
        int catIdx = (int)(effectiveY / 66.0f);
        if (catIdx >= 0 && catIdx < TOTAL_QUEST_CATEGORIES) {
          startRoulette(catIdx);
        }
      }
    } else if (state == QUEST_STATE_CARD_VIEW) {
      // 1. Back button (Top Left: X < 64, Y < 36)
      if (touchY < 36 && touchX < 64) {
        state = QUEST_STATE_CATEGORIES;
        audio.playSound(SOUND_CHIRP);
        return;
      }

      // 2. Reroll button (Left side: X <= 116, Y: 216..276)
      if (touchY >= 216 && touchY <= 276 && touchX <= 116) {
        triggerReroll();
        return;
      }

      // 3. DONE button (Right side: X > 116, Y: 216..276)
      if (touchY >= 216 && touchY <= 276 && touchX > 116) {
        lunaXP += 50;
        lunaFeedCount++;
        saveLunaPetStats();
        sendLunaStatsToBLE();
        completedStartTime = millis();
        state = QUEST_STATE_COMPLETED;
        audio.playSound(SOUND_POWERUP);
        Serial.printf("[QUEST] Completed: %s! +50 XP awarded.\n", QUEST_ITEMS[currentQuestIndex].title);
        return;
      }
    }
  }

  void handleScroll(float dy) {
    if (state != QUEST_STATE_CATEGORIES) return;
    const float MAX_SCROLL = (TOTAL_QUEST_CATEGORIES * 66.0f) - 232.0f;
    scrollY -= dy;
    if (scrollY < 0) scrollY = 0;
    if (scrollY > MAX_SCROLL) scrollY = MAX_SCROLL;
  }

  // ── Render Method ─────────────────────────────────────────────────────────
  void draw(GFXcanvas16& display, int hour, int minute) {
    update();

    if (state == QUEST_STATE_CATEGORIES) {
      drawCategoriesScreen(display, hour, minute);
    } else if (state == QUEST_STATE_SPINNING) {
      drawRouletteScreen(display);
    } else if (state == QUEST_STATE_CARD_VIEW) {
      drawQuestCardScreen(display);
    } else if (state == QUEST_STATE_COMPLETED) {
      drawCompletedScreen(display);
    }

    // Always reset font to default to protect all other screens
    display.setFont(NULL);
    display.setTextSize(1);
  }

private:
  // ── Screen 1: Category Browser (Large Spacious Cards + Canvas Font) ───────
  void drawCategoriesScreen(GFXcanvas16& display, int hour, int minute) {
    display.fillScreen(0x0000); // Pure AMOLED black

    // 1. Top Header Bar
    display.fillRoundRect(6, 6, 56, 26, 6, 0x2124);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(TFT_WHITE);
    display.setCursor(12, 23);
    display.print("< EXIT");

    display.setTextColor(0x07FF);
    display.setCursor(76, 23);
    display.print("QUESTS");

    // Shake hint pill (pure ASCII text)
    display.fillRoundRect(164, 6, 70, 26, 13, 0x10A2);
    display.drawRoundRect(164, 6, 70, 26, 13, 0xFD20);
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(0xFD20);
    display.setCursor(170, 15);
    display.print("SHAKE ROLL");

    display.drawFastHLine(0, 36, SCREEN_WIDTH, 0x2124);

    // 2. Large Category Cards (Height = 58px, Pitch = 66px)
    int startY = 40 - (int)scrollY;
    for (int i = 0; i < TOTAL_QUEST_CATEGORIES; i++) {
      int cardY = startY + (i * 66);
      if (cardY + 58 < 36 || cardY > SCREEN_HEIGHT) continue;

      const QuestCategory& cat = QUEST_CATEGORIES[i];

      // Frosted Glass Card Container
      display.fillRoundRect(8, cardY, SCREEN_WIDTH - 16, 58, 12, 0x0841);
      display.drawRoundRect(8, cardY, SCREEN_WIDTH - 16, 58, 12, cat.color);
      display.drawRoundRect(9, cardY + 1, SCREEN_WIDTH - 18, 56, 11, 0x2186);

      // Left Accent Color Pillar
      display.fillRoundRect(8, cardY, 6, 58, 3, cat.color);

      // Category Number Badge Pill: [01], [02] ...
      display.fillRoundRect(20, cardY + 14, 28, 28, 7, 0x10A2);
      display.drawRoundRect(20, cardY + 14, 28, 28, 7, cat.color);
      display.setFont(NULL);
      display.setTextSize(1);
      display.setTextColor(cat.color);
      display.setCursor(26, cardY + 24);
      char numBuf[8];
      snprintf(numBuf, sizeof(numBuf), "%02d", i + 1);
      display.print(numBuf);

      // Category Title in Smooth Canvas FreeSansBold9pt7b Font!
      display.setFont(&FreeSansBold9pt7b);
      display.setTextColor(TFT_WHITE);
      display.setCursor(54, cardY + 28);
      display.print(cat.name);

      // Subtitle Pill in Smooth FreeSans9pt7b Font
      display.setFont(&FreeSans9pt7b);
      display.setTextColor(cat.color);
      display.setCursor(54, cardY + 48);
      char subBuf[32];
      snprintf(subBuf, sizeof(subBuf), "%d Real Quests", cat.count);
      display.print(subBuf);

      // Right Chevron Button
      display.setFont(NULL);
      display.setTextSize(2);
      display.setTextColor(cat.color);
      display.setCursor(SCREEN_WIDTH - 24, cardY + 22);
      display.print(">");
    }

    // Modern Scrollbar Track & Thumb
    const float MAX_SCROLL = (TOTAL_QUEST_CATEGORIES * 66.0f) - 232.0f;
    if (MAX_SCROLL > 0) {
      int barH = 36;
      int trackH = SCREEN_HEIGHT - 48;
      int barY = 40 + (int)((scrollY / MAX_SCROLL) * (trackH - barH));
      display.fillRoundRect(SCREEN_WIDTH - 4, barY, 3, barH, 1, 0x5AEB);
    }
  }

  // ── Screen 2: Casino Slot Machine Roulette Screen ─────────────────────────
  void drawRouletteScreen(GFXcanvas16& display) {
    display.fillScreen(0x0000);

    const QuestCategory& cat = QUEST_CATEGORIES[selectedCategory];

    // 1. Top Header: Category Tag
    display.fillRoundRect(12, 8, SCREEN_WIDTH - 24, 28, 8, 0x10A2);
    display.drawRoundRect(12, 8, SCREEN_WIDTH - 24, 28, 8, cat.color);
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(cat.color);
    int16_t x1, y1;
    uint16_t tw, th;
    display.getTextBounds(cat.name, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor((SCREEN_WIDTH - tw) / 2, 26);
    display.print(cat.name);

    // 2. Casino Slot Reel Housing
    int boxX = 8;
    int boxY = 42;
    int boxW = SCREEN_WIDTH - 16;
    int boxH = 186;
    
    // Golden casino frame with double outline
    display.fillRoundRect(boxX, boxY, boxW, boxH, 12, 0x0841);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 12, 0xFFE0); // Gold
    display.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 11, 0xFDA0);

    // Slot Title
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(0xFFE0);
    const char* slotTitle = "- LUNA ROULETTE -";
    display.getTextBounds(slotTitle, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor((SCREEN_WIDTH - tw) / 2, boxY + 22);
    display.print(slotTitle);

    // Glowing Center Selection Bar (The Winner Window)
    int winY = boxY + 44;
    int winH = 88;
    display.fillRoundRect(boxX + 6, winY, boxW - 12, winH, 10, 0x10A2);
    display.drawRoundRect(boxX + 6, winY, boxW - 12, winH, 10, 0xFFE0);
    display.drawRoundRect(boxX + 7, winY + 1, boxW - 14, winH - 2, 9, cat.color);

    // Pulsing side pointer arrows
    display.setFont(NULL);
    display.setTextSize(2);
    display.setTextColor(0xFFE0);
    display.setCursor(boxX + 10, winY + 34);
    display.print(">");
    display.setCursor(boxX + boxW - 22, winY + 34);
    display.print("<");

    // The Currently Spinning Item in Center Window with FreeSansBold9pt7b!
    const QuestItem& item = QUEST_ITEMS[spinDisplayItem];
    display.setFont(&FreeSansBold9pt7b);
    wrapFreeFont(display, item.title, boxX + 26, winY + 28, boxW - 52, 2, 22, TFT_WHITE);

    // Duration badge below text (Pure ASCII)
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(cat.color);
    char dBuf[24];
    snprintf(dBuf, sizeof(dBuf), "TIME: %s", item.duration);
    display.setCursor(boxX + 26, winY + 76);
    display.print(dBuf);

    // Rolling ticker hints
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(0x632C);
    display.setCursor(boxX + 14, boxY + boxH - 30);
    display.print("... ROLLING REAL QUESTS ...");

    // 3. Bottom Animated Spin Indicator Dots
    int dotCount = 5;
    int dotSpacing = 16;
    int dotStartX = (SCREEN_WIDTH - (dotCount * dotSpacing)) / 2;
    int activeDot = (millis() / 100) % dotCount;
    for (int d = 0; d < dotCount; d++) {
      uint16_t dCol = (d == activeDot) ? 0xFFE0 : 0x3186;
      display.fillCircle(dotStartX + (d * dotSpacing), 246, (d == activeDot) ? 4 : 2, dCol);
    }

    display.setTextSize(1);
    display.setTextColor(0x8410);
    const char* hint = "LOCKING MISSION...";
    int hW = strlen(hint) * 6;
    display.setCursor((SCREEN_WIDTH - hW) / 2, 262);
    display.print(hint);
  }

  // ── Screen 3: Quest Card View (Spacious Layout + Canvas Typography) ───────
  void drawQuestCardScreen(GFXcanvas16& display) {
    display.fillScreen(0x0000);

    const QuestItem& quest = QUEST_ITEMS[currentQuestIndex];
    const QuestCategory& cat = QUEST_CATEGORIES[quest.catIndex];

    // 1. Top Bar: Back Button & Category Pill & Duration Pill
    display.fillRoundRect(8, 8, 48, 24, 6, 0x2124);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(TFT_WHITE);
    display.setCursor(14, 24);
    display.print("<");

    // Category Tag Pill
    display.fillRoundRect(60, 8, 108, 24, 6, 0x10A2);
    display.drawRoundRect(60, 8, 108, 24, 6, cat.color);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(cat.color);
    display.setCursor(66, 24);
    display.print(cat.tag);

    // Duration Pill (Pure ASCII)
    display.fillRoundRect(172, 8, 60, 24, 6, 0x2124);
    display.drawRoundRect(172, 8, 60, 24, 6, 0x07E0);
    display.setTextColor(0x07E0);
    display.setCursor(176, 24);
    display.print(quest.duration);

    // 2. Main Quest Card Container
    int cardX = 8;
    int cardY = 38;
    int cardW = SCREEN_WIDTH - 16;
    int cardH = 178;

    display.fillRoundRect(cardX, cardY, cardW, cardH, 12, 0x0841);
    display.drawRoundRect(cardX, cardY, cardW, cardH, 12, cat.color);
    display.drawRoundRect(cardX + 1, cardY + 1, cardW - 2, cardH - 2, 11, 0x2186);

    // Mission Title in Bold Smooth FreeSansBold9pt7b!
    display.setFont(&FreeSansBold9pt7b);
    wrapFreeFont(display, quest.title, cardX + 12, cardY + 24, cardW - 24, 2, 22, TFT_WHITE);

    // Divider Line
    display.drawFastHLine(cardX + 10, cardY + 70, cardW - 20, cat.color);

    // Action Prompt Heading
    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(0x8CD6);
    display.setCursor(cardX + 12, cardY + 78);
    display.print("HOW TO COMPLETE:");

    // Real-World Mini Mission Instructions in Smooth FreeSans9pt7b!
    display.setFont(&FreeSans9pt7b);
    wrapFreeFont(display, quest.mission, cardX + 12, cardY + 102, cardW - 24, 4, 18, TFT_WHITE);

    // 3. Bottom Action Buttons (Balanced Layout & Non-Overflowing Typography)
    int btnY = 224;
    int btnH = 46;

    // Left Button: REROLL (X: 8, W: 106, H: 46)
    int rBtnX = 8;
    int rBtnW = 106;
    display.fillRoundRect(rBtnX, btnY, rBtnW, btnH, 10, 0x2124);
    display.drawRoundRect(rBtnX, btnY, rBtnW, btnH, 10, 0xFD20);
    display.setFont(NULL);
    display.setTextSize(2);
    display.setTextColor(0xFD20);
    int rw = 6 * 12; // "REROLL" is 72px wide
    display.setCursor(rBtnX + (rBtnW - rw) / 2, btnY + 9);
    display.print("REROLL");

    display.setTextSize(1);
    display.setTextColor(0xC618);
    const char* rSub = "NEW QUEST";
    int rsw = strlen(rSub) * 6;
    display.setCursor(rBtnX + (rBtnW - rsw) / 2, btnY + 29);
    display.print(rSub);

    // Right Button: DONE (X: 122, W: 110, H: 46)
    int dBtnX = 122;
    int dBtnW = 110;
    display.fillRoundRect(dBtnX, btnY, dBtnW, btnH, 10, 0x04C0); // Vibrant Emerald Green
    display.drawRoundRect(dBtnX, btnY, dBtnW, btnH, 10, 0x07E0);
    display.setFont(NULL);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);
    int dw = 4 * 12; // "DONE" is 48px wide
    display.setCursor(dBtnX + (dBtnW - dw) / 2, btnY + 9);
    display.print("DONE");

    display.setTextSize(1);
    display.setTextColor(0xFFE0);
    const char* xpReward = "+50 XP";
    int xpW = strlen(xpReward) * 6;
    display.setCursor(dBtnX + (dBtnW - xpW) / 2, btnY + 29);
    display.print(xpReward);
  }

  // ── Screen 4: Celebration Complete Screen ─────────────────────────────────
  void drawCompletedScreen(GFXcanvas16& display) {
    display.fillScreen(0x0000);

    // Confetti / Star Sparkles
    for (int i = 0; i < 20; i++) {
      int sx = (i * 37 + (millis() / 40)) % SCREEN_WIDTH;
      int sy = (i * 23 + (millis() / 25)) % SCREEN_HEIGHT;
      uint16_t sc = (i % 3 == 0) ? 0xFFE0 : ((i % 3 == 1) ? 0x07E0 : 0xFD20);
      display.drawPixel(sx, sy, sc);
      display.drawPixel(sx + 1, sy, sc);
      display.drawPixel(sx, sy + 1, sc);
    }

    int boxX = 12;
    int boxY = 28;
    int boxW = SCREEN_WIDTH - 24;
    int boxH = 220;

    display.fillRoundRect(boxX, boxY, boxW, boxH, 16, 0x0841);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 16, 0x07E0);
    display.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 15, 0xFFE0);

    // Vector Geometric Trophy Badge (No corrupted unicode character!)
    int cx = SCREEN_WIDTH / 2;
    int cy = 72;
    display.fillCircle(cx, cy, 24, 0xFD80);
    display.drawCircle(cx, cy, 24, TFT_WHITE);
    display.drawCircle(cx, cy, 21, 0xFFE0);
    // Draw vector star inside trophy circle
    display.fillTriangle(cx, cy - 14, cx + 11, cy + 8, cx - 11, cy + 8, TFT_WHITE);
    display.fillTriangle(cx, cy + 12, cx + 11, cy - 7, cx - 11, cy - 7, TFT_WHITE);

    // Title in FreeSansBold9pt7b
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(0x07E0);
    const char* txt1 = "QUEST DONE!";
    int16_t x1, y1;
    uint16_t tw, th;
    display.getTextBounds(txt1, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor((SCREEN_WIDTH - tw) / 2, 122);
    display.print(txt1);

    // XP Award Pill
    display.fillRoundRect(cx - 60, 138, 120, 32, 10, 0x04C0);
    display.drawRoundRect(cx - 60, 138, 120, 32, 10, 0x07E0);
    display.setTextColor(TFT_WHITE);
    const char* xpTxt = "+50 XP EARNED";
    display.getTextBounds(xpTxt, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor((SCREEN_WIDTH - tw) / 2, 160);
    display.print(xpTxt);

    // Subtitles
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(0x8CD6);
    const char* txt2 = "Real World Connected";
    display.getTextBounds(txt2, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor((SCREEN_WIDTH - tw) / 2, 196);
    display.print(txt2);

    display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(0x632C);
    const char* txt3 = "Returning to companion...";
    int w3 = strlen(txt3) * 6;
    display.setCursor((SCREEN_WIDTH - w3) / 2, 224);
    display.print(txt3);
  }
};

#endif // QUESTS_H
