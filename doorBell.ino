#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssidSchool = "MCS_Guest";
const char* ssidHome = "";
const char* password = "";
bool atSchool = false; // Set to true or false as needed

const char* firebaseURL = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";
unsigned long previousMillis = 0; // Store last request time
const long interval = 5000;       // Interval set to 5 seconds
unsigned long blinkInterval = 20; // LED blink interval in milliseconds
unsigned long ledPreviousMillis = 0; // Timer for blinking LED
bool ledState = LOW;
int rev = 31;

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

void loop() {
    unsigned long currentMillis = millis();

    // Fetch the Firebase data every 5 seconds
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        // Call getDataFromFirebase and store the result in a variable
        String firebaseValue = getDataFromFirebase();

        Serial.print("Received Firebase value: ");
        Serial.println(firebaseValue);
        
        // Check the Firebase value
        if (firebaseValue == "true") {
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

}
