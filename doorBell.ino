#define D13 13      // NeoPixel data pin (GPIO18)
#define D12 12      // Buzzer pin (GPIO19)

// Define built-in LED for ESP32 DevKit if not already defined
#ifndef LED_BUILTIN
#define LED_BUILTIN 2   // Onboard LED is usually GPIO2 on ESP32 DevKit V1
#endif

// LED polarity helpers (for your board: HIGH = ON, LOW = OFF)
#define LED_ON  HIGH
#define LED_OFF LOW

#include <WiFi.h>              // ESP32 WiFi
#include <HTTPClient.h>        // ESP32 HTTP client
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include "jingle.h"
#include "UserSongs.h"

const char* ssidSchool = "MCS_Guest";
const char* ssidHome   = "Bell_Test";
const char* password   = "";   // Home WiFi password
bool atSchool = false;                   // Now unused, kept for minimal diff

const char* firebaseALERT = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";
const char* firebaseUSER  = "https://doorbell-338a5-default-rtdb.firebaseio.com/USER.json";
unsigned long previousMillis = 0;
const long valueCheckInterval = 1000;
const long resetInterval = valueCheckInterval - 500;
unsigned long alertResetMillis = 0;
bool alertActive = false;
bool ledState = LOW; // legacy, not really used for logic anymore
int rev = 50;        // Used to help identify what code is on esp

// LED strip settings
#define LED_PIN    D13      // Pin connected to the data line of the LEDs (uses alias above)
#define NUM_LEDS   300     // Number of LEDs in your strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Buzzer Ping
int buzzerPin = D12;

// Defines the data I pull from firebase
struct FirebaseData {
    String alertValue;
    String userValue;
};

// ---------- BUILT-IN LED STATUS STATE MACHINE ----------

enum LedMode {
    LED_MODE_WIFI_OK,     // solid ON
    LED_MODE_ALERT,       // fast blink
    LED_MODE_WIFI_DOWN    // slow blink
};

LedMode ledMode = LED_MODE_WIFI_DOWN;
unsigned long lastLedMillis = 0;
bool statusLedOn = false;

void setLedMode(LedMode newMode) {
    if (ledMode != newMode) {
        ledMode = newMode;
        lastLedMillis = 0;
        statusLedOn = false;
        digitalWrite(LED_BUILTIN, LED_OFF); // start from OFF
    }
}

void updateStatusLed() {
    unsigned long now = millis();

    switch (ledMode) {
        case LED_MODE_WIFI_OK:
            // Solid ON
            digitalWrite(LED_BUILTIN, LED_ON);
            statusLedOn = true;
            break;

        case LED_MODE_ALERT:
            Serial.println("LED_MODE_ALERT Triggered");
            // Fast blink (every 200 ms)
            if (now - lastLedMillis >= 200) {
                lastLedMillis = now;
                statusLedOn = !statusLedOn;
                digitalWrite(LED_BUILTIN, statusLedOn ? LED_ON : LED_OFF);
                Serial.println("LED_MODE_ALERT Triggered");
            }
            break;

        case LED_MODE_WIFI_DOWN:
            // Slow blink (every 1000 ms)
            if (now - lastLedMillis >= 1000) {
                lastLedMillis = now;
                statusLedOn = !statusLedOn;
                digitalWrite(LED_BUILTIN, statusLedOn ? LED_ON : LED_OFF);
            }
            break;
    }
}

// ---------- WIFI CONNECT ----------

void connectToWiFi() {
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    // Give radio a moment
    delay(500);

    // --- Scan once just to see if we're at HOME (SSID "Loading") ---
    bool loadingFound = false;

    Serial.println("Scanning for WiFi networks...");
    int found = WiFi.scanNetworks();
    Serial.print("  Networks found: ");
    Serial.println(found);

    for (int i = 0; i < found; i++) {
        String ssid = WiFi.SSID(i);
        Serial.print("  SSID[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(ssid);

        if (ssid == "Loading") {      // <--- HOME marker
            loadingFound = true;
            Serial.println("  --> Detected HOME marker SSID 'Loading'");
        }
    }

    if (loadingFound) {
        // We're at home: connect to home WiFi
        Serial.println("Connecting to HOME (Bell_Test)...");
        WiFi.begin(ssidHome, password);
    } else {
        // No "Loading" seen: force school guest network
        Serial.println("No 'Loading' SSID detected. Forcing SCHOOL network (MCS_Guest)...");
        WiFi.begin(ssidSchool);   // open network, no password
    }

    // --- Connect loop: keep trying until we get WL_CONNECTED ---
    while (WiFi.status() != WL_CONNECTED) {
        // Blink NeoPixel #0 green and built-in LED while trying
        strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
        strip.show();
        digitalWrite(LED_BUILTIN, LED_ON);
        delay(250);

        strip.setPixelColor(0, strip.Color(0, 0, 0));   // Off
        strip.show();
        digitalWrite(LED_BUILTIN, LED_OFF);
        delay(250);

        Serial.print(".");
        // Optional: you could periodically call WiFi.begin(ssidSchool) again here
    }

    // Clear pixel once connected
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();

    Serial.println();
    Serial.print("Rev ");
    Serial.println(rev);
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Now that we're connected and idle => WiFi OK mode
    setLedMode(LED_MODE_WIFI_OK);
}


// ---------- SETUP ----------

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);   // Set built-in LED as output
    digitalWrite(LED_BUILTIN, LED_OFF); // Start off

    // Initialize LED strip
    strip.begin();
    strip.setBrightness(50);
    strip.show(); // Initialize all pixels to 'off'

    pinMode(buzzerPin, OUTPUT);
    delay(1000);

    // Initially, WiFi is down
    setLedMode(LED_MODE_WIFI_DOWN);

    // Connect to WiFi
    connectToWiFi();
}

// ---------- LED STRIP & BUZZER HELPERS ----------

// Function to turn on the LED strip (set all LEDs to red)
void turnOnLEDStrip() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red color
    }
    strip.show();
    Serial.println("LED strip turned ON.");
}

// Function to turn off the LED strip (set all LEDs to off)
void turnOffLEDStrip() {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0)); // Off
    }
    strip.show();
    Serial.println("LED strip turned OFF.");
}

void turnOnBuzzer() {
  tone(buzzerPin, 4000); // Generate a 4 kHz tone
}

void turnOffBuzzer() {
  noTone(buzzerPin); // Stop the tone
}

// ---------- FIREBASE ----------

FirebaseData getDataFromFirebase() {
    FirebaseData data = {"", ""};

    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        // Pull the ALERT value
        Serial.print("Connecting to ");
        Serial.println(firebaseALERT);
        http.begin(client, firebaseALERT);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.printf("HTTP GET code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                data.alertValue = http.getString();
            }
        } else {
            Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();

        // Pull the USER value
        Serial.print("Connecting to ");
        Serial.println(firebaseUSER);
        http.begin(client, firebaseUSER);
        httpCode = http.GET();

        if (httpCode > 0) {
            Serial.printf("HTTP GET code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                data.userValue = http.getString();
            }
        } else {
            Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }

    return data;
}

String trimQuotes(String str) {
    if (str.startsWith("\"") && str.endsWith("\"")) {
        return str.substring(1, str.length() - 1); // Remove the first and last characters
    }
    return str;
}

// ---------- MAIN LOOP ----------

void loop() {
    unsigned long currentMillis = millis();

    // Reconnect to WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost connection, attempting to reconnect...");
        setLedMode(LED_MODE_WIFI_DOWN);
        WiFi.disconnect();
        delay(1000);
        connectToWiFi();
    } else {
        // Show blue on NeoPixel 0 when connected
        strip.setPixelColor(0, strip.Color(0, 0, 255));
        strip.show();
    }

    // Fetch the Firebase data every 1 second
    if (currentMillis - previousMillis >= valueCheckInterval) {
        previousMillis = currentMillis;

        // Get both ALERT and USER values
        FirebaseData firebaseData = getDataFromFirebase();

        Serial.print("Received ALERT value: ");
        Serial.println(firebaseData.alertValue);
        Serial.print("Received USER value: ");
        Serial.println(firebaseData.userValue);

        // Check the Firebase value
        if (firebaseData.alertValue == "true") {
            // Remove quotes from userValue
            String trimmedUserValue = trimQuotes(firebaseData.userValue);

            alertActive = true;
            alertResetMillis = currentMillis;
            turnOnLEDStrip(); // Turn on LED strip when ALERT is true

            // Find the user in the array and play their song
            bool userFound = false;
            for (int i = 0; i < userCount; i++) {
                if (userSongs[i].userName == trimmedUserValue) {
                    userSongs[i].playSong(); // Call the song function
                    Serial.print("Playing song for ");
                    Serial.println(userSongs[i].userName);
                    userFound = true;
                    break;
                }
            }

            if (!userFound) {
                Serial.println("No song found for this user.");
                turnOnBuzzer();
            }

            // Built-in LED: alert mode = fast blink
            setLedMode(LED_MODE_ALERT);

        } else if (firebaseData.alertValue == "false") {
            alertActive = false;
            turnOffLEDStrip(); // Turn off LED strip when ALERT is false
            turnOffBuzzer();   // Turn off the Buzzer

            // LED mode depends on WiFi state
            if (WiFi.status() == WL_CONNECTED) {
                setLedMode(LED_MODE_WIFI_OK);   // Solid ON
            } else {
                setLedMode(LED_MODE_WIFI_DOWN); // Slow blink
            }
        } else {
            Serial.println("No valid data received from Firebase.");
        }
    }

    // Update built-in status LED pattern
    updateStatusLed();
}
