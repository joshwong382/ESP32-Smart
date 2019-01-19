#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <fauxmoESP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// FastLED
#define DATA_PIN    5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];
#define FRAMES_PER_SECOND  120
int led_brightness = 255;
uint8_t gHue = 0; // rotating "base color" on FastLED

#define LIGHT_SW D0
#define MUSIC_TRIG D2
#define MUSIC_EXPORT D3
bool MUSIC_SW = HIGH;

// WIFI
const char ssid[] = "***REMOVED***!!! OLD";
const char pass[] = "donotusee89a";
AsyncWebServer server(9999);
fauxmoESP fauxmo;

void setup() {
  pinMode(LIGHT_SW, OUTPUT);
  pinMode(MUSIC_TRIG, INPUT_PULLUP);
  pinMode(MUSIC_EXPORT, OUTPUT);

  // SIGNALS SETUP
  digitalWrite(LIGHT_SW, LOW);
  digitalWrite(MUSIC_EXPORT, LOW);

  // FastLED
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  WiFi.mode(WIFI_STA);
  wifi_check();

  // fauxmo
  fauxmo.addDevice("Desk LED");
  fauxmo.addDevice("Music LED");
  fauxmo.enable(true);
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

    if (strcmp(device_name,"Desk LED") == 0) {
      // analog LEDs
      digitalWrite(LIGHT_SW, state ? HIGH : LOW);
      // digital LEDs
      if (state == 1 && value != 0) {
        led_brightness = int(value);
        FastLED.setBrightness(led_brightness);
      }
    }
    if (strcmp(device_name, "Music LED") == 0) {
      MUSIC_SW = state ? HIGH : LOW;
    }
    
  });

  // Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String read1 = digitalRead(LIGHT_SW) ? "ON<span style='color:white;'>1</span>" : "OFF";
    String read2 = digitalRead(LIGHT_SW) ? "off" : "on";
    String read3 = MUSIC_SW ? "ON<span style='color:white;'>1</span>" : "OFF";
    String read4 = MUSIC_SW ? "off" : "on";
    String response_html = "";
    response_html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    response_html += "<style>@font-face { font-family: rubrik; src: url(https://edev.i.lithium.com/html/assets/Rubrik_Regular.otf); }";
    response_html += "* { font-family: rubrik; } </style></head>";
    response_html += "<body><table style='font-size: 18px;'>";
    response_html += "<tr><td style='text-align: right;'>LED:</td><td>" + read1 + "</td><td>&emsp;<a href=\"/led/" + read2 + "\">Turn " + read2 + "</a></td></tr>";
    response_html += "<tr><td style='text-align: right;'>Music LED:</td><td>" + read3 + "</td><td>&emsp;<a href=\"/music/" + read4 + "\">Turn " + read4 + "</a></td></tr>";
    response_html += "</table></body></html>";
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", response_html);
    response->addHeader("System", "NodeMCU 1.0");
    response->addHeader("Product", "PC RGB Controller");
    response->addHeader("Designer", "C.H.J. WONG");
    request->send(response);
  });

  server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    fauxmo.setState("Desk LED", true, 0);
    request->redirect("/");
  });

  server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    fauxmo.setState("Desk LED", false, 0);
    request->redirect("/");
  });

  server.on("/led/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(LIGHT_SW)));
  });

  server.on("/music/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    fauxmo.setState("Music LED", true, 0);
    request->redirect("/");
  });

  server.on("/music/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    fauxmo.setState("Music LED", false, 0);
    request->redirect("/");
  });

  server.on("/music/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(MUSIC_SW));
  });

  server.onNotFound(notFound);
  server.begin();

}

void notFound(AsyncWebServerRequest *request) {
    request->redirect("/");
}

String ip2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]); 
}

bool isValidIP(const IPAddress& ipAddress) {
  return (ipAddress[0] > 0 && ipAddress[1] >= 0 && ipAddress[2] >= 0 && ipAddress[3] >=0 && ipAddress[0] < 255 && ipAddress[1] < 255 && ipAddress[2] < 255 && ipAddress[3] < 255);
}

long attempt_connect_timer;
bool wifi_check() {

  if (WiFi.status() != WL_CONNECTED && millis()-attempt_connect_timer >= 10000) {
    attempt_connect_timer = millis();
    WiFi.begin(ssid, pass);
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  IPAddress ip = WiFi.localIP();
  if (!isValidIP(ip)) {
    return false;
  }
  
  return true;
}

/*
enum _mode {
  GETDEV,
  NONE,
  LEDON,
  LEDOFF,
  MUSICON,
  MUSICOFF
};
*/

void loop() {
  fauxmo.handle();

  
  if (MUSIC_SW && !digitalRead(MUSIC_TRIG))
    digitalWrite(MUSIC_EXPORT, HIGH);
  else digitalWrite(MUSIC_EXPORT, LOW);


  // Digital LEDs
  if (digitalRead(MUSIC_EXPORT)) {

    // Music LED
    for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    }
    FastLED.setBrightness(96);
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
    FastLED.setBrightness(led_brightness);
    
  } else if (digitalRead(LIGHT_SW)) {

    // Rainbow
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
    
  } else {

    // Lights Off
    for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    }
    FastLED.show();
    
  }
}
