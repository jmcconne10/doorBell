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

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();

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

        // Print WiFi status for debugging
        int status = WiFi.status();
        switch (status) {
            case WL_IDLE_STATUS:
                Serial.println(" WiFi Status: IDLE");
                break;
            case WL_NO_SSID_AVAIL:
                Serial.println(" WiFi Status: SSID not available");
                break;
            case WL_CONNECT_FAILED:
                Serial.println(" WiFi Status: Connection failed");
                break;
            case WL_CONNECTION_LOST:
                Serial.println(" WiFi Status: Connection lost");
                break;
            case WL_DISCONNECTED:
                Serial.println(" WiFi Status: Disconnected");
                break;
            default:
                Serial.print(" WiFi Status Code: ");
                Serial.println(status);
                break;
        }
    }
    Serial.println("rev12");
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

// Updated to return the Firebase value as a String
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
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        // Call getDataFromFirebase and store the result in a variable
        String firebaseValue = getDataFromFirebase();
        
        // Print the value if it's not empty
        if (firebaseValue.length() > 0) {
            Serial.print("Received Firebase value: ");
            Serial.println(firebaseValue);
        } else {
            Serial.println("No data received from Firebase.");
        }
    }
}
