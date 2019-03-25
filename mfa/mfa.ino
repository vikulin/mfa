//***THIS IS THE NEW VERSION THAT WORKS WITH THE NEW LIBRARIES!!!***
// TFTLCD.h and TouchScreen.h are from adafruit.com where you can also purchase a really nice 2.8" TFT with touchscreen :)
// 2019 Vadym Vikulin - vadym.vikulin@gmail.com
#include <Adafruit_GFX.h>    // Core graphics library
#include <TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <EEPROM.h>
#include <DS3232RTC.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "struct.h"

// These are the pins for the shield!
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define PIN_DS18B20 40
#define PIN_PH_SENSOR A8

/*
//tft.setRotation(1);
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
*/

//tft.setRotation(3);
#define TS_MINX 920
#define TS_MINY 940
#define TS_MAXX 150
#define TS_MAXY 120
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
#define TME tmElements_t

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define PIN_BUZZER 38
/**
 * Light lines pins
 */
#define PIN_LIGHT_LINE_1 52
#define PIN_LIGHT_LINE_2 50
#define PIN_LIGHT_LINE_3 48
#define PIN_LIGHT_LINE_4 46
/**
 * Pump pin
 */
#define PIN_PUMP 44

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

DS3232RTC DS3231;

// Создаем объект OneWire
OneWire oneWire(PIN_DS18B20);
// Создаем объект DallasTemperature для работы с сенсорами, передавая ему ссылку на объект для работы с 1-Wire.
DallasTemperature dallasSensors(&oneWire);
DeviceAddress sensorAddress;

TME o_tme;
TME n_tme;

// Color definitions - in 5:6:5
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF
#define TEST            0x1BF5
#define JJCOLOR         0x1CB6
#define JJORNG          0xFD03
#define GRAY          0x8410

int i = 0;
int page = 0;
int sleep = 0;
int pulsev = 0;
int redflag = 0;
int greenflag = 0;

#define Y_DATE 213   // center point

/**
 * Light active lines
 */
boolean activeLightLine[4] = {false,false,false,false};
/**
 * Pump active
 */
boolean activePump[1] = {false};
/**
 * Test active flag
 */
boolean activeTest = false;

byte lightTimeHoursStart[4] = {0,0,0,0};
byte lightTimeMinutesStart[4] = {0,0,0,0};
byte lightTimeHoursEnd[4] = {0,0,0,0};
byte lightTimeMinutesEnd[4] = {0,0,0,0};

int tempC;
int temp;

int battfill;
unsigned long battcheck = 10000; // the amount of time between voltage check and battery icon refresh
unsigned long prevbatt;
int battv;
int battold;
int battpercent;
int barv;
int prevpage;
int sleepnever;
int backlightbox;
int antpos = 278;
unsigned long awakeend;
unsigned long currenttime;
unsigned long ssitime;
char voltage[10];
char battpercenttxt [10];
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
void setup(void) {
  pinMode(3, OUTPUT);
  
  pinMode(PIN_LIGHT_LINE_1, OUTPUT);
  pinMode(PIN_LIGHT_LINE_2, OUTPUT);
  pinMode(PIN_LIGHT_LINE_3, OUTPUT);
  pinMode(PIN_LIGHT_LINE_4, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_LIGHT_LINE_1, HIGH);
  digitalWrite(PIN_LIGHT_LINE_2, HIGH);
  digitalWrite(PIN_LIGHT_LINE_3, HIGH);
  digitalWrite(PIN_LIGHT_LINE_4, HIGH);
  digitalWrite(PIN_PUMP, HIGH);
  lightTimeHoursStart[0] = (byte)EEPROM.read(0);
  lightTimeHoursStart[1] = (byte)EEPROM.read(1);
  lightTimeHoursStart[2] = (byte)EEPROM.read(2);
  lightTimeHoursStart[3] = (byte)EEPROM.read(3);
  lightTimeMinutesStart[0] = (byte)EEPROM.read(4);
  lightTimeMinutesStart[1] = (byte)EEPROM.read(5);
  lightTimeMinutesStart[2] = (byte)EEPROM.read(6);
  lightTimeMinutesStart[3] = (byte)EEPROM.read(7);

  lightTimeHoursEnd[0] = (byte)EEPROM.read(8);
  lightTimeHoursEnd[1] = (byte)EEPROM.read(9);
  lightTimeHoursEnd[2] = (byte)EEPROM.read(10);
  lightTimeHoursEnd[3] = (byte)EEPROM.read(11);
  lightTimeMinutesEnd[0] = (byte)EEPROM.read(12);
  lightTimeMinutesEnd[1] = (byte)EEPROM.read(13);
  lightTimeMinutesEnd[2] = (byte)EEPROM.read(14);
  lightTimeMinutesEnd[3] = (byte)EEPROM.read(15);

  Serial.begin(9600);
  Serial.println("MFA");
  Serial.println("Vadym Vikulin  -  2019");
  
  tft.reset();
  
  uint16_t identifier = 0x9341;
  tft.begin(identifier); 
  tft.fillScreen(BLACK);
  tft.setRotation(3);
  tft.fillRect(71, 70, 50, 100, JJCOLOR);
  tft.fillRect(134, 70, 50, 100, JJCOLOR);
  tft.fillRect(197, 70, 50, 100, JJCOLOR);
  tft.drawRect(46, 45, 228, 150, WHITE);

  delay(250);
  tft.setCursor(85, 100);
  tft.setTextSize(5);
  tft.setTextColor(WHITE);
  tft.print("M");
  delay(250);
  tft.setCursor(147, 100);
  tft.print("F");
  delay(250);
  tft.setCursor(210, 100);
  tft.print("A");
  delay(500);
  tft.setCursor(84, 210);
  tft.setTextSize(1);
  tft.print("Vadym Vikulin  -  2019");
  tft.setCursor(108, 230);
  tft.print("vadym.vikulin@gmail.com");
  delay(500);
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 320, 10, JJCOLOR); // status bar
  drawhomeicon(); // draw the home icon
  
  tft.drawRect(297, 1, 20, 8, WHITE); //battery body
  tft.fillRect(317, 3, 2, 4, WHITE); // battery tip
  tft.fillRect(298, 2, 18, 6, BLACK); // clear the center of the battery
  drawbatt();
  ant(); // draw the bas "antenna" line without the "signal waves"
  signal(); // draw the "signal waves" around the "antenna"
  homescr(); // draw the homescreen
  tft.drawRect(0, 200, 270, 40, WHITE); // message box
  pinMode(13, OUTPUT);
  
  DS3231.squareWave(SQWAVE_1_HZ);
  DS3231.read(n_tme);
  dallasSensors.begin();
  if (!dallasSensors.getAddress(sensorAddress, 0)) Serial.println("Не можем найти первое устройство");
  dallasSensors.setResolution(sensorAddress, 12);
  //beep(100);
  printDate(n_tme.Day, n_tme.Month, n_tme.Year, WHITE);
  tft.print("    MFA 0.1 Alpha");
  TextSecond(n_tme.Second, WHITE);
  TextMinute(n_tme.Minute, WHITE);
  TextHour(n_tme.Hour, WHITE);

  tempC = printTemperature(sensorAddress, temp);      
  temp = tempC;
}

void printTime(){
        // date change
    DS3231.read(n_tme);
    if (o_tme.Day != n_tme.Day || o_tme.Month != n_tme.Month || o_tme.Year != n_tme.Year)
    {
      printDate(o_tme.Day, o_tme.Month, o_tme.Year, BLACK);
      printDate(n_tme.Day, n_tme.Month, n_tme.Year, WHITE);
      //o_tme.Day = n_tme.Day; do not need to copy as it is copied in next loop
    }
    
    if (o_tme.Second != n_tme.Second) // second change or time ticked
    {
      int diff = n_tme.Second - n_tme.Second/10*10;
      
      if(diff==0){
        TextSecond(o_tme.Second, BLACK);
        TextSecond(n_tme.Second, WHITE);

      } else {
        int diff_o = o_tme.Second - o_tme.Second/10*10;
        TextSecondLo(diff_o, BLACK);
        TextSecondLo(diff, WHITE);
      }
      if (o_tme.Minute != n_tme.Minute) // minute change or minute and hour change
      {
        TextMinute(o_tme.Minute, BLACK);
        TextMinute(n_tme.Minute, WHITE);
      }
      // draw hour, minute, second hands even no changes to overlap the "clear stuff"
      if (o_tme.Hour != n_tme.Hour) // minute change or minute and hour change
      {
        TextHour(o_tme.Hour, BLACK);
        TextHour(n_tme.Hour, WHITE);
      }
      copyTME(o_tme, n_tme);
    }
}

void loop() {
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  
  printTime();

  dallasSensors.requestTemperatures();

  tempC = printTemperature(sensorAddress, temp);      
  temp = tempC;

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    
     Serial.print("X = "); 
     Serial.print(p.x);
     Serial.print("\tY = "); 
     Serial.print(p.y);
     Serial.print("\tPressure = "); 
     Serial.println(p.z);
    
    // turn from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    
    Serial.print("p.y:"); // this code will help you get the y and x numbers for the touchscreen
    Serial.print(p.y);
    Serial.print("   p.x:");
    Serial.println(p.x);


    
    // area 1
    if (p.y > 0 && p.y < 146 && p.x > 178 && p.x < 226) { // if this area is pressed
      if (page == 5) { // and if page 5 is drawn on the screen
        m5b1action(); // do whatever this button is
      }
      if (page == 4) {
        m4b1action();
      }
      if (page == 3) {
        m3b1action();
      }
      if (page == 2) {
        m2b1action();
      }
      if (page == 1) {
        showLine1Button();
      }
      if (page == 0) { // if you are on the "home" page (0)
        page = 1; // then you just went to the first page
        redraw(); // redraw the screen with the page value 1, giving you the page 1 menu
      }
    }
    // area 2
    if (p.y > 168 && p.y < 320 && p.x > 180 && p.x < 226) {
      if (page == 5) {
        m5b2action();
      }
      if (page == 4) {
        m4b2action();
      }
      if (page == 3) {
        m3b2action();
      }
      if (page == 2) {
        m2b2action();
      }
      if (page == 1) {
        showLine2Button();
      }
      if (page == 0) {
        page = 2;
        redraw();
      }
    }
    // area 3
    if (p.y > 0 && p.y < 146 && p.x > 120 && p.x < 168) {
      if (page == 5) {
        m5b3action();
      }
      if (page == 4) {
        m4b3action();
      }
      if (page == 3) {
        m3b3action();
      }
      if (page == 2) {
        m2b3action();
      }
      if (page == 1) {
        showLine3Button();
      }
      if (page == 0) {
        page = 3;
        redraw();
      }
    }
    // area 4
    if (p.y > 167 && p.y < 320 && p.x > 120 && p.x < 168) {
      if (page == 5) {
        m5b4action();
      }
      if (page == 4) {
        m4b4action();
      }
      if (page == 3) {
        m3b4action();
      }
      if (page == 2) {
        m2b4action();
      }
      if (page == 1) {
        showLine4Button();
      }
      if (page == 0) {
        page = 4;
        redraw();
      }
    }
    // area 5
    if (p.y > 0 && p.y < 146 && p.x > 54 && p.x < 104) {
      if (page == 5) {
        m5b5action();
      }
      if (page == 4) {
        float pH = m4b5action();
        tft.setCursor(12, 213);
        tft.print(String(pH));
        delay(1000);
        clearmessage();
      }
      if (page == 3) {
        m3b5action();
      }
      if (page == 2) {
        m2b5action();
      }
      if (page == 1) {
        showLineTestButton();
      }
      if (page == 0) {
        page = 5;
        redraw();
      }
    }
    // area 6
    if (p.y > 168 && p.y < 320 && p.x > 54 && p.x < 104) {
      if (page == 5) {
        m5b6action();
      }
      if (page == 4) {
        m4b6action();
      }
      if (page == 3) {
        m3b6action();
      }
      if (page == 2) {
        m2b6action();
      }
      if (page == 1) {
        showLineSchedule();
      }
      if (page == 0) {
        page = 6;
        redraw();
      }
    }
    // home
    if (p.y > 280 && p.y < 340 && p.x > 0 && p.x < 48) { // if the home icon is pressed
      if (page == 8) { // if you are leaving the Schedule page
        printMessage("Settings saved", YELLOW); // display settings saved in message box
        EEPROM.write(0, (byte)lightTimeHoursStart[0]);
        EEPROM.write(1, (byte)lightTimeHoursStart[1]);
        EEPROM.write(2, (byte)lightTimeHoursStart[2]);
        EEPROM.write(3, (byte)lightTimeHoursStart[3]);
        EEPROM.write(4, (byte)lightTimeMinutesStart[0]);
        EEPROM.write(5, (byte)lightTimeMinutesStart[1]);
        EEPROM.write(6, (byte)lightTimeMinutesStart[2]);
        EEPROM.write(7, (byte)lightTimeMinutesStart[3]);

        EEPROM.write(8, (byte)lightTimeHoursEnd[0]);
        EEPROM.write(9, (byte)lightTimeHoursEnd[1]);
        EEPROM.write(10, (byte)lightTimeHoursEnd[2]);
        EEPROM.write(11, (byte)lightTimeHoursEnd[3]);
        EEPROM.write(12, (byte)lightTimeMinutesEnd[0]);
        EEPROM.write(13, (byte)lightTimeMinutesEnd[1]);
        EEPROM.write(14, (byte)lightTimeMinutesEnd[2]);
        EEPROM.write(15, (byte)lightTimeMinutesEnd[3]);
        
        clearSchedule(); // erase all the drawings on the settings page
        resetTest();
      }
      if (page == 0) { // if you are already on the home page
        drawhomeiconred(); // draw the home icon red
        delay(250); // wait a bit
        drawhomeicon(); // draw the home icon back to white
        return; // if you were on the home page, stop.
      }
      else { // if you are not on the settings, home, or keyboard page
        page = prevpage; // a value to keep track of what WAS on the screen to redraw/erase only what needs to be
        page = 0; // make the current page home
        redraw(); // redraw the page
      }
    }
    // message area
    if (p.y > 0 && p.y < 246 && p.x > 4 && p.x < 44) {
      clearmessage(); // erase the message
    }
    // LightTime buttons
    if (p.y > 0 && p.y < 60 && p.x > 180 && p.x < 210) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            declightTimeHoursStart(i);
            break;
          }
        }
      }
    }
    if (p.y > 60 && p.y < 120 && p.x > 180 && p.x < 210) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            inclightTimeHoursStart(i);
            break;
          }
        }
      }
    }
    if (p.y > 200 && p.y < 260 && p.x > 180 && p.x < 210) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            declightTimeMinutesStart(i);
            break;
          }
        }
      }
    }
    if (p.y > 260 && p.y < 320 && p.x > 180 && p.x < 210) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            inclightTimeMinutesStart(i);
            break;
          }
        }
      }
    }
    // LightTime buttons
    if (p.y > 0 && p.y < 60 && p.x > 150 && p.x < 180) {
      if (page == 8) {
         for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            declightTimeHoursEnd(i);
            break;
          }
        }
      }
    }
    if (p.y > 60 && p.y < 120 && p.x > 150 && p.x < 180) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            inclightTimeHoursEnd(i);
            break;
          }
        }
      }
    }
  
   if (p.y > 200 && p.y < 260 && p.x > 150 && p.x < 180) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            declightTimeMinutesEnd(i);
            break;
          }
        }
      }
    }

   if (p.y > 260 && p.y < 320 && p.x > 150 && p.x < 180) {
      if (page == 8) {
        for(int i=0;i<=3;i++){
          if(activeLightLine[i]){
            inclightTimeMinutesEnd(i);
            break;
          }
        }
      }
    }

    /*
    // optional buttons
     if (p.y > 3 && p.y < 66 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3down();
     }
     }
     if (p.y > 269 && p.y < 324 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3up();
     }
     }
     */
  }
  if(currenttime - prevbatt > battcheck) {
    drawbatt();
    prevbatt = currenttime;

  }
}

void beep(int beep_time) {
  myTone(PIN_BUZZER, 2048, beep_time);
}

// Вспомогательная функция печати значения температуры для устрйоства
int printTemperature(DeviceAddress deviceAddress, int oldTemp){
  int tempC = (int)dallasSensors.getTempC(deviceAddress);
  if(tempC!=oldTemp){
    printMessageLeft(String(tempC)+"C", BLUE);
    tft.drawCircle(35, 210, 2, BLUE);
  }
  return tempC;
}

// Print Date in format YYYY-MM-DD
void printDate(int d, int m, int y, int color)
{
  String date = String(y+1970)+"-"+conv_num2char(m)+"-"+conv_num2char(d);
  tft.fillRect(0, 0, 100, 10, JJCOLOR);
  tft.setCursor(1, 1);
  tft.setTextSize(1);
  tft.print(date);
}

void TextHour(int number, int color)
{
  tft.setTextSize(2);
  tft.setCursor(183, Y_DATE);
  tft.setTextColor(color);
  tft.print(conv_num2char(number));
}

void TextMinute(int number, int color)
{
  tft.setTextSize(2);
  tft.setCursor(212, Y_DATE);
  tft.setTextColor(color);
  tft.print(conv_num2char(number));
}

void TextSecond(int number, int color)
{
  tft.setTextSize(2);
  tft.setCursor(242, Y_DATE);
  tft.setTextColor(color);
  tft.print(conv_num2char(number));
}

void TextSecondLo(int number, int color)
{ 
  tft.setTextSize(2);
  tft.setCursor(254, Y_DATE);
  tft.setTextColor(color);
  tft.print(number);
}

void redraw() { // redraw the page
  if ((prevpage != 6) || (page !=7)) {
    clearcenter();
  }
  if (page == 0) {
    homescr();
  }
  if (page == 1) {
    menu1();
  }
  if (page == 2) {
    menu2();
  }
  if (page == 3) {
    menu3();
  }
  if (page == 4) {
    menu4();
  }
  if (page == 5) {
    menu5();
  }
  if (page == 6) {
    //settingsscr();
    
  }
}
void clearcenter() { // the reason for so many small "boxes" is that it's faster than filling the whole thing
  tft.drawRect(0, 20, 150, 50, BLACK);
  tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  tft.drawRect(170, 80, 150, 50, BLACK);
  tft.drawRect(0, 140, 150, 50, BLACK);
  tft.drawRect(170, 140, 150, 50, BLACK);
  tft.fillRect(22, 37, 106, 16, BLACK);
  tft.fillRect(192, 37, 106, 16, BLACK);
  tft.fillRect(22, 97, 106, 16, BLACK);
  tft.fillRect(192, 97, 106, 16, BLACK);
  tft.fillRect(22, 157, 106, 16, BLACK);
  tft.fillRect(192, 157, 106, 16, BLACK);
}
void clearSchedule() { // this is used to erase the extra drawings when exiting the settings page
  tft.fillRect(0, 20, 320, 180, BLACK);
  delay(500);
  clearmessage();
}
void homescr() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  buttons();
}
void menu1() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  
  int w = 150;
  int h = 50;
  int color = BLACK;
  if(activeLightLine[0]){
    color = JJORNG;
  }
  button("Line 1", 0, 20, w, h, JJCOLOR, 22, 17, color);
  if(activeLightLine[1]){
    color = JJORNG;
  } else {
    color = BLACK;
  }
  button("Line 2", 170, 20, w, h, JJCOLOR, 22, 17, color);
  if(activeLightLine[2]){
    color = JJORNG;
  } else {
    color = BLACK;
  }
  button("Line 3", 0, 80, w, h, JJCOLOR, 22, 17, color);
  if(activeLightLine[3]){
    color = JJORNG;
  } else {
    color = BLACK;
  }
  button("Line 4", 170, 80, w, h, JJCOLOR, 22, 17, color);
  button("Test", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  button("Schedule", 170, 140, w, h, JJCOLOR, 22, 17, BLACK);
}
void menu2() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  int w = 150;
  int h = 50;
  button("Main pump", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("--", 170, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("--", 0, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("--", 170, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Test", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  button("Schedule", 170, 140, w, h, JJCOLOR, 22, 17, BLACK);
}
void menu3() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  int w = 150;
  int h = 50;
  button("Menu 3 B1", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 3 B2", 170, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 3 B3", 0, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 3 B4", 170, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 3 B5", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 3 B6", 170, 140, w, h, JJCOLOR, 22, 17, BLACK);
}
void menu4() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  int w = 150;
  int h = 50;
  button("Menu 4 B1", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 4 B2", 170, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 4 B3", 0, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 4 B4", 170, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Test", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  button("Schedule", 170, 140, w, h, JJCOLOR, 22, 17, BLACK);
}
void menu5() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  int w = 150;
  int h = 50;
  button("Menu 5 B1", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 5 B2", 170, 20, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 5 B3", 0, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 5 B4", 170, 80, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 5 B5", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  button("Menu 5 B6", 170, 140, w, h, JJCOLOR, 22, 17, BLACK);
}
void schedule(int lineIndex) {

  tft.setTextColor(WHITE);
  
  int w = 60;
  int h = 30;
  /*
  tft.setTextSize(2);
  if(lightTimeHoursStart[lineIndex]==0 && startlightTimeMinutesStart[lineIndex]==0){
    button("Off", 0, 140, 320, h, JJCOLOR, 140, 17, RED);
  } else {
    button("On", 0, 140, 320, h, JJCOLOR, 140, 17, GREEN);
  }*/
  tft.setTextSize(3);
  button("-", 0, 20, w, h, WHITE, 22, 5, RED);
  button("+", 60, 20, w, h, WHITE, 22, 5, GREEN); 
  button("-", 200, 20, w, h, WHITE, 22, 5, RED);
  button("+", 260, 20, w, h, WHITE, 22, 5, GREEN);
  
  button("-", 0, 55, w, h, WHITE, 22, 5, RED);
  button("+", 60, 55, w, h, WHITE, 22, 5, GREEN);
  button("-", 200, 55, w, h, WHITE, 22, 5, RED);
  button("+", 260, 55, w, h, WHITE, 22, 5, GREEN);
  /*
  button("-", 0, 90, w, h, WHITE, 22, 5, RED); 
  button("+", 260, 90, w, h, WHITE, 22, 5, GREEN);
  button("-", 0, 125, w, h, WHITE, 22, 5, RED);
  button("+", 260, 125, w, h, WHITE, 22, 5, GREEN);
  */
  tft.drawRect(120, 20, 80, 30, JJCOLOR);
  tft.drawRect(120, 55, 80, 30, JJCOLOR);

  showlightTimeHoursStart(lightTimeHoursStart[lineIndex]);
  showlightTimeMinutesStart(lightTimeMinutesStart[lineIndex]);
  showlightTimeHoursEnd(lightTimeHoursEnd[lineIndex]);
  showlightTimeMinutesEnd(lightTimeMinutesEnd[lineIndex]);
  
  //battv = readVcc(); // read the voltage
  //itoa (battv, voltage, 10);
  //printMessage(String(voltage)+"mV", YELLOW); // display settings saved in message box
  /*
  battpercent = (battv / 5000) * 100, 2;
  itoa (battpercent, battpercenttxt, 10);
  tft.print(102, 213, battpercenttxt, YELLOW, 2);
  */
}

void showlightTimeHoursStart(int lightTimeH) {
  tft.fillRect(123, 25, 30, 20, GRAY);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  if(lightTimeH>9){
    tft.setCursor(128, 30);
  } else {
    tft.setCursor(140, 30);
  }
  tft.print(lightTimeH);
  tft.print(":");
}
void showlightTimeMinutesStart(int lightTimeM) {
  tft.fillRect(168, 25, 30, 20, GRAY);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(170, 30);
  tft.print(conv_num2char(lightTimeM));
}

void showlightTimeHoursEnd(int lightTimeH) {
  tft.fillRect(123, 60, 30, 20, GRAY);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  if(lightTimeH>9){
    tft.setCursor(128, 65);
  } else {
    tft.setCursor(140, 65);
  }
  tft.print(lightTimeH);
  tft.print(":");
}
void showlightTimeMinutesEnd(int lightTimeM) {
  tft.fillRect(168, 60, 30, 20, GRAY);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(170, 65);
  tft.print(conv_num2char(lightTimeM));
}

void inclightTimeMinutesStart(int lineIndex) { // sleep increese adjustment
  int h = 50;
  if (lightTimeMinutesStart[lineIndex] < 59) {
    lightTimeMinutesStart[lineIndex] = lightTimeMinutesStart[lineIndex]+1;
  } else {
    lightTimeMinutesStart[lineIndex]=0;
  }
  showlightTimeMinutesStart(lightTimeMinutesStart[lineIndex]);
  if(lightTimeHoursStart[lineIndex]==0 && lightTimeMinutesStart[lineIndex]==0){
    showlightTimeHoursStart(0);
  }
  delay(350);
}
void declightTimeMinutesStart(int lineIndex) { // sleep decreese adjustment
  int h = 50;
  if (lightTimeMinutesStart[lineIndex] > 0) {
    lightTimeMinutesStart[lineIndex] = lightTimeMinutesStart[lineIndex]-1;
  } else {
    lightTimeMinutesStart[lineIndex]=59;
  }
  showlightTimeMinutesStart(lightTimeMinutesStart[lineIndex]);
  if(lightTimeHoursStart[lineIndex]==0 && lightTimeMinutesStart[lineIndex]==0){
    showlightTimeHoursStart(0);
  }
  delay(350);
}
void inclightTimeHoursStart(int lineIndex) { // sleep increese adjustment
  int h = 50;
  if (lightTimeHoursStart[lineIndex] < 23) {
    lightTimeHoursStart[lineIndex] = lightTimeHoursStart[lineIndex]+1;
  } else {
    lightTimeHoursStart[lineIndex]=0;
  }
  showlightTimeHoursStart(lightTimeHoursStart[lineIndex]);
  if(lightTimeHoursStart[lineIndex]==0 && lightTimeMinutesStart[lineIndex]==0){
    showlightTimeMinutesStart(0);
  }
  delay(350);
}
void declightTimeHoursStart(int lineIndex) { // sleep decreese adjustment
  int h = 50;
  if (lightTimeHoursStart[lineIndex] > 0) {
    lightTimeHoursStart[lineIndex] = lightTimeHoursStart[lineIndex]-1;
  } else {
    lightTimeHoursStart[lineIndex]=23;
  }
  showlightTimeHoursStart(lightTimeHoursStart[lineIndex]);  
  if(lightTimeHoursStart[lineIndex]==0 && lightTimeMinutesStart[lineIndex]==0){
    showlightTimeMinutesStart(0);
  }
  delay(350);
}




void inclightTimeMinutesEnd(int lineIndex) { // sleep increese adjustment
  int h = 50;
  if (lightTimeMinutesEnd[lineIndex] < 59) {
    lightTimeMinutesEnd[lineIndex] = lightTimeMinutesEnd[lineIndex]+1;
  } else {
    lightTimeMinutesEnd[lineIndex]=0;
  }
  showlightTimeMinutesEnd(lightTimeMinutesEnd[lineIndex]);
  if(lightTimeHoursEnd[lineIndex]==0 && lightTimeMinutesEnd[lineIndex]==0){
    showlightTimeHoursEnd(0);
  }
  delay(350);
}
void declightTimeMinutesEnd(int lineIndex) { // sleep decreese adjustment
  int h = 50;
  if (lightTimeMinutesEnd[lineIndex] > 0) {
    lightTimeMinutesEnd[lineIndex] = lightTimeMinutesEnd[lineIndex]-1;
  } else {
    lightTimeMinutesEnd[lineIndex]=59;
  }
  showlightTimeMinutesEnd(lightTimeMinutesEnd[lineIndex]);
  if(lightTimeHoursEnd[lineIndex]==0 && lightTimeMinutesEnd[lineIndex]==0){
    showlightTimeHoursEnd(0);
  }
  delay(350);
}
void inclightTimeHoursEnd(int lineIndex) { // sleep increese adjustment
  int h = 50;
  if (lightTimeHoursEnd[lineIndex] < 23) {
    lightTimeHoursEnd[lineIndex] = lightTimeHoursEnd[lineIndex]+1;
  } else {
    lightTimeHoursEnd[lineIndex]=0;
  }
  showlightTimeHoursEnd(lightTimeHoursEnd[lineIndex]);
  if(lightTimeHoursEnd[lineIndex]==0 && lightTimeMinutesEnd[lineIndex]==0){
    showlightTimeMinutesEnd(0);
  }
  delay(350);
}
void declightTimeHoursEnd(int lineIndex) { // sleep decreese adjustment
  int h = 50;
  if (lightTimeHoursEnd[lineIndex] > 0) {
    lightTimeHoursEnd[lineIndex] = lightTimeHoursEnd[lineIndex]-1;
  } else {
    lightTimeHoursEnd[lineIndex]=23;
  }
  showlightTimeHoursEnd(lightTimeHoursEnd[lineIndex]);  
  if(lightTimeHoursEnd[lineIndex]==0 && lightTimeMinutesEnd[lineIndex]==0){
    showlightTimeMinutesEnd(0);
  }
  delay(350);
}




void option3down() { // adjust option 3 down in the settings screen
}
void option3up() { // adjust option 3 up in the settings screen
}
void activateLightLine(int index){
  
}
//custom defined actions - this is where you put your button functions
void showLine1Button() {
  int w = 150;
  int h = 50;
  activeLightLine[0]=!activeLightLine[0];
  if(activeLightLine[0]){
    button("Line 1", 0, 20, w, h, JJCOLOR, 22, 17, JJORNG);
  } else {
    button("Line 1", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  }
  //signal();
}
void showLine2Button() {
  int w = 150;
  int h = 50;
  activeLightLine[1]=!activeLightLine[1];
  if(activeLightLine[1]){
    button("Line 2", 170, 20, w, h, JJCOLOR, 22, 17, JJORNG);
  } else {
    button("Line 2", 170, 20, w, h, JJCOLOR, 22, 17, BLACK);
  }
  //signalact();
}
void showLine3Button() {
  int w = 150;
  int h = 50;
  activeLightLine[2]=!activeLightLine[2];
  if(activeLightLine[2]){
    button("Line 3", 0, 80, w, h, JJCOLOR, 22, 17, JJORNG);
  } else {
    button("Line 3", 0, 80, w, h, JJCOLOR, 22, 17, BLACK);
  }
}
void showLine4Button() {
  int w = 150;
  int h = 50;
  activeLightLine[3]=!activeLightLine[3];
  if(activeLightLine[3]){
    button("Line 4", 170, 80, w, h, JJCOLOR, 22, 17, JJORNG);
  } else {
    button("Line 4", 170, 80, w, h, JJCOLOR, 22, 17, BLACK);
  }
}
void showLineTestButton() {
  //Light test
  int w = 150;
  int h = 50;
  activeTest = !activeTest;
  if(activeTest && (activeLightLine[0] || activeLightLine[1] || activeLightLine[2] || activeLightLine[3])){
      if(activeLightLine[0]){
        digitalWrite(PIN_LIGHT_LINE_1, LOW);
      }
      if(activeLightLine[1]){
        digitalWrite(PIN_LIGHT_LINE_2, LOW);
      }
      if(activeLightLine[2]){
        digitalWrite(PIN_LIGHT_LINE_3, LOW);
      }
      if(activeLightLine[3]){
        digitalWrite(PIN_LIGHT_LINE_4, LOW);
      }
      button("Test", 0, 140, w, h, JJCOLOR, 22, 17, GREEN);
  } else {
     digitalWrite(PIN_LIGHT_LINE_1, HIGH);
     digitalWrite(PIN_LIGHT_LINE_2, HIGH);
     digitalWrite(PIN_LIGHT_LINE_3, HIGH);
     digitalWrite(PIN_LIGHT_LINE_4, HIGH);
     button("Test", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  }
  
}
void showLineSchedule() {
  //Light schedule
  for(int i=0;i<=3;i++){
    if(activeLightLine[i]){
      page=8;
      clearSchedule();
      schedule(i);
      break;
    }
  }
}
void m2b1action() {
  int w = 150;
  int h = 50;
  activePump[0]=!activePump[0];
  if(activePump[0]){
    button("Main pump", 0, 20, w, h, JJCOLOR, 22, 17, JJORNG);
  } else {
    button("Main pump", 0, 20, w, h, JJCOLOR, 22, 17, BLACK);
  }
}
void m2b2action() {
}
void m2b3action() {
}
void m2b4action() {
}
void m2b5action() {
  //Pump test
  int w = 150;
  int h = 50;
  activeTest = !activeTest;
  if(activeTest && activePump[0]){
      if(activePump[0]){
        digitalWrite(PIN_PUMP, LOW);
      }
      button("Test", 0, 140, w, h, JJCOLOR, 22, 17, GREEN);
  } else {
     digitalWrite(PIN_PUMP, HIGH);
     button("Test", 0, 140, w, h, JJCOLOR, 22, 17, BLACK);
  }
}
void m2b6action() {
  //pump schedule
}
void m3b1action() {
}
void m3b2action() {
}
void m3b3action() {
}
void m3b4action() {
}
void m3b5action() {
}
void m3b6action() {
}
void m4b1action() {
}
void m4b2action() {
}
void m4b3action() {
}
void m4b4action() {
}
float m4b5action() {
  //pH test
  float pH = readPh();
  return pH;
}
void m4b6action() {
}
void m5b1action() {
}
void m5b2action() {
}
void m5b3action() {
}
void m5b4action() {
}
void m5b5action() {
}
void m5b6action() {
}

void ant() {
  tft.fillRect((antpos + 5), 4, 1, 6, WHITE); // draws the "antenna" for the signal indicator
}
void button(String label, int x, int y, int w, int h, int color, int paddingX, int paddingY, int background){
  tft.fillRect(x, y, w, h, background);
  tft.drawRect(x, y, w, h, color);
  tft.setCursor(x+paddingX, y+paddingY);
  tft.print(label);
}
void buttons() { // redraw the button outline boxes
  int w = 150;
  int h = 50;
  button("Light", 0, 20, w, h, JJCOLOR, 41, 17, BLACK);
  button("Pump", 170, 20, w, h, JJCOLOR, 40, 17, BLACK);
  button("Heater", 0, 80, w, h, JJCOLOR, 41, 17, BLACK);
  button("pH", 170, 80, w, h, JJCOLOR, 40, 17, BLACK);
  button("Clock", 0, 140, w, h, JJCOLOR, 41, 17, BLACK);
  button("Settings", 170, 140, w, h, JJCOLOR, 30, 17, BLACK);
}
void signal() { // draws a whit 'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, WHITE);
  tft.drawPixel((antpos + 3), 2, WHITE);
  tft.drawPixel((antpos + 3), 7, WHITE);
  tft.drawPixel((antpos + 2), 0, WHITE);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, WHITE);
  tft.drawPixel((antpos + 2), 9, WHITE);
  tft.drawPixel((antpos + 1), 1, WHITE);
  tft.drawPixel((antpos + 1), 8, WHITE);
  tft.drawLine(antpos, 2, antpos, 7, WHITE);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, WHITE);
  tft.drawPixel((antpos + 7), 2, WHITE);
  tft.drawPixel((antpos + 7), 7, WHITE);
  tft.drawPixel((antpos + 8), 0, WHITE);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, WHITE);
  tft.drawPixel((antpos + 8), 9, WHITE);
  tft.drawPixel((antpos + 9), 1, WHITE);
  tft.drawPixel((antpos + 9), 8, WHITE);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, WHITE);
}
void signalact() { // draws a red'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, RED);
  tft.drawPixel((antpos + 3), 2, RED);
  tft.drawPixel((antpos + 3), 7, RED);
  tft.drawPixel((antpos + 2), 0, RED);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, RED);
  tft.drawPixel((antpos + 2), 9, RED);
  tft.drawPixel((antpos + 1), 1, RED);
  tft.drawPixel((antpos + 1), 8, RED);
  tft.drawLine(antpos, 2, antpos, 7, RED);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, RED);
  tft.drawPixel((antpos + 7), 2, RED);
  tft.drawPixel((antpos + 7), 7, RED);
  tft.drawPixel((antpos + 8), 0, RED);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, RED);
  tft.drawPixel((antpos + 8), 9, RED);
  tft.drawPixel((antpos + 9), 1, RED);
  tft.drawPixel((antpos + 9), 8, RED);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, RED);
}
void drawhomeicon() { // draws a white home icon
  tft.drawLine(280, 219, 299, 200, WHITE);
  tft.drawLine(300, 200, 304, 204, WHITE);
  tft.drawLine(304, 203, 304, 200, WHITE);
  tft.drawLine(305, 200, 307, 200, WHITE);
  tft.drawLine(308, 200, 308, 208, WHITE);
  tft.drawLine(309, 209, 319, 219, WHITE);
  tft.drawLine(281, 219, 283, 219, WHITE);
  tft.drawLine(316, 219, 318, 219, WHITE);
  tft.drawRect(284, 219, 32, 21, WHITE);
  tft.drawRect(295, 225, 10, 15, WHITE);
}
void drawhomeiconred() { // draws a red home icon
  tft.drawLine(280, 219, 299, 200, RED);
  tft.drawLine(300, 200, 304, 204, RED);
  tft.drawLine(304, 203, 304, 200, RED);
  tft.drawLine(305, 200, 307, 200, RED);
  tft.drawLine(308, 200, 308, 208, RED);
  tft.drawLine(309, 209, 319, 219, RED);
  tft.drawLine(281, 219, 283, 219, RED);
  tft.drawLine(316, 219, 318, 219, RED);
  tft.drawRect(284, 219, 32, 21, RED);
  tft.drawRect(295, 225, 10, 15, RED);
}
void clearmessage() {
  tft.fillRect(12, 213, 226, 16, BLACK); // black out the inside of the message box
}
void clearmessageLeft(){
  tft.fillRect(12, 213, 40, 16, BLACK); // black out the inside of the message box
}
void clearmessageRight(){
  tft.fillRect(50, 213, 140, 16, BLACK); // black out the inside of the message box
}
void printMessage(String message, int color){
  clearmessage();
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(12, 213);
  tft.print(message);
}

void printMessageLeft(String message, int color){
  clearmessageLeft();
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(12, 213);
  tft.print(message);
}
void printMessageRight(String message, int color){
  clearmessageRight();
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(60, 213);
  tft.print(message);
}
/**
 * Reset test values
 */
void resetTest(){
  activeTest = false;
}
void drawbatt() {
  battv = readVcc(); // read the voltage
  if (battv < battold) { // if the voltage goes down, erase the inside of the battery
    tft.fillRect(298, 2, 18, 6, BLACK);
  }
  battfill = map(battv, 3000, 4850, 2, 18); // map the battery voltage 3000 nis the low, 4150 is the high
  if (battfill > 7) { // if the battfill value is between 8 and 18, fill with green
    tft.fillRect(298, 2, battfill, 6, GREEN);
  }
  else { // if the battfill value is below 8, fill with red
    tft.fillRect(298, 2, battfill, 6, RED);
  }
  battold = battv; // this helps determine if redrawing the battfill area is necessary
}

void myTone(uint8_t i, uint32_t j, uint32_t k){  // Определяем функцию myTone
  j=500000/j;                                    // Меняем значение переменной j на время одного полупериода в мкс
  k+=millis();                                   // Меняем значение переменной к на время завершения вывода сигнала
  pinMode(i,  OUTPUT  );               // Переводим вывод PIN_BUZZER в режим выхода
  while(k>millis()){                             // Выводим сигнал, пока не истечёт указанное время
    digitalWrite(i, HIGH); delayMicroseconds(j); // Устанавливаем на выходе i уровень логической  «1» на время j
    digitalWrite(i, LOW ); delayMicroseconds(j); // Устанавливаем на выходе i уровень логического «0» на время j
  }
}

float readPh(){
    float calibration = 24.1;
    int measuringVal = analogRead(PIN_PH_SENSOR);
    Serial.print("Measuring Raw Value > ");
    Serial.print(measuringVal);
 
    double vltValue = measuringVal*(5.0/1024.0);
    Serial.print("Voltage Value > ");
    Serial.print(vltValue, 3);
 
    float phValue = -5.70 * vltValue + calibration;
    Serial.print("sensor = ");
    Serial.println(phValue);
    return phValue;
}

// convert minute from INT to CHAR
String conv_num2char(int min)
{
  if(min>9){
    return String(min);
  } else {
    return "0"+String(min);
  }
}

