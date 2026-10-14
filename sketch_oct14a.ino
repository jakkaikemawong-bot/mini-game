#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BTN_PIN 18

// สถานะเกม
enum GameState {
  STATE_START,
  STATE_RUNNING,
  STATE_BOSS_FIGHT,
  STATE_GAME_OVER,
  STATE_VICTORY
};
GameState gameState = STATE_START;

// ตัวแปรไดโนเสาร์
float dinoX = 15;
float dinoY = 44;
float dinoVy = 0;
const float gravity = 0.6;
const float jumpForce = -6.5;
const int dinoW = 12;
const int dinoH = 12;
int animFrame = 0;
unsigned long lastAnimTime = 0;

// สิ่งกีดขวาง (กระบองเพชร 3 แบบ)
float obsX = 128;
int obsType = 0; 
int obsWidths[] = {8, 10, 10};
int obsHeights[] = {12, 14, 16};
float obsSpd = 3.0; // ความเร็วคงที่
bool scored = false;

// คะแนนและสถิติ
int score = 0;
int highScore = 0;

// ระบบบอส
int bossIndex = 0; 
int playerHp = 100;
int bossHp = 50;
int bossMaxHp = 50;
const char* bossNames[] = {"Raptorsaur", "Tyrant Rex", "Tri-Scourge"};
int bossHps[] = {50, 100, 150};

// ตัวแปรกล่องจังหวะสู้บอส
float qteX = 128;
float qteSpd = 3.5;
bool qteActive = false;
bool boxProcessed = false; // ตัวแปรเช็คสถานะการกดกล่อง QTE
unsigned long gameOverTimer = 0;

// บิตแมปไดโนเสาร์ (วิ่ง 2 เฟรม)
const unsigned char PROGMEM dino_run1[] = {
  0b00001111, 0b110000,
  0b00001111, 0b111000,
  0b00001111, 0b111100,
  0b00001110, 0b000000,
  0b00111111, 0b110000,
  0b01111111, 0b111000,
  0b11111111, 0b111100,
  0b11111111, 0b111100,
  0b01100110, 0b011000,
  0b01100110, 0b011000,
  0b11000011, 0b001100,
  0b10000001, 0b000000
};

const unsigned char PROGMEM dino_run2[] = {
  0b00001111, 0b110000,
  0b00001111, 0b111000,
  0b00001111, 0b111100,
  0b00001110, 0b000000,
  0b00111111, 0b110000,
  0b01111111, 0b111000,
  0b11111111, 0b111100,
  0b11111111, 0b111100,
  0b01100110, 0b011000,
  0b01100110, 0b011000,
  0b00111100, 0b000000,
  0b01100011, 0b000000
};

// บิตแมปบอส 3 ระดับ
const unsigned char PROGMEM boss_1_bmp[] = {
  0b00000000, 0b00000000,
  0b00000011, 0b11000000,
  0b00000111, 0b11100000,
  0b00001111, 0b11110000,
  0b00001110, 0b00000000,
  0b00111111, 0b11000000,
  0b01111111, 0b11100000,
  0b11111111, 0b11110000,
  0b11111111, 0b11110000,
  0b01100110, 0b01100000,
  0b01100110, 0b01100000,
  0b11000011, 0b00110000,
  0b11000011, 0b00110000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

const unsigned char PROGMEM boss_2_bmp[] = {
  0b00000001, 0b10000000,
  0b00000011, 0b11000000,
  0b00000111, 0b11100000,
  0b00011111, 0b11111000,
  0b00111111, 0b11111100,
  0b01111111, 0b11111110,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b01101110, 0b01101100,
  0b01101110, 0b01101100,
  0b11001101, 0b10011000,
  0b11000000, 0b00001100,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

const unsigned char PROGMEM boss_3_bmp[] = {
  0b00000011, 0b11000000,
  0b00000111, 0b11100000,
  0b00001111, 0b11110000,
  0b00011111, 0b11111000,
  0b00111111, 0b11111100,
  0b01111111, 0b11111110,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b01111111, 0b11111110,
  0b01100110, 0b01100110,
  0b11000011, 0b00001100,
  0b11000011, 0b00001100,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

// บิตแมปกระบองเพชร 3 แบบ
const unsigned char PROGMEM cactus_1[] = {
  0b00011000, 0b00111100, 0b10011010, 0b11011011, 
  0b11011011, 0b00011000, 0b00011000, 0b00011000, 
  0b00011000, 0b00011000, 0b00011000, 0b00011000
};

const unsigned char PROGMEM cactus_2[] = {
  0b00100100, 0b00111100, 0b10111101, 0b11111111, 
  0b11011011, 0b00111100, 0b00111100, 0b00111100, 
  0b00111100, 0b00111100, 0b00111100, 0b00111100,
  0b00111100, 0b00111100
};

const unsigned char PROGMEM cactus_3[] = {
  0b01010101, 0b01111110, 0b11111111, 0b11111111, 
  0b11011011, 0b01111110, 0b01111110, 0b01111110, 
  0b01111110, 0b01111110, 0b01111110, 0b01111110,
  0b01111110, 0b01111110, 0b01111110, 0b01111110
};

void resetGame();
void initBoss();
void triggerGameOver();

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();
  randomSeed(analogRead(0));
}

void loop() {
  bool btnPressed = (digitalRead(BTN_PIN) == LOW);
  unsigned long currentMillis = millis();

  switch (gameState) {
    case STATE_START: {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(24, 15);
      display.print(F("DINO JUMP"));
      display.setCursor(20, 30);
      display.print(F("High Score: "));
      display.print(highScore);
      display.setCursor(18, 48);
      display.print(F("Press to start"));
      display.display();

      if (btnPressed) {
        resetGame();
        gameState = STATE_RUNNING;
        delay(250);
      }
      break;
    }

    case STATE_RUNNING: {
      if (btnPressed && dinoY >= 44) {
        dinoVy = jumpForce;
      }

      dinoVy += gravity;
      dinoY += dinoVy;
      if (dinoY > 44) {
        dinoY = 44;
        dinoVy = 0;
      }

      if (dinoY >= 44 && currentMillis - lastAnimTime > 120) {
        animFrame = !animFrame;
        lastAnimTime = currentMillis;
      }

      obsX -= obsSpd;
      if (obsX < -10) {
        obsX = 128;
        obsType = random(0, 3);
        scored = false;
      }

      if (!scored && obsX < dinoX) {
        score++;
        scored = true;
        
        if (score % 10 == 0 && bossIndex < 3) {
          initBoss();
          gameState = STATE_BOSS_FIGHT;
          delay(300);
          break;
        }
      }

      if (obsX < (dinoX + dinoW - 2) && (obsX + obsWidths[obsType] - 2) > dinoX &&
          (56 - obsHeights[obsType]) < (dinoY + dinoH) && 56 > dinoY) {
        triggerGameOver();
      }

      display.clearDisplay();
      display.drawLine(0, 56, 127, 56, SSD1306_WHITE);

      display.setTextSize(1);
      display.setCursor(85, 2);
      display.print(F("Scr:"));
      display.print(score);

      if (dinoY < 44) {
        display.drawBitmap((int)dinoX, (int)dinoY, dino_run1, dinoW, dinoH, SSD1306_WHITE);
      } else {
        if (animFrame == 0) display.drawBitmap((int)dinoX, (int)dinoY, dino_run1, dinoW, dinoH, SSD1306_WHITE);
        else display.drawBitmap((int)dinoX, (int)dinoY, dino_run2, dinoW, dinoH, SSD1306_WHITE);
      }

      if (obsType == 0) display.drawBitmap((int)obsX, 56 - obsHeights[0], cactus_1, obsWidths[0], obsHeights[0], SSD1306_WHITE);
      else if (obsType == 1) display.drawBitmap((int)obsX, 56 - obsHeights[1], cactus_2, obsWidths[1], obsHeights[1], SSD1306_WHITE);
      else display.drawBitmap((int)obsX, 56 - obsHeights[2], cactus_3, obsWidths[2], obsHeights[2], SSD1306_WHITE);

      display.display();
      break;
    }

    case STATE_BOSS_FIGHT: {
      display.clearDisplay();
      
      display.drawLine(0, 50, 127, 50, SSD1306_WHITE);

      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("P:")); display.print(playerHp);
      display.setCursor(75, 0);
      display.print(bossNames[bossIndex]);

      display.drawRect(75, 9, 50, 6, SSD1306_WHITE);
      int bossBarW = (bossHp * 50) / bossMaxHp;
      display.fillRect(75, 9, bossBarW, 6, SSD1306_WHITE);

      if ((currentMillis / 200) % 2 == 0) {
        display.drawBitmap(40, 38, dino_run1, dinoW, dinoH, SSD1306_WHITE);
      } else {
        display.drawBitmap(40, 38, dino_run2, dinoW, dinoH, SSD1306_WHITE);
      }

      if (bossIndex == 0) {
        display.drawBitmap(95, 34, boss_1_bmp, 16, 16, SSD1306_WHITE);
      } else if (bossIndex == 1) {
        display.drawBitmap(95, 34, boss_2_bmp, 16, 16, SSD1306_WHITE);
      } else {
        display.drawBitmap(95, 34, boss_3_bmp, 16, 16, SSD1306_WHITE);
      }

      for (int i = 0; i < 5; i++) {
        int rx = random(0, 128);
        int ry = random(15, 45);
        display.drawPixel(rx, ry, SSD1306_WHITE);
      }
      if (random(0, 10) > 7) {
        display.drawLine(random(20, 100), 0, random(20, 100), 25, SSD1306_WHITE);
      }

      qteX -= qteSpd;
      if (qteX > 60) {
        boxProcessed = false; // รีเซ็ตสถานะเมื่อกล่องเกิดใหม่ทางขวา
      }

      // ตรวจสอบการกดปุ่มช่วงจังหวะถูกต้อง
      if (btnPressed) {
        if (!boxProcessed && qteX >= 35 && qteX <= 50) {
          bossHp -= 15;
          boxProcessed = true;
          delay(150);
        } else if (!boxProcessed) {
          playerHp -= 20;
          boxProcessed = true;
          delay(150);
        }
      }

      // เงื่อนไข: ถ้ากล่องเลื่อนผ่านพ้นโซนไปแล้ว (น้อยกว่า 35) โดยที่เราไม่กดปุ่ม เลือดจะลด
      if (!boxProcessed && qteX < 35) {
        playerHp -= 20;
        boxProcessed = true;
      }

      if (qteX < -10) {
        qteX = 128;
        boxProcessed = false;
      }

      display.drawRect((int)qteX, 52, 10, 8, SSD1306_WHITE);
      display.drawFastVLine(45, 50, 14, SSD1306_WHITE);

      if (bossHp <= 0) {
        bossIndex++;
        if (bossIndex >= 3) {
          gameState = STATE_VICTORY;
        } else {
          gameState = STATE_RUNNING;
          obsX = 128;
          delay(1000);
        }
      } else if (playerHp <= 0) {
        triggerGameOver();
      }

      display.display();
      break;
    }

    case STATE_GAME_OVER: {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(34, 18);
      display.print(F("Game Over"));
      display.setCursor(31, 32);
      display.print(F("Score: "));
      display.print(score);

      display.drawLine(0, 0, random(50, 100), random(30, 60), SSD1306_WHITE);
      display.drawLine(128, 64, random(30, 80), random(10, 40), SSD1306_WHITE);
      display.display();

      if (millis() - gameOverTimer > 2000) {
        gameState = STATE_START;
      }
      break;
    }

    case STATE_VICTORY: {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(36, 18);
      display.print(F("YOU WIN!"));
      display.setCursor(25, 34);
      display.print(F("Final Score:"));
      display.setCursor(50, 48);
      display.print(score);
      display.display();

      if (btnPressed) {
        gameState = STATE_START;
        delay(300);
      }
      break;
    }
  }

  delay(25);
}

void resetGame() {
  dinoY = 44;
  dinoVy = 0;
  obsX = 128;
  score = 0;
  bossIndex = 0;
  playerHp = 100;
  scored = false;
}

void initBoss() {
  bossHp = bossHps[bossIndex];
  bossMaxHp = bossHps[bossIndex];
  qteX = 128;
  boxProcessed = false;
}

void triggerGameOver() {
  if (score > highScore) {
    highScore = score;
  }
  gameOverTimer = millis();
  gameState = STATE_GAME_OVER;
}