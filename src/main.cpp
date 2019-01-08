#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "StringSplitter.h"

int incomingByte = 0;
int realCores = 0;
int totalThreads = 0;

String currCPULoadPercent = "";

String currMemTotal = "";
String currMemUsed = "";
String currMemFree = "";
String currMemLoadPercent = "";

String currTime = "";
String currDay = "";

int SCREEN_MODE_DEFAULT = 0; // SHOW ALL
int SCREEN_MODE_CPU = 1; // SHOW cpu only
int SCREEN_MODE_MEM = 2; // show mem only

LiquidCrystal_I2C lcd(0x3F,20,4);

void showStartupMessage(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Waiting for connect");
}

void setup() {
  Serial.begin(115200);

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
    String cpuInfo = s.substring(strlen("cpu_stat_end"), cpuEndIndx);
  
    StringSplitter *cpuSplitter = new StringSplitter(cpuInfo, ',', 200);
    int cpuItemCount = cpuSplitter->getItemCount();    

    for(int i = 0; i < cpuItemCount; i++){
      String item = cpuSplitter->getItemAtIndex(i);
      // Serial.println("Item #" + String(i) + ": " + item);
      if(i == cpuItemCount - 1){
        currCPULoadPercent = item;
      }
    }
    // Serial.println("Cpu load: " + currCPULoadPercent);

    currCPULoadPercent = cpuInfo.substring(cpuInfo.lastIndexOf(",") + 1);
    // Serial.println("Cpu load: " + currCPULoadPercent);

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

void parseButton(){

}

void loop(){
  if (Serial.available() > 0) {    
    String s = Serial.readString();  
    parseReadings(s);
    printScreenDefault();
    Serial.println(s);
  }else{
    showStartupMessage();
    delay(1000);
  }
}