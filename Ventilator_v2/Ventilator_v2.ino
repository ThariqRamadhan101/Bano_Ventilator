// Description : This sketch is for ventilator that was made to deal with COVID -19

// LIBRARY
// OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Joystick
int button_Pin = 2;
int x_Pin = A0;
int y_Pin = A1;
int x_Position = 0;
int y_Position = 0;
int buttonState = 1;
// Servo
#include <Servo.h>
Servo myservo;
#define enable_motor true // useful for debugging without noise
#define servo_pin 3

// STATE
#define MODE 0
#define SETTING 1
#define ON 2
static int state = MODE;
//// STATE -> MODE
//#define PAPR 3
//#define NIV 4
//static int mode = PAPR;


// DEFAULT VARIABLE
// Menu

int posCur[3] = {18, 33, 48};  // Position cursor in Y dimension
int posX = 0;
int posY = 0;
String optionMode[2] = {"PAPR", "NIV"};
String mode =  optionMode[0];
String optionSet[3] = {"BR", "PEEP", "SMAX"};
#define max_speed 180
#define min_speed 0
int target_speed_low = 0;
int BR[5] = {10, 15, 20, 25, 35};
int br = BR[0];
int PEEP[5] = {35, 40, 45, 50, 55};
int peep = PEEP[0];
int SMAX[5] = {60, 70, 80, 90, 100};
int smax = SMAX[0];
int brPos = 0;
int peepPos = 0;
int smaxPos = 0;
// Debounce
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 500;    // the debounce time; increase if the output flickers
long lastCursorX = 0;
long lastCursorY = 0;
// cycle
int cycle_counter = 0;
int cycle_phase = 0;

void setup() {
  // Call Serial Monitor
  Serial.begin(9600);

  // Call OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);


  // Call Joystick
  pinMode(x_Pin, INPUT);
  pinMode(y_Pin, INPUT);

  // Call Servo
  if (enable_motor) {
    myservo.attach(servo_pin, 1000, 2000); // some motors need min/max setting
    Serial.print("Initializing ESC...");
  } else {
    Serial.println("Motor disabled.  Bypassing initialization");
  }


  // Call interrupt
  pinMode(button_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button_Pin), changeState, RISING);

  // Call state because there is intteruption when upload
  state = MODE;
}

void loop() {
  FSM();

}

void FSM() {
  switch (state) {
    case MODE :
      // Display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Mode:");
      for (int i = 0; i < 2; i++) {
        display.setCursor(15, posCur[i]);
        display.print(optionMode[i]);
      }

      // Choose option
      y_Position = analogRead(y_Pin);
      if (y_Position < 10) { // DOWN
        posY = cursorY(posY, 2, "DOWN");
      }
      if (y_Position > 1013) { // UP
        posY = cursorY(posY, 2, "UP");
      }
      display.setCursor(0, posCur[posY]);
      display.print(">");
      mode = optionMode[posY];

      display.display();
      break;
    case SETTING :
      // Display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Set ");
      display.print(mode);
      display.print(":");
      if (mode == "PAPR") {
        display.setCursor(15, posCur[0]);
        display.print(optionSet[2]);
        display.setCursor(60, posCur[0]);
        display.print(":");
        display.setCursor(0, posCur[0]);
        display.print(">");
        // Choose Option
        x_Position = analogRead(x_Pin);
        if (x_Position < 10) { // LEFT
          posX = cursorX(posX, "LEFT");
        }
        if (x_Position > 1013) { // RIGHT
          posX = cursorX(posX, "RIGHT");
        }
        display.setCursor(70, posCur[0]);
        display.print(SMAX[posX]);

        br = 0;
        smax = SMAX[posX];
        peep = smax;
      }
      if (mode == "NIV") {
        for (int i = 0; i < 3; i++) {
          display.setCursor(15, posCur[i]);
          display.print(optionSet[i]);
          display.setCursor(60, posCur[i]);
          display.print(":");
          // Choose Option
          for (int i = 0; i < 3; i++) {
            display.setCursor(15, posCur[i]);
            display.print(optionSet[i]);
          }
          // Choose option
          y_Position = analogRead(y_Pin);
          if (y_Position < 10) { // DOWN
            posY = cursorY(posY, 3, "DOWN");
          }
          if (y_Position > 1013) { // UP
            posY = cursorY(posY, 3, "UP");
            posX = 0;
          }
          display.setCursor(0, posCur[posY]);
          display.print(">");
          switch (posY) {
            case 0 :
              posX = brPos;
              break;
            case 1 :
              posX = peepPos;
              break;
            case 2 :
              posX = smaxPos;
              break;
          }

          x_Position = analogRead(x_Pin);
          if (x_Position < 10) { // LEFT
            posX = cursorX(posX, "LEFT");
          }
          if (x_Position > 1013) { // RIGHT
            posX = cursorX(posX, "RIGHT");
          }
          switch (posY) {
            case 0 :
              br = BR[posX];
              brPos = posX;
              break;
            case 1 :
              peep = PEEP[posX];
              peepPos = posX;
              break;
            case 2 :
              smax = SMAX[posX];
              smaxPos = posX;
              break;
          }
          display.setCursor(70, posCur[0]);
          display.print(br);
          display.setCursor(70, posCur[1]);
          display.print(peep);
          display.setCursor(70, posCur[2]);
          display.print(smax);
        }
      }

      display.display();
      break;
    case ON :

      cycle_counter += 1;
      if ( (30 * 100) / br < cycle_counter) {
        cycle_phase = (cycle_phase + 1) % 2;
        cycle_counter = 0;
        Serial.print("\tphase speed:");
        if (cycle_phase == 0) {
          if (enable_motor)
            myservo.write(smax);
          Serial.println(smax);
        } else {
          if (enable_motor)
            myservo.write(peep);
          Serial.println(peep);
        }
      }
      // Display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Mode ");
      display.print(mode);
      for (int i = 0; i < 3; i++) {
        display.setCursor(0, posCur[i]);
        display.print(optionSet[i]);
        display.setCursor(45, posCur[i]);
        display.print(":");
      }
      display.setCursor(55, posCur[0]);
      display.print(br);
      display.setCursor(55, posCur[1]);
      display.print(peep);
      display.setCursor(55, posCur[2]);
      display.print(smax);
      display.display();

      // TO DO COMUNICATION
      break;
  }
}

void changeState() {
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    posX = 0;
    posY = 0;
    switch (state) {
      case MODE :
        state = SETTING;
        break;
      case SETTING :
        state = ON;
        break;
      case ON :
        state = MODE;
        break;
    }
    lastDebounceTime = millis();
  }
}

int cursorX(int pos, String opt) {
  if ((millis() - lastCursorX) > (debounceDelay)) {
    if (opt == "LEFT") {
      pos--;
    }
    if (opt == "RIGHT") {
      pos++;
    }
    pos = pos % 5;
    if (pos < 0) { // because with abs cannot dunno why
      pos = 5 + pos;
    }
    Serial.println(pos);
    lastCursorX = millis();
  }
  return pos;
}

int cursorY(int pos, int maxOpt, String opt) {
  if ((millis() - lastCursorY) > debounceDelay) {
    if (opt == "DOWN") {
      pos--;
    }
    if (opt == "UP") {
      pos++;
    }
    pos = pos % maxOpt;
    if (pos < 0) { // because with abs cannot dunno why
      pos = maxOpt + pos;
    }
    Serial.println(pos);
    lastCursorY = millis();
  }
  return pos;
}
