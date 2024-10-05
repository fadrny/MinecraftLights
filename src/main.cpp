#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>

const char* ssid = "";
const char* password = "";
const char* url = "https://sandbox.fadrny.com/data.json";

const int TICK_SUNRISE_START = 22300;
const int TICK_DAY_START = 0;
const int TICK_NOON = 6000;
const int TICK_SUNSET_START = 12000;
const int TICK_NIGHT_START = 13700;
const int TICK_MIDNIGHT = 18000;
const int TICK_TOTAL = 24000;

#define NUM_LEDS 100
#define NUM_STRIPS 4
#define DATA_PIN_1 5
#define DATA_PIN_2 18
#define DATA_PIN_3 2
#define DATA_PIN_4 21

CRGB leds[NUM_STRIPS][NUM_LEDS];
void setAllLeds(CRGB color, uint8_t brightness);
CRGB getColorForTime(int ticks);
uint8_t getBrightnessForTime(int ticks);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  FastLED.addLeds<WS2812B, DATA_PIN_1, BGR>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_2, BGR>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_3, BGR>(leds[2], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_4, BGR>(leds[3], NUM_LEDS);
}

void loop() { 
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Cache-Control", "no-cache");
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        int arg0 = doc["arg0"];
        Serial.print("Ticks: ");
        Serial.print(arg0);
        
        CRGB color = getColorForTime(arg0);
        uint8_t brightness = getBrightnessForTime(arg0);
        
        Serial.print(", Color: R=");
        Serial.print(color.r);
        Serial.print(" G=");
        Serial.print(color.g);
        Serial.print(" B=");
        Serial.print(color.b);
        Serial.print(", Brightness: ");
        Serial.println(brightness);
        
        setAllLeds(color, brightness);
      } else {
        Serial.println("JSON parsing failed");
      }
    } else {
      Serial.println("HTTP request failed");
    }
    http.end();
  }
  delay(500);
}

CRGB getColorForTime(int ticks) {

  if (ticks >= 0 && ticks < 12000) { // Daytime
    if (ticks < 6000) { // Morning
      return blend(CRGB::LightSkyBlue, CRGB::FloralWhite, map(ticks, 0, 6000, 0, 255));
    } else { // Afternoon
      return blend(CRGB::FloralWhite, CRGB::LightSkyBlue, map(ticks, 6000, 12000, 0, 255));
    }
  } else if (ticks >= 12000 && ticks < 12500) { // Sunset
    return blend(CRGB::LightSkyBlue, CRGB::OrangeRed, map(ticks, 12000, 12500, 0, 255));
  } else if (ticks >= 12500 && ticks < 13000) { // Nightfall
    return blend(CRGB::OrangeRed, CRGB::DarkBlue, map(ticks, 12500, 13000, 0, 255));
  }   else if (ticks >= 13000 && ticks < 23000) { // Nighttime
    return CRGB::DarkBlue;
  } else { // Sunrise (23000-24000)
    return blend(CRGB::DarkBlue, CRGB::LightSkyBlue, map(ticks, 23000, 24000, 0, 255));
  }
}

uint8_t getBrightnessForTime(int ticks) {
  // Normalizace tiků na rozsah 0-24000
  ticks = ticks % 24000;
  
  if (ticks >= 0 && ticks < 12000) { // Daytime
    return 255;
  } else if (ticks >= 12000 && ticks < 13000) { // Sunset
    return map(ticks, 12000, 13000, 255, 64);
  } else if (ticks >= 13000 && ticks < 23000) { // Nighttime
    return 64;
  } else { // Sunrise (23000-24000)
    return map(ticks, 23000, 24000, 64, 255);
  }
}

void setAllLeds(CRGB color, uint8_t brightness) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[i][j] = color;
    }
  }
  FastLED.setBrightness(brightness);
  FastLED.show();
}