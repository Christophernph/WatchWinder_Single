//Encoder:
// GND to GND
// + to 5V
// SW to A2
// DT to A0
// CLK to A1
//--------------------------
// OLED Display:
// GND to GND
// VCC to 5V
// SCL to A5
// SDA to A4
//--------------------------
// RTC Module:
// GND to GND
// VCC to 5V
// SCL to A5
// SDA to A4
//--------------------------
// TB6612 Stepper Driver:
// VM to 7.4V (0.28-0.6 A)
// VCC to 5V
// GND to GND
// PWMB to 5V
// BIN2 to PIN7
// BIN1 to PIN6
// STB to PIN3
// AIN1 to PIN5
// AIN2 to PIN4
// PWMA to 5V
//--------------------------
#include <Stepper.h>
#include <string.h>
#include <ClickEncoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3232RTC.h>

// DEFINES
// ----------------------------------------------------------------------------------------------------------------------
//Winder Class
#define CLKW 1
#define ACLKW 2
#define BDIR 0
const unsigned long MS_PER_DAY = 86400000; //86400000
#define TIMER_MS 4194 //4194
#define STB_PIN 3
#define WINDER_OFF 0
#define WINDER_ON 1
#define WINDER_NIGHT 2

//Stepper
#define STEPS 200
const unsigned long NUM_ROT = 10;
#define CYCLE_STEPS (NUM_ROT * STEPS)
#define STEPPER_SPEED 10

//Encoder
#define ENC_OPEN 0
#define ENC_HELD 3
#define ENC_RELEASE 4
#define ENC_CLICK 5
#define ENC_DCLICK 6

//Display
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16 // Necessary?
#define LOGO16_GLCD_WIDTH  16 // Necessary?

//Menu
#define TEXT_SIZE 1
#define Y_LINE1 3
#define Y_LINE2 13
#define Y_LINE3 23
#define TIMEOUT_MS (5 * TIMER_MS)
#define WINDER_STATUS 0
#define WINDER_TPD 1
#define WINDER_DIR 2

//System states
enum SYSTEM_STATES {
  SYSTEM_IDLE,
  SYSTEM_PRINT_MENU,
  SYSTEM_PRINT_CURSOR,
  SYSTEM_PRINT_SETTINGS,
  SYSTEM_MENU_IDLE,
  SYSTEM_UPDATE_TPD_PRINT,
  SYSTEM_UPDATE_TPD,
  SYSTEM_UPDATE_RTC_INIT,
  SYSTEM_UPDATE_RTC_PRINT,
  SYSTEM_UPDATE_RTC,
  SYSTEM_SAVE_RTC,
  SYSTEM_NIGHTMODE_PRINT,
  SYSTEM_NIGHTMODE_ADJUST,
  SYSTEM_HALT
};

//RTC_cursor positions
enum RTC_CURSOR {
  HOUR,
  MIN,
  SEC,
  DAY,
  MON,
  YEAR
};

const unsigned char BootBitmap [] PROGMEM = {
  // 'HASSØ, 128x32px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x1f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x03, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x01, 0x01, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x00, 0x30, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x38, 0x01, 0x00, 0x38, 0x00,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x30, 0x01, 0x00, 0x18, 0x00,
  0x00, 0x18, 0x30, 0x78, 0x3f, 0x1f, 0x83, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x41, 0x00, 0x1c, 0x00,
  0x00, 0x18, 0x30, 0x78, 0x31, 0x18, 0x86, 0x38, 0x00, 0x00, 0x00, 0x60, 0x61, 0x00, 0x1c, 0x00,
  0x00, 0x18, 0x30, 0x58, 0x70, 0x30, 0x0c, 0x3c, 0x00, 0x00, 0x00, 0x60, 0x19, 0x00, 0x0c, 0x00,
  0x00, 0x18, 0x30, 0xcc, 0x38, 0x38, 0x08, 0x4c, 0x00, 0x00, 0x00, 0x60, 0x0f, 0x00, 0x0c, 0x00,
  0x00, 0x1f, 0xf0, 0xcc, 0x3e, 0x1f, 0x18, 0xcc, 0x00, 0x00, 0x00, 0x60, 0x07, 0x80, 0x0c, 0x00,
  0x00, 0x1f, 0xf1, 0x86, 0x0f, 0x07, 0x98, 0x8c, 0x00, 0x00, 0x00, 0x60, 0x03, 0x00, 0x0c, 0x00,
  0x00, 0x18, 0x31, 0xfe, 0x03, 0x81, 0xc9, 0x8c, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x0c, 0x00,
  0x00, 0x18, 0x31, 0xfe, 0x01, 0x80, 0xcf, 0x0c, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x0c, 0x00,
  0x00, 0x18, 0x33, 0x03, 0x63, 0x31, 0x8f, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x18, 0x00,
  0x00, 0x18, 0x33, 0x03, 0x3f, 0x3f, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x18, 0x00,
  0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x70, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0xe0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x01, 0xc0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x07, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x3f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// GLOBAL VARIABLES
// ----------------------------------------------------------------------------------------------------------------------

//Stepper Motor
Stepper stepper(STEPS, 4, 5, 6, 7);

//Encoder
ClickEncoder *encoder;
int16_t last, value;
int button_state = ENC_OPEN;

//State
int system_state = SYSTEM_IDLE;

//Menu
unsigned long elapsed_time = TIMEOUT_MS;
int cursor_pos = 0;
int RTC_cursor = 0;

//Winder
unsigned long winder_time = 0;
int winder_status = 0;
int lastDirection = ACLKW;
unsigned long restTimeMS = 1;
unsigned long turns_per_day = 650; //86400 for 10 sec interval
int turn_dir = 0;
int winder_night_start = 10;
int winder_night_stop = 22;

//RTC
int nHour;
int nMinute;
int nSecond;
int nDay;
int nMonth;
int nYear;

// ISR
// ----------------------------------------------------------------------------------------------------------------------

// Update elapsed time for winder and timeout
ISR(TIMER1_OVF_vect) {
  if (winder_status != WINDER_OFF) {
    winder_time = winder_time + TIMER_MS;
  }
  if (elapsed_time < 2 * TIMEOUT_MS) {
    elapsed_time = elapsed_time + TIMER_MS;
  }
}

// Service encoder
ISR(TIMER2_COMPA_vect) {
  encoder->service();
}

// SETUP
// ----------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600);

  //Encoder
  encoder = new ClickEncoder(A1, A0, A2, 4);
  last = -1;

  //Stepper Motor speed
  stepper.setSpeed(STEPPER_SPEED);

  //Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
  display.clearDisplay();                     // Clear the buffer.

  //Boot bitmap
  display.drawBitmap(0, 0, BootBitmap, 128, 64, WHITE);
  display.display();
  delay(2000);
  display.clearDisplay();

  //TIMER1 (Winder Timer)
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);     //Prescaler of 1024
  TIMSK1 |= (1 << TOIE1);                                //Enable overflow interrupt
  TCNT1H = 0;                                            //Initialize timer at 0
  TCNT1L = 0;                                            //Initialize timer at 0

  //TIMER2 (Encoder Timer)
  TCCR2A = 0;
  TCCR2A |= (1 << WGM21) | (0 << WGM20);                 //Clear on compare match
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) | (0 << CS21) | (1 << CS20);     //Prescaler of 128
  OCR2A = 125;                                           //Load compare value to 125 (1 ms)
  TIMSK2 |= (1 << OCIE2A);                               //Enable compare match interrupt
  TCNT2 = 0;                                             //Initialize timer at 0

  //RTC
  setSyncProvider(RTC.get);

  //Winder
  updateRestTime();
}

// MAIN LOOP
// ----------------------------------------------------------------------------------------------------------------------
void loop() {
  //Per loop updating
  if (elapsed_time > TIMEOUT_MS) {  //timeout
    if (system_state != SYSTEM_UPDATE_RTC && system_state != SYSTEM_SAVE_RTC)  //prevent timeout when setting RTC
      system_state = SYSTEM_IDLE;
  }

  button_state = encoder->getButton();
  value += encoder->getValue();
  if (value != last) {
    elapsed_time = 0;     //reset timeout timer

    if (system_state != SYSTEM_UPDATE_TPD && system_state != SYSTEM_UPDATE_RTC && system_state != SYSTEM_NIGHTMODE_ADJUST) {
      last = value;
    }

    //Possibly different states depending on changing value
    if (system_state == SYSTEM_MENU_IDLE || system_state == SYSTEM_IDLE) {
      system_state = SYSTEM_PRINT_MENU;
    }
  }

  //System state switching
  switch (system_state) {
    case SYSTEM_IDLE:
      encoder->reset(); value = 0; last = 0;      //Encoder reset

      //Display idle
      display.clearDisplay();                     //empty display buffer
      displayText(getTime(), 18, 5, 2, false);
      displayText(getDate(), 35, 23, 1, false);
      display.display();

      cursor_pos = 0;

      runCycle();                                 //Check if winder should be run
      break;
    case SYSTEM_PRINT_MENU:
      display.clearDisplay();
      printMenu();
      system_state = SYSTEM_PRINT_CURSOR;
      break;
    case SYSTEM_PRINT_CURSOR:
      setCursorPos(value);
      printCursor(cursor_pos);
      system_state = SYSTEM_PRINT_SETTINGS;
      break;
    case SYSTEM_PRINT_SETTINGS:
      printSettings();
      display.display();
      system_state = SYSTEM_MENU_IDLE;
      break;
    case SYSTEM_MENU_IDLE:
      if (button_state != ENC_OPEN) {       //Button input
        elapsed_time = 0;                   //reset timeout timer

        //React to button input
        switch (button_state) {
          case ENC_CLICK:
            if (cursor_pos == WINDER_STATUS) {
              toggleWinderStatus();
              system_state = SYSTEM_PRINT_MENU;
            } else if (cursor_pos == WINDER_TPD) {
              system_state = SYSTEM_UPDATE_TPD_PRINT;
            } else if (cursor_pos == WINDER_DIR) {
              toggleWinderDir();
              system_state = SYSTEM_PRINT_MENU;
            }
            break;
          case ENC_DCLICK:
            system_state = SYSTEM_NIGHTMODE_PRINT;
            break;
          case ENC_HELD:
            system_state = SYSTEM_UPDATE_RTC_INIT;
            break;
          case ENC_RELEASE: break;
        }
      }
      break;
    case SYSTEM_UPDATE_TPD_PRINT: //TODO
      display.clearDisplay();
      printMenu();
      printSettings();
      displayText(">", 80, Y_LINE2, TEXT_SIZE, false);   //Show TPD is selected
      display.display();
      system_state = SYSTEM_UPDATE_TPD;
      break;
    case SYSTEM_UPDATE_TPD:

      // Encoder input
      if (value != last) {
        if (value > last) {
          turns_per_day += 10;
        } else {
          if (turns_per_day > 10) {
            turns_per_day -= 10;
          }
        }
        updateRestTime();
        last = value;
        system_state = SYSTEM_UPDATE_TPD_PRINT;
      }

      // Button input
      if (button_state == ENC_CLICK) {
        system_state = SYSTEM_PRINT_MENU;
      }
      break;
    case SYSTEM_UPDATE_RTC_INIT:
      // Load current time and date into integers.
      nHour = hour();
      nMinute = minute();
      nSecond = second();
      nDay    = day();
      nMonth  = month();
      nYear   = year();
      system_state = SYSTEM_UPDATE_RTC_PRINT;
      break;
    case SYSTEM_UPDATE_RTC_PRINT:
      display.clearDisplay();
      updateRTCPrint(nHour, nMinute, nSecond, nDay, nMonth, nYear);

      // displayText(getTime(), 18, 5, 2, false);
      // displayText(getDate(), 35, 23, 1, false);
      switch (RTC_cursor) {
        case HOUR:
          //displayText("-", 24, 14, 2, false);
          displayText("HOUR", 5, 23, 1, false);
          break;
        case MIN:
          //displayText("-", 59, 14, 2, false);
          displayText("MIN", 5, 23, 1, false);
          break;
        case SEC:
          //displayText("-", 96, 14, 2, false);
          displayText("SEC", 5, 23, 1, false);
          break;
        case DAY:
          //displayText("-", 38, 29, 1, false);
          displayText("DAY", 5, 23, 1, false);
          break;
        case MON:
          //displayText("-", 35, 23, 1, false);
          displayText("MON", 5, 23, 1, false);
          break;
        case YEAR:
          //displayText("-", 35, 23, 1, false);
          displayText("YEAR", 5, 23, 1, false);
          break;
      }
      display.display();
      system_state = SYSTEM_UPDATE_RTC;
      break;
    case SYSTEM_UPDATE_RTC:
      // Cancel set time
      if (button_state == ENC_DCLICK) {
        RTC_cursor = HOUR;
        system_state = SYSTEM_PRINT_MENU;
      }

      // Switch to next input
      if (button_state == ENC_CLICK) {
        RTC_cursor++;
        if (RTC_cursor <= YEAR) {
          system_state = SYSTEM_UPDATE_RTC_PRINT;
        } else {
          system_state = SYSTEM_SAVE_RTC;
        }
      }

      // Update current selection
      if (value != last) {
        switch (RTC_cursor) {
          case HOUR:
            nHour += value - last;
            if (nHour < 0) {
              nHour = 23;
            }
            if (nHour > 23) {
              nHour = 0;
            }
            break;
          case MIN:
            nMinute += value - last;
            if (nMinute < 0) {
              nMinute = 59;
            }
            if (nMinute > 59) {
              nMinute = 0;
            }
            break;
          case SEC:
            nSecond += value - last;
            if (nSecond < 0) {
              nSecond = 59;
            }
            if (nSecond > 59) {
              nSecond = 0;
            }
            break;
          case DAY:
            nDay += value - last;
            if (nDay < 1) {
              nDay = 31;
            }
            if (nDay > 31) {
              nDay = 1;
            }
            break;
          case MON:
            nMonth += value - last;
            if (nMonth < 1) {
              nMonth = 12;
            }
            if (nMonth > 12) {
              nMonth = 1;
            }
            break;
          case YEAR:
            nYear += value - last;
            if (nYear < 1970) {
              nYear = 1970;
            }
            if (nYear > 3000) {
              nYear = 3000;
            }
            break;
        }
        last = value;
        system_state = SYSTEM_UPDATE_RTC_PRINT;
      }
      break;
    case SYSTEM_SAVE_RTC:
      time_t t;
      tmElements_t tm;

      tm.Year   = CalendarYrToTm(nYear);
      tm.Month  = nMonth;
      tm.Day    = nDay;
      tm.Hour   = nHour;
      tm.Minute = nMinute;
      tm.Second = nSecond;
      t = makeTime(tm);
      RTC.set(t);
      setTime(t);

      RTC_cursor = HOUR;
      system_state = SYSTEM_MENU_IDLE;
      break;
    case SYSTEM_NIGHTMODE_PRINT:
      display.clearDisplay();
      displayText("START:", 18, 5, 1, false);
      displayText("STOP :", 18, 20, 1, false);
      displayText(time2String(winder_night_start), 70, 5, 1, false);
      displayText(time2String(winder_night_stop), 70, 20, 1, false);
      displayText(":00", 82, 5, 1, false);
      displayText(":00", 82, 20, 1, false);
      display.display();

      system_state = SYSTEM_NIGHTMODE_ADJUST;
      break;
    case SYSTEM_NIGHTMODE_ADJUST:
      if (button_state == ENC_CLICK || button_state == ENC_DCLICK) {
        system_state = SYSTEM_PRINT_MENU;
      }

      if (value != last) {
        winder_night_start += value - last;
        winder_night_stop += value - last;
        if (winder_night_start < 0) {
          winder_night_start = 23;
        }
        if (winder_night_start > 23) {
          winder_night_start = 0;
        }
        if (winder_night_stop < 0) {
          winder_night_stop = 23;
        }
        if (winder_night_stop > 23) {
          winder_night_stop = 0;
        }

        last = value;
        system_state = SYSTEM_NIGHTMODE_PRINT;
      }
      break;
    case SYSTEM_HALT: //Test
      break;
  }

}

// FUNCTIONS
// ----------------------------------------------------------------------------------------------------------------------
void displayText(String text, int x, int y, int size, boolean d) {

  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(text);
  if (d) {
    display.display();
  }
}

//Convert int time (sec, min, hour, day, month, year) into string
String time2String(int nTime) {
  String sTime;
  if (nTime < 10)
    sTime = "0" + String(nTime);
  else
    sTime = String(nTime);

  return sTime;
}

//returns current time (hour:min:sec) as string
String getTime() {
  int nHour   = hour();
  int nMinute = minute();
  int nSecond = second();

  String sHour = time2String(nHour);
  String sMinute = time2String(nMinute);
  String sSecond = time2String(nSecond);

  String sTime = sHour + ":" + sMinute + ":" + sSecond;
  return sTime;
}

//returns current date (day-month-year) as string
String getDate() {
  int nDay    = day();
  int nMonth  = month();
  int nYear   = year();

  String sDay = time2String(nDay);
  String sMonth = time2String(nMonth);
  String sYear = time2String(nYear);

  String sDate = sDay + "-" + sMonth + "-" + sYear;
  return sDate;
}

void updateRTCPrint(int H, int Min, int S, int D, int Mon, int Y) {
  String sHour = time2String(H);
  String sMinute = time2String(Min);
  String sSecond = time2String(S);
  String sDay = time2String(D);
  String sMonth = time2String(Mon);
  String sYear = time2String(Y);

  String sTime = sHour + ":" + sMinute + ":" + sSecond;
  String sDate = sDay + "-" + sMonth + "-" + sYear;

  displayText(sTime, 18, 5, 2, false);
  displayText(sDate, 35, 23, 1, false);
}

void printMenu() {

  //Line 1
  displayText("STATUS", 12, Y_LINE1, TEXT_SIZE, false);

  //Line 2
  displayText("TPD", 12, Y_LINE2, TEXT_SIZE, false);

  //Line 3
  displayText("DIR", 12, Y_LINE3, TEXT_SIZE, false);

  //display.display();
}

void setCursorPos(int x) {
  int xCopy = x;

  while (xCopy < 0) {
    xCopy = x + 3;
  }
  cursor_pos = xCopy % 3;
}

void printCursor(int cursorPos) {
  int x_pos = 4, y_pos, vert_pos = cursorPos % 3;
  if (vert_pos == 0) {
    y_pos = Y_LINE1;
  }
  if (vert_pos == 1) {
    y_pos = Y_LINE2;
  }
  if (vert_pos == 2) {
    y_pos = Y_LINE3;
  }

  displayText(">", x_pos, y_pos, TEXT_SIZE, false);
}

void printSettings() {

  //Line 1 - ON/OFF/NIGHT
  switch (winder_status) {
    case WINDER_OFF:
      displayText("OFF", 90, Y_LINE1, TEXT_SIZE, false);
      break;
    case WINDER_ON:
      displayText("ON", 90, Y_LINE1, TEXT_SIZE, false);
      break;
    case WINDER_NIGHT:
      displayText("NIGHT", 90, Y_LINE1, TEXT_SIZE, false);
      break;
  }


  //Line 2 - TPD
  String string_holder = String(turns_per_day);
  displayText(string_holder, 90, Y_LINE2, TEXT_SIZE, false);

  //Line 3 - BDIR/CLKW/CCLKW
  switch (turn_dir) {
    case BDIR:
      displayText("BDIR", 90, Y_LINE3, TEXT_SIZE, false);
      break;
    case CLKW:
      displayText("CLKW", 90, Y_LINE3, TEXT_SIZE, false);
      break;
    case ACLKW:
      displayText("CCLKW", 90, Y_LINE3, TEXT_SIZE, false);
      break;
  }
}

void updateRestTime() {
  restTimeMS = (MS_PER_DAY / turns_per_day) * NUM_ROT;
}

void toggleWinderStatus() {
  winder_status = (winder_status + 1) % 3;
}

void toggleWinderDir() {
  turn_dir = (turn_dir + 1) % 3;
}

void runCycle() {
  if (winder_status == WINDER_NIGHT) {
    //Convert time to more convenient test interval
    int stop_time = 0;
    int start_time = winder_night_start - winder_night_stop;
    if (start_time < 0) {
      start_time = start_time + 24;
    }
    int current_time = hour() - winder_night_stop;
    if (current_time < 0) {
      current_time = current_time + 24;
    }
    if (current_time >= stop_time && current_time <= start_time) {  // If night mode is enabled and the time is night, don't run winder
      winder_time = 0;
      return;
    }
    if (winder_time > (restTimeMS / 2)) { //halfed due to nighttime
      digitalWrite(STB_PIN, HIGH);
      if (turn_dir == BDIR) {
        if (lastDirection == CLKW) {
          stepper.step(-CYCLE_STEPS);   //Step motor CYCLE_STEPS anti-clockwise
          lastDirection = ACLKW;        //Update last turn direction
        } else {
          stepper.step(CYCLE_STEPS);    //Step motor CYCLE_STEPS clockwise
          lastDirection = CLKW;         //Update last turn direction
        }
      } else {  //if watch is uni-directional
        if (turn_dir == CLKW) {
          stepper.step(CYCLE_STEPS);    //Step motor CYCLE_STEPS clockwise
        } else {
          stepper.step(-CYCLE_STEPS);   //Step motor CYCLE_STEPS anti-clockwise
        }
      }
      winder_time = 0;                  //Reset the time since last wind
      digitalWrite(STB_PIN, LOW);
    }
  } else if (winder_status == WINDER_ON) {
    if (winder_time > restTimeMS) {
      digitalWrite(STB_PIN, HIGH);
      if (turn_dir == BDIR) {
        if (lastDirection == CLKW) {
          stepper.step(-CYCLE_STEPS);   //Step motor CYCLE_STEPS anti-clockwise
          lastDirection = ACLKW;        //Update last turn direction
        } else {
          stepper.step(CYCLE_STEPS);    //Step motor CYCLE_STEPS clockwise
          lastDirection = CLKW;         //Update last turn direction
        }
      } else {  //if watch is uni-directional
        if (turn_dir == CLKW) {
          stepper.step(CYCLE_STEPS);    //Step motor CYCLE_STEPS clockwise
        } else {
          stepper.step(-CYCLE_STEPS);   //Step motor CYCLE_STEPS anti-clockwise
        }
      }
      winder_time = 0;                  //Reset the time since last wind
      digitalWrite(STB_PIN, LOW);
    }
  }
}
