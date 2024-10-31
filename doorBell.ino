#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "MCS_Guest";
const char* firebaseURL = "https://doorbell-338a5-default-rtdb.firebaseio.com/ALERT.json";

void setup() {
    Serial.begin(115200);
    delay(100);

    // Connect to WiFi
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid);

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
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Fetch data from Firebase
    getDataFromFirebase();
}

void getDataFromFirebase() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;              // Create a WiFiClient object
        HTTPClient http;

        Serial.print("Connecting to ");
        Serial.println(firebaseURL);

        // Start connection and send HTTP GET request
        http.begin(client, firebaseURL); // Pass WiFiClient object and URL
        int httpCode = http.GET();

        // Check the returning HTTP code
        if (httpCode > 0) {
            // HTTP header has been received and is positive
            Serial.printf("HTTP GET code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                // Parse the response payload
                String payload = http.getString();
                Serial.println("Received payload:");
                Serial.println(payload);  // Print the value of ALERT.json
            }
        } else {
            Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        // End the HTTP connection
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

void loop() {
    // Keep the loop empty
}
