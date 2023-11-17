#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";
const char* apiUrl = "https://pcabackendapi.azurewebsites.net/api/v1/PowerConsumption"; // Replace with your API endpoint

const char* deviceSerialKey = "ESP32_001";
const float consumedUnits = 25.5; 
const int sendInterval = 60000; // Interval in milliseconds (1 minute)

AsyncUDP udp;

void setup() {
  Serial.begin(115200);
  connectToWiFi();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    sendDataToAPI();
    delay(sendInterval);
  } else {
    connectToWiFi();
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

void sendDataToAPI() {
  AsyncHTTPClient http;

  // Get UTC time
  time_t now = time(nullptr);
  struct tm* timeinfo;
  timeinfo = gmtime(&now);

  // Format logTimestamp in UTC
  char formattedTime[25]; // Adjust buffer size if necessary
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%dT%H:%M:%SZ", timeinfo);

  DynamicJsonDocument payload(200);
  payload["deviceSerialKey"] = deviceSerialKey;
  payload["consumedUnits"] = consumedUnits;
  payload["logTimestamp"] = formattedTime;

  String jsonString;
  serializeJson(payload, jsonString);

  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");

  http.POST(jsonString, [](AsyncHTTPClient* httpClient, AsyncHTTPResponse* response) {
    int httpResponseCode = response->statusCode();
    if (httpResponseCode > 0) {
      String payload = response->getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
  });

  http.end();
}
