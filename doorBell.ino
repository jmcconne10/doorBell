#define D5 18      // NeoPixel data pin (GPIO18)
#define D6 19      // Buzzer pin (GPIO19)

// Define built-in LED for ESP32 DevKit if not already defined
#ifndef LED_BUILTIN
#define LED_BUILTIN 2   // Onboard LED is usually GPIO2 on ESP32 DevKit V1
#endif

#include <WiFi.h>              // ESP32 WiFi
#include <HTTPClient.h>        // ESP32 HTTP client
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include "jingle.h"
#include "UserSongs.h"

const char* ssidSchool = "MCS_Guest";
const char* ssidHome   = "Bell_Test";
const char* password   = "";   // Home WiFi password
bool atSchool = false;                   // Now unused, but left for minimal change

const char* firebaseALERT = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";
const char* firebaseUSER  = "https://doorbell-338a5-default-rtdb.firebaseio.com/USER.json";
unsigned long previousMillis = 0;
const long valueCheckInterval = 1000;
const long resetInterval = valueCheckInterval - 500;
unsigned long alertResetMillis = 0;
bool alertActive = false;
bool ledState = LOW; // Used for built-in LED
int rev = 50;        // Used to help identify what code is on esp

// LED strip settings
#define LED_PIN    D5      // Pin connected to the data line of the LEDs (uses alias above)
#define NUM_LEDS   300     // Number of LEDs in your strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Buzzer Ping
int buzzerPin = D6;

// Defines the data I pull from firebase
struct FirebaseData {
    String alertValue;
    String userValue;
};

void connectToWiFi() {
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);

    // NEW: Scan and auto-pick school vs home
    Serial.println("Scanning for available WiFi networks...");
    int found = WiFi.scanNetworks();
    bool schoolFound = false;

    for (int i = 0; i < found; i++) {
        Serial.print("Found SSID: ");
        Serial.println(WiFi.SSID(i));

        if (WiFi.SSID(i) == ssidSchool) {
            schoolFound = true;
        }
    }

    if (schoolFound) {
        Serial.println("School SSID detected. Connecting to school (open network)...");
        WiFi.begin(ssidSchool);              // School guest assumed open / no password
    } else {
        Serial.println("School SSID not detected. Connecting to home...");
        WiFi.begin(ssidHome, password);      // Home uses password
    }

    while (WiFi.status() != WL_CONNECTED) {
        // Blink the first LED in the strip to green while trying to connect
        strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green color
        digitalWrite(LED_BUILTIN, LOW);  // Turn LED on
        strip.show();
        delay(500);
        strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn off the LED
        strip.show();
        digitalWrite(LED_BUILTIN, HIGH); // Turn LED off
        delay(500);
        Serial.print(".");
    }
    strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn off the LED after connection
    strip.show();
    Serial.println("");
    Serial.print("Rev ");
    Serial.println(rev);
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    pinMode(LED_BUILTIN, OUTPUT);  // Set built-in LED as output

    // Initialize LED strip
    strip.begin();
    strip.setBrightness(50);
    strip.show(); // Initialize all pixels to 'off'

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    delay(1000);

    // Connect to WiFi
    connectToWiFi();
}

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

// Function to get data from Firebase
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

void loop() {
    unsigned long currentMillis = millis();

    // Reconnect to WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost connection, attempting to reconnect...");
        WiFi.disconnect();
        delay(1000);
        connectToWiFi();
    } else {
        strip.setPixelColor(0, strip.Color(0, 0, 255)); // Turn LED to blue when connected to wifi
        strip.show();
        digitalWrite(LED_BUILTIN, LOW); // Turn LED on
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

            // Turn on the LED if the value is true
            digitalWrite(LED_BUILTIN, LOW); // Turn LED on
            ledState = LOW;                 // Ensure LED stays on
        } else if (firebaseData.alertValue == "false") {
            turnOffLEDStrip(); // Turn off LED strip when ALERT is false
            turnOffBuzzer();   // Turn off the Buzzer
            // Turn off the LED if the value is false
            digitalWrite(LED_BUILTIN, HIGH);
            ledState = HIGH;
        } else {
            Serial.println("No valid data received from Firebase.");
        }
    }
}
