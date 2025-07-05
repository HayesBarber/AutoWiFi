#include "AutoWiFi.h"
#include <Preferences.h>
#include <ArduinoOTA.h>

AutoWiFi::AutoWiFi() : _state(State::NOT_CONNECTED) {}

AutoWiFi::State AutoWiFi::connect() {
    checkForDeviceReset();

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

    if (_state == State::WIFI_CONNECTED) {
        setupOTA();
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
    Preferences preferences;
    preferences.begin("ap", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.isEmpty() || password.length() < 8) {
        Serial.println("[AutoWiFi] AP credentials missing or password too short. Cannot start AP.");
        return State::NOT_CONNECTED;
    }

    if (!WiFi.softAP(ssid, password)) {
        Serial.println("[AutoWiFi] Failed to start AP.");
        return State::NOT_CONNECTED;
    }

    Serial.printf("[AutoWiFi] AP '%s' started.\n", ssid);

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
    } else if (_state == State::NOT_CONNECTED || WiFi.status() != WL_CONNECTED) {
        Serial.println("[AutoWiFi] No connection. Attempting to reconnect...");
        State result = connect();
        if (result == State::WIFI_CONNECTED) {
            _state = result;
        } else {
            Serial.println("[AutoWiFi] Reconnection failed. Restarting in 10 seconds...");
            delay(10000);
            ESP.restart();
        }
    } else {
        ArduinoOTA.handle();
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

void AutoWiFi::setAccessPointCredentials(const String& ssid, const String& password) {
    Preferences preferences;
    preferences.begin("ap", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("Access point credentials set");
}

void AutoWiFi::setOTACredentials(const String& hostName, const String& password) {
    Preferences preferences;
    preferences.begin("OTA", false);
    preferences.putString("hostName", hostName);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("OTA credentials set");
}

void AutoWiFi::checkForDeviceReset() {
    Preferences bootPrefs;
    bootPrefs.begin("boot", false);
    int bootCount = bootPrefs.getInt("boot_count", 0);
    bootCount += 1;
    bootPrefs.putInt("boot_count", bootCount);
    bootPrefs.end();

    Serial.printf("[AutoWiFi] current boot count: %d\n", bootCount);

    if (bootCount >= 4) {
        Serial.println("[AutoWiFi] Detected 5 fast reboots. Clearing WiFi credentials.");
        Preferences wifiPrefs;
        wifiPrefs.begin("wifi", false);
        bool result = wifiPrefs.clear();
        wifiPrefs.end();
        Serial.printf("[AutoWiFi] Preferences clear result: %s. Restarting...\n", result ? "success" : "failure");
        ESP.restart();
    }

    xTaskCreatePinnedToCore(
        bootResetTask,
        "BootResetTask",
        1000,
        NULL,
        1,
        NULL,
        1
    );
}

void AutoWiFi::bootResetTask(void* parameter) {
    vTaskDelay(4000 / portTICK_PERIOD_MS);

    Preferences prefs;
    prefs.begin("boot", false);
    prefs.putInt("boot_count", 0);
    prefs.end();

    Serial.println("[AutoWiFi] boot count reset to 0");

    vTaskDelete(NULL);
}

void AutoWiFi::setupOTA() {

}
