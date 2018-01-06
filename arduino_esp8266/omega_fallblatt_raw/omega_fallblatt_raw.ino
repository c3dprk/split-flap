extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <Ticker.h>

#define FLAPCOUNT 62
//#define FLAPCOUNT 40

#define PIN_TRIAC_N D3
#define PIN_STROBE D2
#define PIN_EC_0 D5
#define PIN_EC_1 D1
#define PIN_EC_2 D7
#define PIN_EC_3 D0
#define PIN_EC_4 D6
#define PIN_EC_5 D4
#define STROBE_READ_DELAY 30
#define ENCODER_REREAD_DELAY 4

#define STA_HOSTNAME "flipflap"

const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);

volatile int read_error_count = 0;
volatile uint8_t encoder_val = 1; // last read value
uint8_t encoder_desired_val = 1; // value to stop at
volatile int strobe_int = 0; // interrupt counter
bool timer_running = false; // a timer is active, don't start a new one
Ticker timerReadEncoder;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIAC_N, OUTPUT);
  pinMode(PIN_STROBE, INPUT);
  pinMode(PIN_EC_0, INPUT);
  pinMode(PIN_EC_1, INPUT);
  pinMode(PIN_EC_2, INPUT);
  pinMode(PIN_EC_3, INPUT);
  pinMode(PIN_EC_4, INPUT);
  pinMode(PIN_EC_5, INPUT);
  digitalWrite(PIN_TRIAC_N, 1);
  digitalWrite(PIN_STROBE, 0);

  delay(100);
  Serial.setDebugOutput(true);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.hostname(STA_HOSTNAME);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected! Open http://");
  Serial.print(WiFi.localIP());
  Serial.println(" in your browser");

  server.on("/formData", HTTP_GET, []() {
    sendFormData();
  });

  server.on("/flap", HTTP_GET, []() {
    sendFlap();
  });

  server.on("/flap", HTTP_POST, []() {
    String value = server.arg("value");
    encoder_desired_val = value.toInt() + 1;
    digitalWrite(PIN_TRIAC_N, 0);
  });
  /*
    server.on("/flapDown", HTTP_POST, []() {
      encoder_desired_val--;
      if(encoder_desired_val < 1)
        encoder_desired_val = FLAPCOUNT;
      digitalWrite(PIN_TRIAC_N, 0);
    });
  */
  server.serveStatic("/index.htm", SPIFFS, "/index.htm");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/", SPIFFS, "/index.htm");

  server.begin();

  Serial.println("HTTP server started");

  attachInterrupt(digitalPinToInterrupt(PIN_STROBE), isr_strobe, FALLING);
}

String flapNames[] = {
  "Hält nicht in Reinfeld",
  "Geänderte Wagenreihung",
  "fährt in 3 Teilen",
  "fährt in 2 Teilen",
  "fährt nicht über..",
  "Hält nicht bis..",
  "Hält nicht in..",
  "Hält nicht in..",
  "Hält nicht bis Münster",
  "Hält nicht bis Neumünster",
  "Hält nicht bis Puttgarden",
  "Hält nicht bis Lübeck",
  "Hält nicht bis Hannover",
  "Hält nicht bis Bremen",
  "Hält nicht überall",
  "Bitte Ansage beachten",
  "etwa 45 Min später",
  "etwa 30 Min später",
  "etwa 20 Min später",
  "etwa 15 Min später",
  "etwa 10 Min später",
  "[nichts]",
  "Schienenersatzverkehr ab..",
  "ICE Fahrpreis",
  "Zusätzlicher Zug besonderer Fahrpreis",
  "Schienenersatzverkehr",
  "Hält nicht in..",
  "S..",
  "Hält nicht bis..",
  "Hält nicht bis..",
  "mehr als 2 Std später",
  "etwa 120 Min später",
  "etwa 100 Min später",
  "etwa 80 Min später",
  "etwa 60 Min später",
  "Hält nicht bis..",
  "Hält nicht bis..",
  "S..",
  "Zusätzlicher Zug",
  "Zuschlag siehe Abfahrtplan",
  "mit Zuschlag",
  "Zusätzlicher Zug mit Zuschlag",
  "Zusätzlicher Schnellzug",
  "ALPEN-SEE-EXPRESS",
  "bis Friedrichsruh",
  "ALPEN-SEE-EXPRESS",
  "besonderer Fahrpreis",
  "Messe-Rapid nur 1. Kl.",
  "Reisebüro-Sonderzug",
  "Sonderzug",
  "nur 2. Klasse",
  "Fahrt ins Blaue",
  "Umleitung über Flensburg",
  "HVV Fahrausweise ungültig",
  "Mit [Pax-Waggon]",
  "Nur Schlaf-u.Liegewagen",
  "[Fähre] Helogland",
  "City-Bahn..",
  "S..",
  "S..",
  "S..",
  "S4.."
};

void sendFormData()
{
  String json = "{";

  json += "\"currentFlap\":{";
  json += "\"index\":" + String(encoder_desired_val - 1);
  json += ",\"name\":\"" + flapNames[encoder_desired_val - 1] + "\"}";

  json += ",\"flaps\":[";
  for (uint8_t i = 0; i < FLAPCOUNT; i++)
  {
    json += "\"" + flapNames[i] + "\"";
    if (i < FLAPCOUNT - 1)
      json += ",";
  }
  json += "]";

  json += "}";

  server.send(200, "text/json", json);
  json = String();
}

void sendFlap()
{
  String json = "{";
  json += "\"index\":" + String(encoder_desired_val - 1);
  json += ",\"name\":\"" + flapNames[encoder_desired_val - 1] + "\"";
  json += "}";
  server.send(200, "text/json", json);
  json = String();
}

void readEncoder() {
  int prev_val = encoder_val;
  int oneflip = prev_val - 1;
  if (oneflip < 1)
    oneflip = FLAPCOUNT;

  pinMode(PIN_STROBE, OUTPUT);
  digitalWrite(PIN_STROBE, 0); // pull down strobe, this lets us read the EC pins

  encoder_val = digitalRead(PIN_EC_0);
  encoder_val += digitalRead(PIN_EC_1) << 1;
  encoder_val += digitalRead(PIN_EC_2) << 2;
  encoder_val += digitalRead(PIN_EC_3) << 3;
  encoder_val += digitalRead(PIN_EC_4) << 4;
  encoder_val += digitalRead(PIN_EC_5) << 5;

  if (encoder_val == 63 || encoder_val != oneflip) { // re-trigger a read if we read an unexpected result. this eliminates the need for real-time finetuning of the read interval.
    encoder_val = prev_val;
    timerReadEncoder.once_ms(ENCODER_REREAD_DELAY, readEncoder);
    read_error_count++;
    return;
  }

  if (encoder_val == encoder_desired_val) {
    digitalWrite(PIN_TRIAC_N, 1);  // stop the motor when we have reached the desired position.
  }

  pinMode(PIN_STROBE, INPUT); // strobe back to input, this allows the interrupt to be triggered
  timer_running = false;
  strobe_int = 0;
}

void isr_strobe() {
  strobe_int++;
}

void loop() {
  static int prev_val = 0; // just for printing changes

  server.handleClient();

  if (strobe_int > 0 && !timer_running) {
    timer_running = true;
    timerReadEncoder.once_ms(STROBE_READ_DELAY, readEncoder);
  }

  if (strobe_int > 1) {
    Serial.print("b/o ");Serial.println(strobe_int); // bounce or overrun
  }
  if (read_error_count > 0) {
    Serial.print("re ");Serial.println(read_error_count);
    read_error_count = 0;
  }

  if (encoder_val != prev_val) {
    Serial.println(encoder_val);
    sendFlap();
    prev_val = encoder_val;
  }

  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == 'a')
      encoder_desired_val++;
    if (ch == 's')
      encoder_desired_val--;
    if (encoder_desired_val < 1)
      encoder_desired_val = FLAPCOUNT;
    if (encoder_desired_val > FLAPCOUNT)
      encoder_desired_val = 1;
    if (ch == 'x')
      digitalWrite(PIN_TRIAC_N, 1);
    if (ch == 'y')
      digitalWrite(PIN_TRIAC_N, 0);
  }
}
