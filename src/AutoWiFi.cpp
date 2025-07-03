#include "AutoWiFi.h"
#include <Preferences.h>

Preferences preferences;

AutoWiFi::AutoWiFi(const char* apSSID) : _apSSID(apSSID), _state(State::NOT_CONNECTED) {}

AutoWiFi::State AutoWiFi::connect() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("[AutoWiFi] No WiFi credentials found. Configuring access point.");
        _state = startAccessPoint();
    } else {
        Serial.printf("[AutoWiFi] Found SSID: %s. Attempting to connect...\n", ssid.c_str());
        _state = connectToWiFi(ssid, password);
    }

    if (_state != State::NOT_CONNECTED) {
        Serial.println("IP address: ");
        Serial.println(getIP());
    }

    return _state;
}

AutoWiFi::State AutoWiFi::connectToWiFi(const String& ssid, const String& password) {
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
        return State::WIFI_CONNECTED;
    } else {
        Serial.println("[AutoWiFi] Failed to connect to WiFi.");
        return State::NOT_CONNECTED;
    }
}

AutoWiFi::State AutoWiFi::startAccessPoint() {
    if (!WiFi.softAP(_apSSID)) {
        Serial.println("[AutoWiFi] Failed to start AP.");
        return State::NOT_CONNECTED;
    }
    Serial.printf("[AutoWiFi] AP '%s' started.\n", _apSSID);
    return State::AP_MODE;
}

void AutoWiFi::update() {}

IPAddress AutoWiFi::getIP() const {
    return WiFi.localIP();
}

AutoWiFi::State AutoWiFi::getState() const {
    return _state;
}
