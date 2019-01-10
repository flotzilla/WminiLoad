#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "StringSplitter.h"

const int changeModeButtonPin = D5;
const int SCREEN_MODE_DEFAULT = 0; // SHOW ALL
const int SCREEN_MODE_CPU = 1; // SHOW cpu only
const int SCREEN_MODE_MEM = 2; // show mem only

int incomingByte = 0;

int realCores = 0;
int totalThreads = 0;
int readingTimeOut = 100;
bool isPrevReadingReceiveData = false;

String currCPULoadPercent = "";
String cpuTempArray[20];
int cpuTempArraySize = 0;  

String currMemTotal = "";
String currMemUsed = "";
String currMemFree = "";
String currMemLoadPercent = "";

String currTime = "";
String currDay = "";

LiquidCrystal_I2C lcd(0x3F,20,4);

void showStartupMessage(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Waiting for connect");
}

void setup() {
  Serial.begin(115200);

  pinMode(changeModeButtonPin, INPUT);    

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  showStartupMessage();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void parseReadings(String s){  
  if(s.startsWith("cpu_stat")){
    int cpuEndIndx = s.indexOf("cpu_stat_end");
    String cpuInfo = s.substring(strlen("cpu_stat"), cpuEndIndx);    

    int cpuTempArraySize = 0;  
    int occurencesIndexes[20];
    for(int x = 0; x < cpuInfo.length(); x++){
      if(cpuInfo[x] == ','){
          occurencesIndexes[cpuTempArraySize] = x;
          cpuTempArraySize++;
      }
    }
  
    if(cpuTempArraySize > 0){    
      cpuTempArray[0] = cpuInfo.substring(0, occurencesIndexes[0]);      

      for(int i=0, j = 1; i < cpuTempArraySize - 1 ; i++, j++){
        cpuTempArray[j] = cpuInfo.substring(occurencesIndexes[i] + 1, occurencesIndexes[i+1]);        
      }       
    } 

    currCPULoadPercent = cpuInfo.substring(cpuInfo.lastIndexOf(",") + 1);

    int memIndx = s.indexOf("mem_stat");
    int memEndIndx = s.indexOf("mem_stat_end");
    String memInfo = s.substring(memIndx + strlen("mem_stat"), memEndIndx);
    StringSplitter *splitter = new StringSplitter(memInfo, ',', 4);

    if(splitter->getItemCount() == 4){
      currMemTotal = splitter->getItemAtIndex(0);  
      currMemUsed = splitter->getItemAtIndex(1);  
      currMemFree = splitter->getItemAtIndex(2);  
      currMemLoadPercent = splitter->getItemAtIndex(3);  
    }

    int timeIndx = s.indexOf("current_time");
    int timeEndIndx = s.indexOf("current_time_end");
    currTime = s.substring(timeIndx + strlen("current_time"), timeEndIndx);

    int dayIndx = s.indexOf("curr_day");
    int dayEndIndx = s.indexOf("curr_day_end");
    currDay = s.substring(dayIndx + strlen("curr_day"), dayEndIndx);  
  }
}

void printScreenDefault(){
  lcd.setCursor(0,0);
  lcd.print("CPU:");
  lcd.setCursor(4,0);
  lcd.print(currCPULoadPercent);
  if(currCPULoadPercent.length() == 5){
    lcd.setCursor(9,0);
    lcd.print("  ");
  }

  lcd.setCursor(11,0);
  lcd.print("Mem:");
  lcd.setCursor(15,0);
  lcd.print(currMemLoadPercent);

  lcd.setCursor(0,1);
  lcd.print(currMemUsed);
  if(currMemUsed.length() == 6){
    lcd.setCursor(6,1);
    lcd.print(" ");
  }
  lcd.setCursor(7,1);
  lcd.print("/");
  lcd.setCursor(8,1);
  lcd.print(currMemTotal);
  lcd.setCursor(15,1);
  lcd.print("     ");

  lcd.setCursor(0,2);
  lcd.print(currTime);
  lcd.setCursor(17,2);
  lcd.print(currDay);
}

void drawInfo(String val, int xPos, int yPos){
  
}

void printCPUScreen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CPU:");
  lcd.setCursor(4,0);  
  lcd.print(currCPULoadPercent);  

  int arrayLength = sizeof(cpuTempArray)/sizeof(cpuTempArray[0]);

  for(int i=0, xPos=0, yPos=1; i < arrayLength; i++){    
    if(i > 21) return;

    if(xPos > 18){
      xPos = 0; yPos+= 1;
    }

    lcd.setCursor(xPos, yPos);
    lcd.print(cpuTempArray[i]);

    xPos = xPos + 3;
  }
}

void parseButton(){

}

void loop(){
  if (Serial.available() > 0) {    
    String s = Serial.readString();  
    parseReadings(s);
    // printScreenDefault();
    printCPUScreen();
    Serial.println(s);
    // isPrevReadingReceiveData = true;
  }else{
    int reading = digitalRead(changeModeButtonPin);
    Serial.print(reading);

    // if(isPrevReadingReceiveData){
      // showStartupMessage();
      // isPrevReadingReceiveData = false; 
    // }
    delay(readingTimeOut);
  }
}