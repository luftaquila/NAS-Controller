#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp8266.h>

#define ADC  A0
#define FAN  14
#define HDD  15
#define COM  16

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char auth[] = "YOUR_AUTH_TOKEN";
const char ssid[] = "YOUR_SSID";
const char pass[] = "YOUR_PASSWORD";

BlynkTimer timer;

WidgetLED LED1(V3);
WidgetLED LED2(V4);

bool status = false;
bool autoFan = false;
bool isNew = true;

int isOnline;
int HDD_STAT = false;
int COM_STAT = false;
int FAN_STAT = false;

char rcvData[130];
char cputemp[50] = "00.0";
char localIP[50]  = "Starting";
char publicIP[50] = "Device";
char upTime[50]  = "0sec";
char nowTime[50] = "1999-05-12 00:00:00.0";

float svrVolt;

void setup() {
  pinMode(FAN, OUTPUT);
  pinMode(HDD, OUTPUT);
  pinMode(COM, OUTPUT);

  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  delay(500);
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE, BLACK);

  initOLED();
  OLED.display();

  timer.setInterval(500, updater);
  timer.setInterval(60000, rstOLED);
  isOnline = timer.setTimeout(3000, serverNotAvailable);
}

void loop() {
  Blynk.run();
  timer.run();
  receiver();
}

BLYNK_WRITE(V0) { // HDD Control
  HDD_STAT = param.asInt();
  OLED.setCursor(30, 9);
  if(HDD_STAT) { digitalWrite(HDD, HIGH); OLED.print("O"); }
  else         { digitalWrite(HDD, LOW ); OLED.print("X"); }
  OLED.display();
}

BLYNK_WRITE(V1) { // Main Control
  COM_STAT = param.asInt();
  OLED.setCursor(72, 9);
  if(COM_STAT) { digitalWrite(COM, HIGH); OLED.print("O"); }
  else         { digitalWrite(COM, LOW ); OLED.print("X"); }
  OLED.display();
}

BLYNK_WRITE(V2) { // FAN Control
  FAN_STAT = param.asInt();
  OLED.setCursor(114, 9);
  if(FAN_STAT) { digitalWrite(FAN, HIGH); OLED.print("O"); }
  else if(autoFan) Blynk.virtualWrite(V2, HIGH);
  else { digitalWrite(FAN, LOW ); OLED.print("X"); }
  OLED.display();
}

BLYNK_CONNECTED() { Blynk.syncAll(); }

void updater() {
  // Voltage Divider : 98.2kΩ, 19.57kΩ   divide ratio : 0.166
  float serverVoltage = svrVolt = float(analogRead(ADC)) / 1023.0 / 0.166;
  float temp = String(cputemp).toFloat();

  if(serverVoltage > 4.6) LED1.on();  else LED1.off();
  if(status)            LED2.on();  else LED2.off();
  Blynk.virtualWrite(V5, String(serverVoltage, 3) + "V");
  Blynk.virtualWrite(V6, String(publicIP) + " : " + String(localIP));
  Blynk.virtualWrite(V7, upTime);
  Blynk.virtualWrite(V8, nowTime);
  Blynk.virtualWrite(V9, String(cputemp) + "°C");

  if(temp > 60.0 && !FAN_STAT) {
    autoFan = true;
    FAN_STAT = HIGH;
    Blynk.virtualWrite(V2, HIGH);
    digitalWrite(FAN, HIGH);
    OLED.setCursor(114, 9);
    OLED.print("O");
    OLED.display();
  }
  else if(temp < 60.0 && FAN_STAT && autoFan) {
    autoFan = false;
    FAN_STAT = LOW;
    Blynk.virtualWrite(V2, LOW);
    digitalWrite(FAN, LOW);
    OLED.setCursor(114, 9);
    OLED.print("X");
    OLED.display();
  }
  updateOLED(serverVoltage);
}

void receiver() {
  if(Serial.available()) {
    if(timer.isEnabled(isOnline)) timer.restartTimer(isOnline);

    Serial.readStringUntil('|').toCharArray(rcvData, 150);
    if(isNew) { isNew = !isNew; return; }

    byte count = 0;
    bool isValid = false;
    char *p = strtok(rcvData, "/");
    while(p) {
      if (!count) {
        if(!strcmp(p, "+")) status = true;
        else return;
      }
      else if(count == 1) strcpy(cputemp,  p);
      else if(count == 2) strcpy(publicIP, p);
      else if(count == 3) strcpy(localIP,  p);
      else if(count == 4) strcpy(upTime,   p);
      else if(count == 5) {
        if(strcmp(nowTime, p)) strcpy(nowTime,  p);
        else serverNotAvailable();
      }
      p = strtok(NULL, "/");
      count++;
    }
  }
  else if(!timer.isEnabled(isOnline)) isOnline = timer.setTimeout(3000, serverNotAvailable);
}

void initOLED() {
  OLED.setCursor(0, 0);
  OLED.print("PWR:");
  OLED.setCursor(67, 0);
  OLED.print("CPU:");
  OLED.setCursor(6, 9);
  OLED.print("HDD:");
  OLED.setCursor(48, 9);
  OLED.print("COM:");
  OLED.setCursor(90, 9);
  OLED.print("FAN:");
  OLED.setCursor(0, 19);
  OLED.print("P.IP:");
  OLED.setCursor(0, 30);
  OLED.print("L.IP:");
  OLED.setCursor(0, 42);
  OLED.print("UP:");
  OLED.setCursor(30, 9);
  OLED.print(HDD_STAT ? "O" : "X");
  OLED.setCursor(72, 9);
  OLED.print(COM_STAT ? "O" : "X");
  OLED.setCursor(114, 9);
  OLED.print(FAN_STAT ? "O" : "X");
}

void updateOLED(float serverVoltage) {
  OLED.setCursor(24, 0);
  OLED.print(serverVoltage, 3);
  OLED.print("V");
  OLED.setCursor(91, 0);
  OLED.print(cputemp);
  if(strcmp(cputemp, "N/A   ")) OLED.print("'C");
  OLED.setCursor(33, 19);
  OLED.print(publicIP);
  OLED.setCursor(33, 30);
  OLED.print(localIP);
  OLED.setCursor(22, 42);
  OLED.print(upTime);

  String str = String(nowTime);
  str.remove(21, 2);
  OLED.setCursor(0, 52);
  OLED.print(str);

  OLED.display();
}

void rstOLED() {
  OLED.clearDisplay();
  initOLED();
  updateOLED(svrVolt);
}

void serverNotAvailable() {
  isNew = true;
  status = false;
  strcpy(cputemp,  "N/A   ");
  strcpy(publicIP, "N/A           ");
  strcpy(localIP,  "N/A           ");
  strcpy(upTime,   "N/A                 ");
  strcpy(nowTime,  "SERVER OFF            ");
  OLED.display();
  while(Serial.available()) Serial.read();
}
