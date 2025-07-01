#include "AutoWiFi.h"
#include <Preferences.h>

Preferences preferences;

AutoWiFi::AutoWiFi(const char* apSSID) : _apSSID(apSSID) {}

void AutoWiFi::connect() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("[AutoWiFi] No WiFi credentials found. Configuring access point.");
        startAccessPoint();
    } else {
        Serial.printf("[AutoWiFi] Found SSID: %s. Attempting to connect...\n", ssid.c_str());
        connectToWiFi(ssid, password);
    }

    Serial.println("IP address: ");
    Serial.println(getIP());
}

void AutoWiFi::connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());

    int retries = 0;
    const int maxRetries = 20;

    while (WiFi.status() != WL_CONNECTED && retries++ < maxRetries) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[AutoWiFi] WiFi connected.");
    } else {
        Serial.println("[AutoWiFi] Failed to connect to WiFi.");
    }
}

void AutoWiFi::startAccessPoint() {
    if (!WiFi.softAP(_apSSID)) {
        Serial.println("[AutoWiFi] Failed to start AP.");
        while (1);
    }
    Serial.printf("[AutoWiFi] AP '%s' started.\n", _apSSID);
}

void AutoWiFi::update() {}

bool AutoWiFi::isConnected() const {
    return WiFi.isConnected();
}

IPAddress AutoWiFi::getIP() const {
    return WiFi.localIP();
}
