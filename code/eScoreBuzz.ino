//This code fetches and displays live matches from a given series ID 24c36b5a-0ae1-40cf-8cf5-dd0b9d1be6bd                
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include "wclogo.h"

HTTPClient http;
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=5*/ 27, /*DC=*/ 26, /*RST=*/ 13, /*BUSY=*/ 33)); // GDEH0154D67 200x200, SSD1681

String match_ID, match_started, match_ended;
String series_id = "bd830e89-3420-4df5-854d-82cfab3e1e04"; //ICC World Cup 2023
String api_key = "API KEY";

const char* ssid = "WIFI SSID";
const char* pass = "PASSWORD";

unsigned long previousMillis = 0;
const long interval = 900000;

int ledPin = 4;
int livematch = 0;

void setup() {
  
  pinMode(ledPin, OUTPUT);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wifi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFI network");
  getlivematch();
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    if(millis() - previousMillis > interval){
      getlivematch();
      previousMillis = millis();
    }
    while(match_started == "true" && match_ended == "false"){
      if(millis() - previousMillis > interval){
        displayScore(match_ID);
        previousMillis = millis();
      }
    }
  }
}

void displayScore(String matchID){
  for(int i = 0; i<8; i++){
    digitalWrite(ledPin, HIGH);
    delay(75);
    digitalWrite(ledPin, LOW);
    delay(75);
  }
  display.setFont(&FreeSansBold24pt7b);
  display.firstPage();
    String match_url = "https://api.cricapi.com/v1/match_info?apikey=" + api_key + "&id=" + matchID;
    http.begin(match_url);
    http.GET();
    DynamicJsonBuffer matchBuffer;
    JsonObject& matchData = matchBuffer.parseObject(http.getStream());
    if (!matchData.success()){
      Serial.println("parseObject() failed");
      return;
    }
    String name = matchData["data"]["name"];
    Serial.println("Match: " + name);

    JsonArray& teaminfoArr = matchData["data"]["teamInfo"];
    String team0name = teaminfoArr[0]["name"];
    String team0shortname = teaminfoArr[0]["shortname"];
    String team1name = teaminfoArr[1]["name"];
    String team1shortname = teaminfoArr[1]["shortname"];

    JsonArray& scoreArr = matchData["data"]["score"];
    String check2ndInning = scoreArr[1];
    
    if(check2ndInning == NULL){ //2nd Innings has not started & 1st Inning is in progress
      String runs0 = scoreArr[0]["r"];
      String wkts0 = scoreArr[0]["w"];
      String over0 = scoreArr[0]["o"];
      String team0 = scoreArr[0]["inning"];
      String status = matchData["data"]["status"];
      int index = team0.indexOf(' ');
      String teamBatting = team0.substring(0, index);
      String teamBattingShort;
      if (teamBatting == team0name){
        teamBattingShort = team0shortname;
      }else{
        teamBattingShort = team1shortname;
      }
      Serial.println(teamBattingShort + " " + runs0 + "/" + wkts0);
      Serial.println(over0);
      do{
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(0, 15);
        display.println(name);
        display.drawFastHLine(0, 45, 200, GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(0, 75);
        display.println(teamBattingShort);
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(0, 120);        
        display.println(runs0 + "/" + wkts0);
        display.setFont(&FreeSans12pt7b);
        display.setCursor(0, 150);
        display.println("(" + over0 + ")");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(0, 175);
        display.println(status);
      }   
      while (display.nextPage());
    }else{
      String over1 = scoreArr[1]["o"];
      String team1 = scoreArr[1]["inning"];
      String status = matchData["data"]["status"];
      int index = team1.indexOf(' ');
      String teamBatting = team1.substring(0, index);
      String teamBattingShort;
      if (teamBatting == team0name){
       teamBattingShort = team0shortname;
      }else{
      teamBattingShort = team1shortname;
      }
      Serial.println(teamBattingShort + " " + runs1 + "/" + wkts1);
      Serial.println(over1);
      do{
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(0, 15);
        display.println(name);
        display.drawFastHLine(0, 45, 200, GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(0, 75);
        display.println(teamBattingShort);
        display.setFont(&FreeSansBold24pt7b);
        display.setCursor(0, 120);
        display.println(runs1 + "/" + wkts1);
        display.setFont(&FreeSans12pt7b);
        display.setCursor(0, 150);
        display.println("(" + over1 + ")");
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(0, 175);
        display.println(status);
      }   
      while (display.nextPage());
    }
  String matchStarted = matchData["data"]["matchStarted"];
  String matchEnded = matchData["data"]["matchEnded"];
  match_started = matchStarted;
  match_ended = matchEnded;
}


void getlivematch(){
  Serial.println("Looking for live match...");
  String series_url = "https://api.cricapi.com/v1/series_info?apikey=" + api_key + "&id=" + series_id;
  http.begin(series_url); 
  http.GET();
  DynamicJsonBuffer jb;
  JsonObject& obj = jb.parseObject(http.getStream());
    
  if (!obj.success()){
    Serial.println("parseObject() failed");
    return;
  }

  String matches = obj["data"]["info"]["matches"];

  JsonArray& matchlistArr = obj["data"]["matchList"];
  for(int i=0; i<matches.toInt(); i++){
    String name = matchlistArr[i]["name"];
    String matchStarted = matchlistArr[i]["matchStarted"];
    String matchEnded = matchlistArr[i]["matchEnded"];
    String status = matchlistArr[i]["status"];
    if(matchStarted == "true" && matchEnded == "false"){
      Serial.print("Match ");
      Serial.print(i);
      Serial.println(" is live");
      String matchID = matchlistArr[i]["id"];
      match_ID = matchID;
      match_started = matchStarted;
      match_ended = matchEnded;
      livematch = 1;
      displayScore(match_ID);
      break;
      
    }
  }
  if(livematch == 0){
    Serial.println("No live matches currently! :(");
    do{
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(0, 0, wc_logo, 200, 200, GxEPD_BLACK);
  }   
  while(display.nextPage());
  }
}
