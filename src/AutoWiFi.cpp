#include "AutoWiFi.h"
#include <Preferences.h>

Preferences preferences;

AutoWiFi::AutoWiFi(const char* apSSID) : _apSSID(apSSID) {}

bool AutoWiFi::connect() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("[AutoWiFi] No WiFi credentials found. Configuring access point.");
        _startupSuccess = startAccessPoint();
    } else {
        Serial.printf("[AutoWiFi] Found SSID: %s. Attempting to connect...\n", ssid.c_str());
        _startupSuccess = connectToWiFi(ssid, password);
    }

    if (_startupSuccess) {
        Serial.println("IP address: ");
        Serial.println(getIP());
    }

    return _startupSuccess;
}

bool AutoWiFi::connectToWiFi(const String& ssid, const String& password) {
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
        return true;
    } else {
        Serial.println("[AutoWiFi] Failed to connect to WiFi.");
        return false;
    }
}

bool AutoWiFi::startAccessPoint() {
    if (!WiFi.softAP(_apSSID)) {
        Serial.println("[AutoWiFi] Failed to start AP.");
        return false;
    }
    Serial.printf("[AutoWiFi] AP '%s' started.\n", _apSSID);
    return true;
}

void AutoWiFi::update() {}

bool AutoWiFi::isConnected() const {
    return WiFi.isConnected();
}

IPAddress AutoWiFi::getIP() const {
    return WiFi.localIP();
}
