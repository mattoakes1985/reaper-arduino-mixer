#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Encoders and buttons
Encoder encoder1(2, 3);
Encoder encoder2(5, 6);
const int button1Pin = 4;
const int button2Pin = 7;

// Control mode
bool controlPan = false;
int lastEncoder1 = 0;

// Track display values
String trackName = "Waiting...";
int volValue = 0;
int panValue = 64;

// Bus selection state
bool busSelected = false;
int selectedBus = 0;

// Debounce timing for encoder2 jog
static int lastRaw2 = 0;
static unsigned long lastTime2 = 0;
const unsigned long ENC2_DEBOUNCE_MS = 50;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed! Check wiring.");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED OK");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void loop() {
  if (!busSelected) {
    handleBusSelection();
  } else {
    handleEncoder1();
    handleEncoder2();
    readSerial();
  }
}

void handleBusSelection() {
  int newPos = encoder2.read() / 4;
  if (newPos != selectedBus) {
    selectedBus = max(0, newPos);
    drawBusSelection();
  }

  static bool lastBtn = HIGH;
  bool currentBtn = (digitalRead(button2Pin) == LOW);
  if (lastBtn == HIGH && currentBtn == LOW) {
    Serial.print("SELECTBUS:");
    Serial.println(selectedBus);
    busSelected = true;
    display.clearDisplay();
    // reset encoder2 raw count
    lastRaw2 = encoder2.read();
  }
  lastBtn = currentBtn;
}

void drawBusSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Select Bus to Control");
  display.setTextSize(2);
  display.setCursor(20, 30);
  display.print("Bus ");
  display.println(selectedBus);
  display.display();
}

void handleEncoder1() {
  int newPos = encoder1.read() / 4;
  if (newPos != lastEncoder1) {
    int delta = lastEncoder1 - newPos;
    lastEncoder1 = newPos;
    if (controlPan) {
      Serial.print("SETPAN:");
    } else {
      Serial.print("SETVOL:");
    }
    Serial.println(delta);
  }

  static bool lastBtn1 = HIGH;
  bool currentBtn1 = (digitalRead(button1Pin) == LOW);
  if (lastBtn1 == HIGH && currentBtn1 == LOW) controlPan = !controlPan;
  lastBtn1 = currentBtn1;
}

void handleEncoder2() {
  // Rotary detent with debounce for track jog
  int raw = encoder2.read();
  unsigned long now = millis();
  if (raw - lastRaw2 >= 4 && now - lastTime2 >= ENC2_DEBOUNCE_MS) {
    Serial.println("JOG:1");
    lastRaw2 += 4;
    lastTime2 = now;
  } else if (raw - lastRaw2 <= -4 && now - lastTime2 >= ENC2_DEBOUNCE_MS) {
    Serial.println("JOG:-1");
    lastRaw2 -= 4;
    lastTime2 = now;
  }

  static bool lastBtn2 = HIGH;
  bool currentBtn2 = (digitalRead(button2Pin) == LOW);
  if (lastBtn2 == HIGH && currentBtn2 == LOW) {
    if (busSelected) {
      Serial.println("TOGGLEBUS");
    }
  }
  lastBtn2 = currentBtn2;
}

void readSerial() {
  static String buffer = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseMessage(buffer);
      buffer = "";
    } else if (c != '\r') {
      buffer += c;
    }
  }
}

void parseMessage(const String &msg) {
  int nameStart = msg.indexOf("NAME:");
  int volStart = msg.indexOf("|VOL:");
  int panStart = msg.indexOf("|PAN:");

  if (nameStart != -1 && volStart != -1 && panStart != -1) {
    trackName = msg.substring(nameStart + 5, volStart);
    volValue = msg.substring(volStart + 5, panStart).toInt();
    panValue = msg.substring(panStart + 5).toInt();
    drawTrack();
  }
}

void drawTrack() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(trackName);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Vol: ");
  float db = -60.0 + (72.0 * volValue / 127.0);
  if (volValue == 0) {
    display.print("-inf");
  } else {
    display.print(db, 1);
    display.print("dB");
  }

  display.drawRect(0, 30, 100, 8, SSD1306_WHITE);
  display.fillRect(1, 31, volValue * 100 / 127, 6, SSD1306_WHITE);

  display.setCursor(0, 42);
  display.print("Pan: ");
  int panPercent = map(panValue, 0, 127, -100, 100);
  if (panPercent < -10) display.print("L");
  else if (panPercent > 10) display.print("R");
  else display.print("C");
  display.print(abs(panPercent));
  display.print("%");

  int panX = map(panValue, 0, 127, 10, 117);
  display.setCursor(panX, 56);
  display.print("^");

  display.display();
}