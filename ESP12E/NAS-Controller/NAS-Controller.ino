#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp8266.h>

#define ADC  A0
#define FAN  14
#define COM  15

const char auth[] = "64vBgqBMh3kK7eEPWFgMblKib7sNwqLk";
const char ssid[] = "LUFT-AQUILA";
const char pass[] = "rokaFWIf512#";

int isOnline;
bool SVR = true;
bool COM_STAT = false;
bool FAN_STAT = false;

BlynkTimer timer;

WidgetLED PWR(V3);
WidgetLED ACT(V4);

Adafruit_SSD1306 OLED(128, 64, &Wire, -1);

void setup() {
  pinMode(FAN, OUTPUT);
  pinMode(COM, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  delay(300);
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE, BLACK);

  initOLED();
  OLED.display();

  timer.setInterval(500, voltageUpdater);
  timer.setInterval(1000, blynkSync);
  timer.setTimeout(60000, resetOLED);
  isOnline = timer.setTimeout(3000, serverNotAvailable);
}

void loop() {
  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V0) { // COM Control
  COM_STAT = param.asInt();
  OLED.setCursor(30, 9);
  if(COM_STAT) { digitalWrite(COM, HIGH); OLED.print("O"); }
  else         { digitalWrite(COM, LOW ); OLED.print("X"); }
  OLED.display();
}

BLYNK_WRITE(V2) { // FAN Control
  FAN_STAT = param.asInt();
  OLED.setCursor(114, 9);
  if(FAN_STAT) { digitalWrite(FAN, HIGH); OLED.print("O"); }
  else         { digitalWrite(FAN, LOW ); OLED.print("X"); }
  OLED.display();
}

BLYNK_WRITE(V6) { // IP
  char input[100];
  strcpy(input, param.asStr());
  char *ptr = strtok(input, " : ");
  int num = 1;
  while(ptr) {
    if     (num == 1) {
      String publicIP(ptr);
      OLED.setCursor(33, 19);
      OLED.print(publicIP);
      OLED.display();
    }
    else if(num == 2) {
      String localIP(ptr);
      localIP.remove(0, 2);
      OLED.setCursor(33, 30);
      OLED.print(localIP);
      OLED.display();
    }
    ptr = strtok(NULL, "");
    num++;
  }
}

BLYNK_WRITE(V7) { // Uptime
  OLED.setCursor(22, 42);
  OLED.print(param.asStr());
  OLED.display();
}

BLYNK_WRITE(V8) { // Server active Signal
  String input = param.asStr();
  static String prev = String("SERVER OFF");
  if(input != "SERVER OFF") input.remove(21, 2);
  if(!input.equals(prev)) {
    if(timer.isEnabled(isOnline)) timer.restartTimer(isOnline);
    SVR = true;
    ACT.on();
    OLED.setCursor(0, 52);
    OLED.print(input);
    OLED.display();
  }
  else if(SVR && !timer.isEnabled(isOnline)) isOnline = timer.setTimeout(3000, serverNotAvailable);
  prev = input;
}

BLYNK_WRITE(V9) { // CPU Temp
  String cpuTemp = param.asStr();
  OLED.setCursor(91, 0);
  if(cpuTemp == "N/A") OLED.print(cpuTemp);
  else OLED.print(cpuTemp.substring(0, 4) + "'C");
  OLED.display();
}

BLYNK_CONNECTED() { Blynk.syncAll(); }

void voltageUpdater() {
  // Voltage Divider : 98.2kΩ, 19.57kΩ   divide ratio : 0.166
  float serverVoltage = float(analogRead(ADC)) / 1023.0 / 0.166;
  Blynk.virtualWrite(V5, String(serverVoltage, 3) + "V");
  if(serverVoltage > 4.6) PWR.on();  else PWR.off();
  OLED.setCursor(24, 0);
  OLED.print(serverVoltage, 3);
  OLED.print("V");
  OLED.display();
}

void initOLED() {
  OLED.setCursor(0, 0);
  OLED.print("PWR:");
  OLED.setCursor(67, 0);
  OLED.print("CPU:");
  OLED.setCursor(6, 9);
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
  OLED.print(COM_STAT ? "O" : "X");
  OLED.setCursor(114, 9);
  OLED.print(FAN_STAT ? "O" : "X");
}

void serverNotAvailable() {
  ACT.off();
  SVR = false;
  Blynk.virtualWrite(V9, "N/A");
  Blynk.virtualWrite(V8, "SERVER OFF");
  Blynk.virtualWrite(V7, "N/A");
  Blynk.virtualWrite(V6, "N/A : N/A");

  OLED.setCursor(91, 0);
  OLED.print("N/A   ");
  OLED.setCursor(33, 19);
  OLED.print("N/A           ");
  OLED.setCursor(33, 30);
  OLED.print("N/A           ");
  OLED.setCursor(22, 42);
  OLED.print("N/A                 ");
  OLED.setCursor(0, 52);
  OLED.print("SERVER OFF            ");
  OLED.display();
}

void resetOLED() {
  OLED.clearDisplay();
  initOLED();
  timer.restartTimer(isOnline);
  Blynk.syncAll();
}

void blynkSync() { Blynk.syncAll(); }
