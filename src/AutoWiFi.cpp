#include "AutoWiFi.h"
#include <Preferences.h>


AutoWiFi::AutoWiFi() : _apSSID("AutoWiFi"), _apPassword(nullptr), _state(State::NOT_CONNECTED) {}

AutoWiFi::State AutoWiFi::connect() {
    Preferences preferences;
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
    if (_apPassword == nullptr || strlen(_apPassword) < 8) {
        Serial.println("[AutoWiFi] AP password not set or too short. Cannot start AP.");
        return State::NOT_CONNECTED;
    }

    if (!WiFi.softAP(_apSSID, _apPassword)) {
        Serial.println("[AutoWiFi] Failed to start AP.");
        return State::NOT_CONNECTED;
    }

    Serial.printf("[AutoWiFi] AP '%s' started.\n", _apSSID);

    _beacon.begin();
    _beacon.onMessage([](const Message& msg) -> String {
        String ssid = msg.getProperty("ssid");
        String password = msg.getProperty("password");

        if (ssid.isEmpty() || password.isEmpty()) {
            return "Missing SSID or password";
        }

        Preferences preferences;
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();

        Serial.println("[AutoWiFi] Credentials saved. Restarting...");
        delay(1000);
        ESP.restart();
        return "Credentials saved, restarting...";
    });

    return State::AP_MODE;
}

void AutoWiFi::loop() {
    if (_state == State::AP_MODE) {
        _beacon.loop();
    } else if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[AutoWiFi] No wifi connection. Attempting to reconnect...");
        State result = connect();
        if (result == State::WIFI_CONNECTED) {
            _state = result;
        } else {
            Serial.println("[AutoWiFi] Reconnection failed. Restarting in 10 seconds...");
            delay(10000);
            ESP.restart();
        }
    }
}

IPAddress AutoWiFi::getIP() const {
    switch (_state) {
        case State::WIFI_CONNECTED:
            return WiFi.localIP();
        case State::AP_MODE:
            return WiFi.softAPIP();
        case State::NOT_CONNECTED:
        default:
            return IPAddress(0, 0, 0, 0);
    }
}

AutoWiFi::State AutoWiFi::getState() const {
    return _state;
}

void AutoWiFi::setAccessPointCredentials(const char* ssid, const char* password) {
    _apSSID = ssid;
    _apPassword = password;

    Preferences preferences;
    preferences.begin("ap", false);
    preferences.putString("ssid", _apSSID);
    preferences.putString("password", _apPassword);
    preferences.end();
}
