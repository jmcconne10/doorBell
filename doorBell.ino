#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>

const char* ssidSchool = "MCS_Guest";
const char* ssidHome = "";
const char* password = "";
bool atSchool = false;

const char* firebaseURL = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";
unsigned long previousMillis = 0;
const long valueCheckInterval = 5000;
const long resetInterval = valueCheckInterval - 500;
unsigned long alertResetMillis = 0;
bool alertActive = false;
bool ledState = LOW; // Used for built-in LED
int rev = 44; //Used to help identify what code is on esp

// LED strip settings
#define LED_PIN    D5      // Pin connected to the data line of the LEDs
#define NUM_LEDS   300      // Number of LEDs in your strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Buzzer Ping
int buzzerPin = D6;

void connectToWiFi() {
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);

    if (atSchool) {
        WiFi.begin(ssidSchool);
        Serial.println("At School");
    } else {
        WiFi.begin(ssidHome, password);
        Serial.println("At Home");
    }

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
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
String getDataFromFirebase() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        Serial.print("Connecting to ");
        Serial.println(firebaseURL);

        http.begin(client, firebaseURL);
        int httpCode = http.GET();
        String payload = "";

        if (httpCode > 0) {
            Serial.printf("HTTP GET code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                payload = http.getString();
            }
        } else {
            Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
        return payload;
    } else {
        Serial.println("WiFi not connected");
        return "";
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Reconnect to WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost connection, attempting to reconnect...");
        WiFi.disconnect();
        delay(1000);
        connectToWiFi();
    }

    // Fetch the Firebase data every 5 seconds
    if (currentMillis - previousMillis >= valueCheckInterval) {
        previousMillis = currentMillis;
        
        // Call getDataFromFirebase and store the result in a variable
        String firebaseValue = getDataFromFirebase();

        Serial.print("Received Firebase value: ");
        Serial.println(firebaseValue);
        
        // Check the Firebase value
        if (firebaseValue == "true") {
            alertActive = true;
            alertResetMillis = currentMillis;
            turnOnLEDStrip(); // Turn on LED strip when ALERT is true
            turnOnBuzzer(); // Turn on Buzzer
            // Turn on the LED if the value is true
            digitalWrite(LED_BUILTIN, LOW); // Turn LED on (LOW is on for built-in LED)
            ledState = LOW;                 // Ensure LED stays on
        } else if (firebaseValue == "false") {
            turnOffLEDStrip(); // Turn off LED strip when ALERT is false
            turnOffBuzzer(); //Turn off the Buzzer
            // Turn off the LED if the value is false
            digitalWrite(LED_BUILTIN, HIGH);
            ledState = HIGH;  
        } else {
            Serial.println("No valid data received from Firebase.");
        }
    }
}
