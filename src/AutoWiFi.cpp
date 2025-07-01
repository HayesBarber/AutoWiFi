#include "AutoWiFi.h"
#include <Preferences.h>

Preferences preferences;

AutoWiFi::AutoWiFi() {}

void AutoWiFi::connect() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.isEmpty()) {
        Serial.println("[AutoWiFi] No WiFi credentials found.");
    } else {
        Serial.printf("[AutoWiFi] Found SSID: %s\n", ssid.c_str());
    }
}

void AutoWiFi::update() {}

bool AutoWiFi::isConnected() const {
    return WiFi.isConnected();
}

IPAddress AutoWiFi::getIP() const {
    return WiFi.localIP();
}
