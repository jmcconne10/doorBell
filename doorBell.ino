#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssidSchool = "MCS_Guest";
const char* ssidHome = "";
const char* password = "";
bool atSchool = false; // Set to true or false as needed

const char* firebaseURL = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";
unsigned long previousMillis = 0; // Store last request time
const long valueCheckInterval = 5000;       // How often we check the value of ALERT.json
const long resetInterval = valueCheckInterval - 500;       // How long we leave the value set to True, must be less then valueCheckInterval
unsigned long blinkInterval = 20; // LED blink interval in milliseconds
unsigned long ledPreviousMillis = 0; // Timer for blinking LED
unsigned long alertResetMillis = 0; // Timer for resetting ALERT
bool ledState = LOW;
bool alertActive = false; // New flag to track ALERT state
int rev = 40;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);  // Set built-in LED as output
    delay(5000);

    // Connect to WiFi
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);

    if (atSchool) {
        WiFi.begin(ssidSchool);
    } else {
        WiFi.begin(ssidHome, password);
    }

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Test DNS Resolution
    testDNS();
}

void testDNS() {
    const char* testHost = "example.com";
    Serial.print("Resolving DNS for ");
    Serial.println(testHost);

    IPAddress testIP;
    if (WiFi.hostByName(testHost, testIP)) {
        Serial.print("DNS lookup succeeded. IP Address: ");
        Serial.println(testIP);
    } else {
        Serial.println("DNS lookup failed.");
    }
}

// Modified to return the Firebase value as a String
String getDataFromFirebase() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;      // Use WiFiClientSecure for HTTPS
        client.setInsecure();          // Disable SSL certificate verification
        HTTPClient http;

        Serial.print("Connecting to ");
        Serial.println(firebaseURL);

        // Start connection and send HTTP GET request
        http.begin(client, firebaseURL); // Pass WiFiClientSecure and URL
        int httpCode = http.GET();
        String payload = "";  // Initialize payload as an empty string

        // Check the returning HTTP code
        if (httpCode > 0) {
            Serial.printf("HTTP GET code: %d\n", httpCode);
            Serial.print("Rev ");
            Serial.println(rev);
            if (httpCode == HTTP_CODE_OK) {
                payload = http.getString();  // Store the received value in payload
            }
        } else {
            Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        // End the HTTP connection
        http.end();
        return payload;  // Return the received payload
    } else {
        Serial.println("WiFi not connected");
        return "";  // Return an empty string if not connected
    }
}

void updateFirebaseToFalse() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;      // Use WiFiClientSecure for HTTPS
        client.setInsecure();          // Disable SSL certificate verification
        HTTPClient http;

        Serial.print("Updating Firebase to false at ");
        Serial.println(firebaseURL);

        // Start connection and send HTTP PUT request
        http.begin(client, firebaseURL); // Pass WiFiClientSecure and URL
        http.addHeader("Content-Type", "application/json"); // Set content type to JSON
        int httpCode = http.PUT("false");  // Send "false" as JSON payload

        // Check the returning HTTP code
        if (httpCode > 0) {
            Serial.printf("HTTP PUT code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                Serial.println("Successfully updated Firebase to false.");
            }
        } else {
            Serial.printf("PUT request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        // End the HTTP connection
        http.end();
    } else {
        Serial.println("WiFi not connected. Failed to update Firebase.");
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Fetch the Firebase data every 5 seconds
    if (currentMillis - previousMillis >= valueCheckInterval) {
        previousMillis = currentMillis;
        
        // Call getDataFromFirebase and store the result in a variable
        String firebaseValue = getDataFromFirebase();

        Serial.print("Received Firebase value: ");
        Serial.println(firebaseValue);
        
        // Check the Firebase value
        if (firebaseValue == "true") {
            alertActive = true;             // Set alertActive to true
            alertResetMillis = currentMillis; // Start the reset timer
            digitalWrite(LED_BUILTIN, LOW); // Turn LED on (LOW is on for built-in LED)
            ledState = LOW;                 // Ensure LED stays on
        } else if (firebaseValue == "false") {
            // Blink the LED if the value is false
            if (currentMillis - ledPreviousMillis >= blinkInterval) {
                ledPreviousMillis = currentMillis;
                ledState = !ledState;        // Toggle LED state
                digitalWrite(LED_BUILTIN, ledState);
            }
        } else {
            Serial.println("No valid data received from Firebase.");
        }
    }

    // Check if alertActive is true and reset it after 5 seconds
    if (alertActive && (currentMillis - alertResetMillis >= resetInterval)) {
        alertActive = false; // Reset the alertActive flag
        Serial.println("ALERT reset to false in Firebase");
        digitalWrite(LED_BUILTIN, HIGH); // Turn off LED (HIGH is off for built-in LED)
        
        // Update Firebase to set ALERT to false
        updateFirebaseToFalse();
        Serial.println("ALERT reset to false in Firebase");
    }
}
