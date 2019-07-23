#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp8266.h>

#define ADC  A0
#define FAN  14
#define HDD  15
#define SVR  16

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char auth[] = "64vBgqBMh3kK7eEPWFgMblKib7sNwqLk";
const char ssid[] = "LUFT-AQUILA";
const char pass[] = "rokaFWIf512#";

BlynkTimer timer;
bool client = false;
WidgetLED LED1(V3);
WidgetLED LED2(V4);

void setup() {
  pinMode(FAN, OUTPUT);
  pinMode(HDD, OUTPUT);
  pinMode(SVR, OUTPUT);

  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE, BLACK);
  OLED.setCursor(0, 0);
  OLED.print("Voltage : ");
  OLED.setCursor(0, 20);
  OLED.print("CPU Temp : ");
  OLED.setCursor(0, 30);
  OLED.print("IP : ");
  OLED.setCursor(0, 40);
  OLED.print("Uptime : ");
  OLED.setCursor(0, 50);
  OLED.print("Time : ");
  OLED.display();

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(500L, voltageUpdater);
}

void loop() {
  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V0) { // HDD Control
  int stat = param.asInt();
  if(stat) digitalWrite(HDD, HIGH);
  else     digitalWrite(HDD, LOW);
}

BLYNK_WRITE(V1) { // Main Control
  int stat = param.asInt();
  if(stat) digitalWrite(SVR, HIGH);
  else     digitalWrite(SVR, LOW);
}

BLYNK_WRITE(V2) { // FAN Control
  int stat = param.asInt();
  if(stat) digitalWrite(FAN, HIGH);
  else     digitalWrite(FAN, LOW);
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_APP_CONNECTED() {
  client = true;
}

BLYNK_APP_DISCONNECTED() {
  client = false;
}

void voltageUpdater() {
  float serverVoltage = float(analogRead(ADC)) / 1023.0 / 0.166; // Voltage Divider : 98.2kΩ, 19.57kΩ

  OLED.setCursor(60, 0);
  OLED.print(serverVoltage, 3);
  OLED.print("V");
  OLED.display();

  if(client) Blynk.virtualWrite(V5, String(serverVoltage, 3) + "V");
  if(client && serverVoltage > 3) LED1.on();  else LED1.off();
}
