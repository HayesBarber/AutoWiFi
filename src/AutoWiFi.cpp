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
        Serial.println("[AutoWiFi] No WiFi credentials found. Configuring access point.");

        if (!WiFi.softAP("TODO")) {
            Serial.println("Failed to create AP.");
            while(1);
        }

        Serial.println("AP created.");
    } else {
        Serial.printf("[AutoWiFi] Found SSID: %s, Will attempt to connect.\n", ssid.c_str());
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        Serial.println("");
        Serial.println("WiFi connected.");
    }
    
    Serial.println("IP address: ");
    Serial.println(getIP());
}

void AutoWiFi::update() {}

bool AutoWiFi::isConnected() const {
    return WiFi.isConnected();
}

IPAddress AutoWiFi::getIP() const {
    return WiFi.localIP();
}
