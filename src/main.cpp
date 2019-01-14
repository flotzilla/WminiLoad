#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "StringSplitter.h"
#include <pt.h>

const int changeModeButtonPin = D5;
const int SCREEN_MODE_INIT = -1;    // SHOW LOGO
const int SCREEN_MODE_DEFAULT = 0;    // SHOW ALL
const int SCREEN_MODE_CPU = 1;        // SHOW cpu only
const int SCREEN_MODE_MEM = 2;        // show mem only
const int SCREEN_MODE_VA_DISPLAY = 3; // show video adapter name
const int SCREEN_MODE_VA = 4;         // show  video adapter stats
const int READING_TIMEOUT = 100;

// Duration of logo showing
// time = READING_TIMEOUT * VA_DISPLAY_TIMES_SHOW
const int VA_DISPLAY_TIMES_SHOW = 2;

static struct pt serialReedHandlePt, buttonHanldePt, lcdHanldePt;

bool isPrevReadingReceiveData = false;

int cpuTempArraySize = 0,
    cpuCount = 0,
    cpuReal = 0,
    vaCount = 0,

    buttonState,
    lastButtonState = LOW,

    currentScreen = SCREEN_MODE_INIT,
    currentVaScreen = 0,
    currentVaDispolayScreenShowed = 0;

unsigned long lastDebounceTime = 0,
              debounceDelay = 50;

String cpuTempArray[20],
    currCPULoadPercent = "",
    currMemTotal = "",
    currMemUsed = "",
    currMemFree = "",
    currMemLoadPercent = "",

    currTime = "",
    currDay = "";

struct VideoCard
{
  int id;
  String name;
  String engClock;
  String memClock;
  String currTemp;
  String usage;
  String fanPercent;
  String fanRpm;
};

VideoCard videocards[10];

LiquidCrystal_I2C lcd(0x3F, 20, 4);

void showStartupMessage()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Waiting for connect");
}

void setup()
{
  pinMode(changeModeButtonPin, INPUT);

  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  PT_INIT(&serialReedHandlePt);
  PT_INIT(&buttonHanldePt);
  PT_INIT(&lcdHanldePt);

  showStartupMessage();
}

String getReading(String from, String tagStart)
{
  int startIndx = from.indexOf(tagStart);
  int endIndx = from.indexOf(tagStart + "_end");
  return from.substring(startIndx + tagStart.length(), endIndx);
}

void parseReadings(String s)
{
  if (s.startsWith("cpu_stat")) {
    // cpu stats parsing
    int cpuEndIndx = s.indexOf("cpu_stat_end");
    String cpuInfo = s.substring(strlen("cpu_stat"), cpuEndIndx);

    int cpuTempArraySize = 0;
    int occurencesIndexes[20];
    for (int x = 0; x < cpuInfo.length(); x++) {
      if (cpuInfo[x] == ',') {
        occurencesIndexes[cpuTempArraySize] = x;
        cpuTempArraySize++;
      }
    }

    if (cpuTempArraySize > 0) {
      cpuTempArray[0] = cpuInfo.substring(0, occurencesIndexes[0]);

      for (int i = 0, j = 1; i < cpuTempArraySize - 1; i++, j++) {
        cpuTempArray[j] = cpuInfo.substring(occurencesIndexes[i] + 1, occurencesIndexes[i + 1]);
      }
    }

    currCPULoadPercent = cpuInfo.substring(cpuInfo.lastIndexOf(",") + 1);

    // memory parsing
    String memInfo = getReading(s, "mem_stat");
    StringSplitter *splitter = new StringSplitter(memInfo, ',', 4);

    if (splitter->getItemCount() == 4) {
      currMemTotal = splitter->getItemAtIndex(0);
      currMemUsed = splitter->getItemAtIndex(1);
      currMemFree = splitter->getItemAtIndex(2);
      currMemLoadPercent = splitter->getItemAtIndex(3);
    }

    // cpu stats parsing
    cpuCount = getReading(s, "cpu_count").toInt();
    cpuReal = getReading(s, "cpu_real").toInt();
    vaCount = getReading(s, "va_count").toInt();
    currTime = getReading(s, "current_time");
    currDay = getReading(s, "curr_day");

    // parsing gpu settings
    currTime = getReading(s, "current_time");
    if (vaCount > 0) {
      for (int i = 0; i < vaCount; i++) {
        VideoCard card;
        String vaParams = getReading(s, "va" + i);

        card.id = i;
        card.name = getReading(vaParams, "dev_name");
        card.engClock = getReading(vaParams, "eng_clock");
        card.memClock = getReading(vaParams, "mem_clock");
        card.currTemp = getReading(vaParams, "currtemp");
        card.usage = getReading(vaParams, "usage");
        card.fanPercent = getReading(vaParams, "fs_percent");
        card.fanRpm = getReading(vaParams, "fs_rpm");
        videocards[i] = card;
      }
    }
  }
}

void printScreenDefault()
{
  lcd.setCursor(0, 0);
  lcd.print("CPU:");
  lcd.setCursor(4, 0);
  lcd.print(currCPULoadPercent);
  if (currCPULoadPercent.length() == 5) {
    lcd.setCursor(9, 0);
    lcd.print("  ");
  }
  if (currCPULoadPercent.length() == 4) {
    lcd.setCursor(8, 0);
    lcd.print("   ");
  }

  lcd.setCursor(11, 0);
  lcd.print("Mem:");
  lcd.setCursor(15, 0);
  lcd.print(currMemLoadPercent);

  lcd.setCursor(0, 1);
  lcd.print(currMemUsed);
  if (currMemUsed.length() == 6){
    lcd.setCursor(6, 1);
    lcd.print(" ");
  }
  lcd.setCursor(7, 1);
  lcd.print("/");
  lcd.setCursor(8, 1);
  lcd.print(currMemTotal);
  lcd.setCursor(15, 1);
  lcd.print("     ");

  lcd.setCursor(0, 2);
  lcd.print(currTime);
  lcd.setCursor(17, 2);
  lcd.print(currDay);
}

void printCPUScreen()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CPU:");
  lcd.setCursor(4, 0);
  lcd.print(currCPULoadPercent);

  lcd.setCursor(11, 0);
  lcd.print("Mem:");
  lcd.setCursor(15, 0);
  lcd.print(currMemLoadPercent);

  int arrayLength = sizeof(cpuTempArray) / sizeof(cpuTempArray[0]);

  for (int i = 0, xPos = 0, yPos = 1; i < arrayLength; i++) {
    if (i > 21) return;

    if (xPos > 18) {
      xPos = 0;
      yPos += 1;
    }

    lcd.setCursor(xPos, yPos);
    lcd.print(cpuTempArray[i]);

    xPos = xPos + 3;
  }
}

void showVideoCardLogo(VideoCard *card)
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(card->name.substring(0, 19));
}

void printGpuScreen(VideoCard *card)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("F:");
  lcd.setCursor(2, 0);
  lcd.print(card->fanPercent);
  lcd.setCursor(7, 0);
  lcd.print(card->fanRpm);

  lcd.setCursor(0, 1);
  lcd.print("E:");
  lcd.setCursor(2, 1);
  lcd.print(card->engClock);
  lcd.setCursor(15, 1);
  lcd.print("T");
  lcd.setCursor(16, 1);
  lcd.print(card->currTemp);

  lcd.setCursor(0, 2);
  lcd.print("M:");
  lcd.setCursor(2, 2);
  lcd.print(card->memClock);
  lcd.setCursor(15, 2);
  lcd.print("U");
  lcd.setCursor(16, 2);
  lcd.print(card->usage);

  lcd.setCursor(0, 3);
  lcd.print("C:");
  lcd.setCursor(2, 3);
  lcd.print(currCPULoadPercent);
  lcd.setCursor(9, 3);
  lcd.print("M:");
  lcd.setCursor(11, 3);
  lcd.print(currMemLoadPercent);
  lcd.setCursor(18, 3);
  lcd.print(String(card->id + 1));
}

void parseButton()
{
  Serial.println("Clicked");
  ++currentScreen;

  // show different subscreens for VA adapters
  if ((currentScreen == SCREEN_MODE_VA_DISPLAY && vaCount > 0) 
    || (currentScreen == SCREEN_MODE_VA && vaCount > 0)){
    if (currentVaScreen == vaCount - 1) {
      currentVaScreen = 0;
    } else {
      currentVaScreen++;
    }
    return;
  }else{
    lcd.clear(); // clean up lcd on mode change
  }

  // more than last one screen
  if (currentScreen > SCREEN_MODE_VA) {
    currentScreen = SCREEN_MODE_DEFAULT;
  }
}

static int readButton(struct pt *pt, int interval)
{
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);

  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();

    int reading = digitalRead(changeModeButtonPin);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;

        if (buttonState == HIGH){
          parseButton();
        }
      }
    }

    lastButtonState = reading;
  }
  PT_END(pt);
}

static int readSerial(struct pt *pt, int interval)
{
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();

    if (Serial.available() > 0) {
      String s = Serial.readString();
      parseReadings(s);
      Serial.println(s);
      delay(READING_TIMEOUT);
      
      if(currentScreen == SCREEN_MODE_INIT){
        currentScreen++;
      }
    }
  }
  PT_END(pt);
}

static int showScreen(struct pt *pt, int interval)
{
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();

    switch (currentScreen) {    
      case SCREEN_MODE_DEFAULT:
        printScreenDefault();
        break;

      case SCREEN_MODE_CPU:
        printCPUScreen();
        break;

      case SCREEN_MODE_MEM:
        printScreenDefault();
        break;

      case SCREEN_MODE_VA_DISPLAY:
        if (currentVaDispolayScreenShowed > VA_DISPLAY_TIMES_SHOW) {
          currentScreen = SCREEN_MODE_VA;
          currentVaDispolayScreenShowed = 0;
          break;
        } else {
          showVideoCardLogo(&videocards[currentVaScreen]);
          currentVaDispolayScreenShowed++;
          break;
        }

      case SCREEN_MODE_VA:
        printGpuScreen(&videocards[currentVaScreen]);
        break;

      case SCREEN_MODE_INIT:
      default:
        showStartupMessage();
    }

  }
  PT_END(pt);
}

void loop()
{
  readSerial(&serialReedHandlePt, 10);
  readButton(&buttonHanldePt, 10);
  showScreen(&lcdHanldePt, 500);
}