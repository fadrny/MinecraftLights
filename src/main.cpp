#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>

const char* ssid = "";
const char* password = "";
const char* url = "https://sandbox.fadrny.com/data.json"; // Změň na URL tvého JSON souboru

#define NUM_LEDS 100 // Počet LED na jednom pásku
#define NUM_STRIPS 4 // Počet pásků
#define DATA_PIN_1 5 // Pin pro první pásek
#define DATA_PIN_2 18 // Pin pro druhý pásek
#define DATA_PIN_3 2 // Pin pro třetí pásek
#define DATA_PIN_4 21 // Pin pro čtvrtý pásek

CRGB leds[NUM_STRIPS][NUM_LEDS];
void setAllLeds(CRGB color, int ticks);
CRGB getColorForTime(int ticks);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Inicializace LED pásků
  FastLED.addLeds<WS2812B, DATA_PIN_1, BGR>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_2, BGR>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_3, BGR>(leds[2], NUM_LEDS);
  FastLED.addLeds<WS2812B, DATA_PIN_4, BGR>(leds[3], NUM_LEDS);
}

void loop() { 
  if ((WiFi.status() == WL_CONNECTED)) { // Check WiFi connection status
    HTTPClient http;
    http.begin(url); // Specify the URL

    http.addHeader("Cache-Control", "no-cache");
    int httpCode = http.GET(); // Make the request

    if (httpCode > 0) { // Check for the returning code
      String payload = http.getString();
      Serial.println(payload);

      // Parse JSON
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      // Předpokládejme, že JSON má klíč "arg0"
      int arg0 = doc["arg0"];
      Serial.println(arg0);

      // Změna barvy na základě arg0
      CRGB color = getColorForTime(arg0);
      setAllLeds(color, arg0);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end(); // Free resources
  }
  delay(500); // Wait for 0.5 seconds
}

CRGB getColorForTime(int ticks) {
  if (ticks >= 0 && ticks < 3000) { // Ráno
    return blend(CRGB::Orange, CRGB::White, map(ticks, 0, 3000, 0, 255));
  } else if (ticks >= 3000 && ticks < 9000) { // Poledne
    return CRGB::White;
  } else if (ticks >= 9000 && ticks < 12000) { // Večer
    return blend(CRGB::White, CRGB::Red, map(ticks, 9000, 12000, 0, 255));
  } else { // Noc
    return CRGB::Blue;
  }
}

void setAllLeds(CRGB color, int ticks) {
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[i][j] = color;
    }
  }

  // Modulace jasu během noci
  if (ticks >= 12000 && ticks < 18000) {
    uint8_t brightness = map(ticks, 12000, 18000, 255, 128); // Sniž jas uprostřed noci
    FastLED.setBrightness(brightness);
  } else {
    FastLED.setBrightness(255); // Plný jas během dne
  }

  FastLED.show();
}