#define BLYNK_TEMPLATE_ID "TMPL6C21JRy5X"
#define BLYNK_TEMPLATE_NAME "FurFeast"
#define BLYNK_AUTH_TOKEN "kUHVFBKuTdchCSEz3nEtOh2S9PDLFmNw"

const char* ssid = "Emiliejeans1_2.4G";
const char* pass = "emilie19";
const char* webApp = "https://script.google.com/macros/s/AKfycbw-YFZw0srT4RFY42tr9TSxoOBGaUt0Ks5bFk3R9lE0mMqrtVMNgZcRnVZ2JrnGAPCR/exec";

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// LCD config
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// RTC config
ThreeWire myWire(12, 27, 13);  // IO, SCLK, CE (adjust pins as needed)
RtcDS1302<ThreeWire> Rtc(myWire);

int servo_def_pos = 0; // 0 or 180, depends on the build

// For betterDelay function
unsigned long delayStartFeed = 0;
bool delayingFeed = false;

unsigned long delayStartBlink = 0;
bool delayingBlink = false;

unsigned long delayStartReset = 0;
bool delayingReset = false;

unsigned long delayStartLog = 0;
bool delayingLog = false;

// Triggers
bool eStop = false;
bool prev_eStop = false;
bool actionStarted = false;
bool timerTriggered = false;
bool isTimer = false; // triggers with onTimer(), only when the clock hits

#define LED_BUILTIN 2
#define LED_Status 4
#define LED_WiFi 5
#define buttonStartPin 18
#define buttonStopPin 19
#define buzzerPin 23
#define servoPin 26

int Power_Status = 0; // LED_BUILTIN (V0)
int prev_Power_Status = 0;
int Feed_Status = 0; // LED_Status (V1)
int WiFi_Status = 0; // LED_WiFi (V2)
int prev_WiFi_Status = 0;

Servo myServo;

// reading the values from Start & E-Stop buttons
bool startValue = digitalRead(buttonStartPin);
bool stopValue = digitalRead(buttonStopPin);

// struct = data container which acts similar to class
struct feedTimer {
  int hr;
  int min;
  int sec;
  String size;
  int duration;
  bool triggered;
};

feedTimer feedingTimes[3] = {
  // {14, 43, 0, "small", 3000, false},
  // {14, 45, 0, "medium", 5000, false},
  // {14, 47, 0, "big", 7000, false}
};

// sizeof() = total size in byte (3 elements * 2 bytes each = 6 bytes)
// total of 6 bytes divided by first byte = 2; ans = 3;
int arrLength = sizeof(feedingTimes) / sizeof(feedingTimes[0]);
int lastIndex = arrLength - 1;

int feed_arrCounter = 0;

struct rtcStatus {
  int rtc_hr;
  int rtc_min;
  int rtc_sec;
};

rtcStatus rtcTimer() {
  rtcStatus status; // create a local struct variable
  RtcDateTime now = Rtc.GetDateTime();

  status.rtc_hr = now.Hour(); // field struct fields
  status.rtc_min = now.Minute();
  status.rtc_sec = now.Second();
  
  return status;
}

rtcStatus status; // declaring (globally)

struct FeedState {
  bool active = false;
  unsigned long startTime = 0;
  int duration = 0;
  String size = "";
};

FeedState currentFeed;

BLYNK_CONNECTED(){
  Blynk.syncVirtual(V4, V5, V6, V13, V14, V15);
}

void setup() {

  Serial.begin(115200);

  // I/O pins setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_Status, OUTPUT);
  pinMode(LED_WiFi, OUTPUT);
  pinMode(buttonStartPin, INPUT);
  pinMode(buttonStopPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Servo setup
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(servo_def_pos);
  
  // LCD setup
  Wire.begin(21, 22); // set SDA and SCLK
  lcd.init(); // initialize the lcd 
  lcd.backlight(); // turn on backlight
  lcd.setCursor(0,0);
  lcd.print("INITIALIZING...");
  delay(3000);
  lcd.clear();        

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    lcd.print("CONNECTING...");
    delay(3000);
    lcd.clear();
  }
  
  Serial.println("WiFi Connected!");

  // statuses (on-start)
  // power status
  digitalWrite(LED_BUILTIN, HIGH);
  Blynk.virtualWrite(V0, HIGH);
  Power_Status = 1;
  prev_Power_Status = Power_Status;

  // wifi status
  if (Blynk.connected()){ // true
    digitalWrite(LED_WiFi, HIGH);
    Blynk.virtualWrite(V2, HIGH);
    WiFi_Status = 1;
    prev_WiFi_Status = WiFi_Status;
  } else {
    digitalWrite(LED_WiFi, LOW);
    Blynk.virtualWrite(V2, LOW);
    WiFi_Status = 0;
    prev_WiFi_Status = WiFi_Status;
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // reset the array (might be a button later)
  // resetArray();

  // read current items in schedule timer array
  readArray(0);
  readArray(1);
  readArray(2);

  resetFeedStatus(); // to be checked *

  Serial.println("Counter (startup): " + String(feed_arrCounter));
  
  // RTC setup
  // Uncomment these for the first time upload, after that comment them again
  // Reupload if the RTC is no longer supply or has no alternate battery
  // RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  // Rtc.SetDateTime(compiled);       // update the status (globally)

  lcd.print("ALL SETUP DONE!");
  delay(3000);
  lcd.clear();
}

void loop() {
  
  Blynk.run();

  WiFi_Status = Blynk.connected();
  
  // status shifting handlers
  // power status
  if (Power_Status != prev_Power_Status) {
    prev_Power_Status = Power_Status; // update
    digitalWrite(LED_BUILTIN, Power_Status ? HIGH : LOW);
    Blynk.virtualWrite(V0, Power_Status ? HIGH : LOW);
  }
  // wifi status
  if (WiFi_Status != prev_WiFi_Status) {
    prev_WiFi_Status = WiFi_Status;
    digitalWrite(LED_WiFi, WiFi_Status ? HIGH : LOW);
    Blynk.virtualWrite(V2, WiFi_Status ? HIGH : LOW);
  }

  // RtcDateTime now = Rtc.GetDateTime(); 
  status = rtcTimer(); // update the status (globally)
  
  startValue = digitalRead(buttonStartPin);
  stopValue = digitalRead(buttonStopPin);

  // E-stop
  eStop = (stopValue == HIGH); // true if pressed, false if released
  // E_STOP(eStop);
  // if (eStop != prev_eStopg){
  //   prev_eStop = eStop;
  
  //   if (eStop) {
  //     myServo.detach();
  //     Serial.println("E-STOP IS ACTIVATED!");
  //     // Blynk.virtualWrite(V12, "E-STOP IS ACTIVATED!"); 
  //   }
  //   else {
  //     myServo.attach(servoPin);
  //     Serial.println("SERVO IS BACK ONLINE!");
  //     // Blynk.virtualWrite(V12, "SERVO IS BACK ONLINE!"); 
  //   }
  // }

  // Manual Feed (button push)
  startFeed("big", 5000, false, false);

  if (feed_arrCounter > 0){ // accept only 3 schedules
    // Blynk.virtualWrite(V12, "Running..."); 
    onTimer();
  }

  delayTillReset();

  // delay(100);
}

// functions down here

int portion(String size){
  if (size=="small") {
    return 30;
  }
  else if (size=="medium"){
    return 60;
  }
  else if (size=="big"){
    return 90;
  }
  else {
    return 0;
  }
}

bool betterDelay(unsigned long duration, unsigned long &delayStart, bool &delaying){
  if (!delaying){ // if delaying == true:
    delayStart = millis(); // snapshot of that moment of time (ex: 1000 ms)
    delaying = true;
  }

  if (millis() - delayStart >= duration) { // if our stopwatch time rn (5000 ms) - another counter, a snapshot (1000ms) >= our delay
    delaying = false; // when our stopwatch reaches the selected delay, we stop it
    return true;
  }

  return false;
}

String rtc_strTimer(){
  RtcDateTime now = Rtc.GetDateTime();

  String formattedTime = "";
  formattedTime += String(now.Day());
  formattedTime += "/";
  formattedTime += String(now.Month());
  // formattedTime += "/";
  // formattedTime += String(now.Year());
  formattedTime += " ";
  formattedTime += String(now.Hour());
  formattedTime += ":";
  formattedTime += String(now.Minute());
  // formattedTime += ":";
  // formattedTime += String(now.Second());

  return formattedTime;
}

// feed on schedule sys
void onTimer(){
  for (int i = 0; i < 3; i++){
    feedTimer &t = feedingTimes[i];
    if (!t.triggered) {
      int rtcSec = status.rtc_hr*3600 + status.rtc_min*60 + status.rtc_sec;
      int timSec = t.hr*3600 + t.min*60 + t.sec;

      Serial.println("RTC(sec)   : " + String(rtcSec));
      Serial.println("TIMER(sec) : " + String(timSec));
      
      if (rtcSec == timSec){
        startFeed(t.size, t.duration, true, false);
        t.triggered = true;

        String msg = "Schedule(" + String(i+1) + ") is fed!";
        Blynk.logEvent("feeding_on_schedule", msg);

        if (i == 0) {
          Blynk.virtualWrite(V7, 1);
          sheetLogger(String(i+1), "Yes", "No");
        } 
        if (i == 1) {
          Blynk.virtualWrite(V8, 1);
          sheetLogger(String(i+1), "Yes", "No");
        }
        if (i == 2) {
          Blynk.virtualWrite(V9, 1);
          sheetLogger(String(i+1), "Yes", "No");
        } 
      }
    }
  }
}


// Feeding sys
void startFeed(String size, int delay, bool isTimer, bool manual){
  if((startValue == HIGH || isTimer || manual) && !currentFeed.active && eStop != true){
    currentFeed.active = true;
    currentFeed.startTime = millis(); // start counting from now
    currentFeed.duration = delay;
    currentFeed.size = size;

    digitalWrite(buzzerPin, HIGH);
    digitalWrite(LED_Status, HIGH);
    Feed_Status = 1;
    Blynk.virtualWrite(V1, Feed_Status);
    Serial.println(isTimer ? "TIMED FEEDING" : "FEEDING...");
    
    lcd.setCursor(0,0);
    lcd.print(isTimer ? "TIMED FEEDING" : "FEEDING...");
    lcd.setCursor(0,1);
    lcd.print(rtc_strTimer());
    myServo.write(portion(size));
  }

  updateFeed();
}

void updateFeed(){
  if (!currentFeed.active){
    return;
  }

  if (millis() - currentFeed.startTime >= currentFeed.duration){  
      digitalWrite(buzzerPin, LOW);
      digitalWrite(LED_Status, LOW);
      Feed_Status = 0;
      Blynk.virtualWrite(V1, Feed_Status);
      myServo.write(servo_def_pos);
      lcd.clear();
      currentFeed.active = false;
    }
}

void resetArray(){
    for(int i = 0; i < 3; i++){
        feedingTimes[i] = {0, 0, 0, "", 0, false};
    }

    feed_arrCounter = 0;
}

void resetFeedStatus(){
    Blynk.virtualWrite(V7, 0);
    Blynk.virtualWrite(V8, 0);
    Blynk.virtualWrite(V9, 0);
}

// Debugger
String readArray(int x){
  feedTimer &t = feedingTimes[x];
  String output = "Time: " + String(t.hr) + ":" + String(t.min) + ":" + String(t.sec)
  + "Duration: " + String(t.duration)
  + "| Is Triggered?: " + String(t.triggered ? "true" : "false"); 

  return output;
}

void delayTillReset(){
  // int lastSchedule = 0;
  feedTimer &t = feedingTimes[lastIndex];
  if (t.triggered) {
    // delay(10000);
    if (betterDelay(10000, delayStartReset, delayingReset)){
      resetSchedule();
      Serial.println("Reset!");
    }
  }
}

void resetSchedule(){
  for (int i = 0; i < 3; i++){
    feedingTimes[i].triggered = false;
  }
  resetFeedStatus();
}

void sheetLogger(String val1, String val2, String val3){
  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String url = String(webApp) + "?value1=" + String(val1) + "&value2=" + String(val2) + "&value3=" + String(val3);
    
    http.begin(url);
    int code = http.GET(); // http respond code (usually 0+, -1 = failed)

    if (code > 0){
      Serial.print("Logged: ");
      Serial.println(http.getString());
    } else {
      Serial.print("Error code: ");
      Serial.println(code);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected!");
  }  

  // betterDelay(5000, delayStartLog, delayingLog);
}

  // Manual Feed on Blynk
BLYNK_WRITE(V3){ // triggers when the value of this Vx changes
    bool triggered = param.asInt(); // get value from app (0 or 1)
    // Serial.println(triggered);
    if (triggered) { // if receive 1 then
      startFeed("big", 5000, false, true);
    } 
  }

BLYNK_WRITE(V4){
  int total = param[0].asInt();
  int hr = total / 3600; // 13 hr
  int min = (total % 3600) / 60; // '%' gives u the remainder
  int sec = total % 60;
  Serial.println("Total Seconds (V4): " + String(total));
  Serial.println(String(hr) + ":" + String(min) + ":" + String(sec));
  feedTimer &t = feedingTimes[0];
  t.hr = hr;
  t.min = min;
  t.sec = sec;
  t.triggered = false;
  feed_arrCounter+=1;
  Serial.println("Counter (V6): " + String(feed_arrCounter));
}

BLYNK_WRITE(V5){
  int total = param[0].asInt();
  int hr = total / 3600; // 13 hr
  int min = (total % 3600) / 60; // '%' gives u the remainder
  int sec = total % 60;
  Serial.println("Total Seconds (V5): " + String(total));
  Serial.println(String(hr) + ":" + String(min) + ":" + String(sec));
  feedTimer &t = feedingTimes[1];
  t.hr = hr;
  t.min = min;
  t.sec = sec;
  t.triggered = false;
  feed_arrCounter+=1;
  Serial.println("Counter (V6): " + String(feed_arrCounter));
}

BLYNK_WRITE(V6){
  int total = param[0].asInt();
  int hr = total / 3600; // 13 hr
  int min = (total % 3600) / 60; // '%' gives u the remainder
  int sec = total % 60;
  Serial.println("Total Seconds (V6): " + String(total));
  Serial.println(String(hr) + ":" + String(min) + ":" + String(sec));
  feedTimer &t = feedingTimes[2];
  t.hr = hr;
  t.min = min;
  t.sec = sec;
  t.triggered = false;
  feed_arrCounter+=1;
  Serial.println("Counter (V6): " + String(feed_arrCounter));
}

BLYNK_WRITE(V10){ // resetSchedule
    bool input = param.asInt();
    if (input) {
      resetArray();
      readArray(0); readArray(1); readArray(2);   
    }
    
}

BLYNK_WRITE(V11){ // button for readSchedule (terminal)
  bool input = param.asInt();
  if (input) {
    String msg = "Schedule:\n";
    for(int i = 0; i < 3; i++){
      feedTimer &t = feedingTimes[i];
      msg += String(i+1) + ") " +
             String(t.hr) + ":" + String(t.min) + ":" + String(t.sec)
             + " | " + t.size 
             + " | " + String(t.duration / 1000) + "s\n"; 
    }
    Serial.print(msg);
    // Blynk.virtualWrite(V12, msg); // readSchedule  
    // Blynk.virtualWrite(V12, "Schedule:\n"); // readSchedule
    Blynk.virtualWrite(V12, msg); // readSchedule
    // for (int i = 0; i < 3; i++) {
    //   Blynk.virtualWrite(V12, String(i+1) + ") " + readArray(i) + "\n");
  }  
}

BLYNK_WRITE(V13){ // schedule 1
  feedTimer &t = feedingTimes[0];
  switch (param.asInt()){ 
    case 0: { // S
      t.size = "small";
      t.duration = 2000;
      break;
    }
    case 1: { // M
      t.size = "medium";
      t.duration = 4000;
      break;
    }
    case 2: { // L
      t.size = "big";
      t.duration = 6000;
      break;
    }
  }
}

BLYNK_WRITE(V14){ // schedule 2
  feedTimer &t = feedingTimes[1];
  switch (param.asInt()){ 
    case 0: { // S
      t.size = "small";
      t.duration = 2000;
      break;
    }
    case 1: { // M
      t.size = "medium";
      t.duration = 4000;
      break;
    }
    case 2: { // L
      t.size = "big";
      t.duration = 6000;
      break;
    }
  }
}

BLYNK_WRITE(V15){ // schedule 3
  feedTimer &t = feedingTimes[2];
  switch (param.asInt()){ 
    case 0: { // S
      t.size = "small";
      t.duration = 2000;
      break;
    }
    case 1: { // M
      t.size = "medium";
      t.duration = 4000;
      break;
    }
    case 2: { // L
      t.size = "big";
      t.duration = 6000;
      break;
    }
  }
}

BLYNK_WRITE(V16){ // e-stop function
  bool input = param.asInt();
  // E_STOP(input);  
}

// void E_STOP(bool state) {
//   if (state != prev_eStop){
//   prev_eStop = state; // state update

//     if (state) {
//       myServo.detach();
//       Serial.println("E-STOP IS ACTIVATED!");
//       // Blynk.virtualWrite(V12, "E-STOP IS ACTIVATED!"); 
//     }
//     else {
//       myServo.attach(servoPin);
//       Serial.println("SERVO IS BACK ONLINE!");
//       // Blynk.virtualWrite(V12, "SERVO IS BACK ONLINE!"); 
//     }
//   }

//   eStop = state;
// }
