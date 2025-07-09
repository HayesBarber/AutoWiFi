#include "AutoWiFi.h"
#include <Preferences.h>
#include <ArduinoOTA.h>

namespace {
    constexpr const char* WIFI_NS = "wifi";
    constexpr const char* AP_NS   = "ap";
    constexpr const char* OTA_NS  = "OTA";
    constexpr const char* BOOT_NS = "boot";
}

AutoWiFi::AutoWiFi() : _state(State::NOT_CONNECTED), _otaState(OTAState::NOT_CONNECTED), _beacon(8080) {}

AutoWiFi::State AutoWiFi::connect() {
    checkForDeviceReset();

    Preferences preferences;
    preferences.begin(WIFI_NS, true);
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
        Serial.printf("[AutoWiFi] IP address: %s\n", getIP().toString().c_str());
    }

    if (_state == State::WIFI_CONNECTED) {
        _otaState = setupOTA();
    }

    return _state;
}

AutoWiFi::State AutoWiFi::connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    if (waitForWiFiConnection()) {
        Serial.println("[AutoWiFi] WiFi connected.");
        return State::WIFI_CONNECTED;
    }
    Serial.println("[AutoWiFi] Failed to connect to WiFi.");
    return State::NOT_CONNECTED;
}

AutoWiFi::State AutoWiFi::startAccessPoint() {
    Preferences preferences;
    preferences.begin(AP_NS, true);
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

    Serial.printf("[AutoWiFi] AP '%s' started.\n", ssid.c_str());

    _beacon.begin();
    _beacon.onMessage([](const Message& msg) -> String {
        if (msg.getProperty("restart") == "true") {
            xTaskCreatePinnedToCore(
                [](void*) {
                    delay(5000);
                    ESP.restart();
                },
                "RestartTask",
                1000,
                NULL,
                1,
                NULL,
                1
            );
            return "restarting in 5 seconds";
        }

        String ssid = msg.getProperty("ssid");
        String password = msg.getProperty("password");

        if (ssid.isEmpty() || password.isEmpty()) {
            return "Missing SSID or password";
        }

        setWiFiCredentials(ssid, password);

        return "Credentials saved.";
    });

    return State::AP_MODE;
}

void AutoWiFi::loop() {
    if (_state == State::AP_MODE) {
        _beacon.loop();
    } else if (_state == State::NOT_CONNECTED || WiFi.status() != WL_CONNECTED) {
        handleDisconnected();
    } else if (_otaState == OTAState::OTA_INITIALIZED) {
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
    preferences.begin(AP_NS, false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("Access point credentials set");
}

std::tuple<String, String> AutoWiFi::getAPCredentials() {
    Preferences prefs;
    prefs.begin(AP_NS, true);
    String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    prefs.end();
    
    return std::make_tuple(ssid, password);
}

void AutoWiFi::setOTACredentials(const String& hostName, const String& password) {
    Preferences preferences;
    preferences.begin(OTA_NS, false);
    preferences.putString("hostName", hostName);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("OTA credentials set");
}

std::tuple<String, String> AutoWiFi::getOTACredentials() {
    Preferences preferences;
    preferences.begin(OTA_NS, true);
    String hostName = preferences.getString("hostName", "");
    String password = preferences.getString("password", "");
    preferences.end();

    return std::make_tuple(hostName, password);
}

void AutoWiFi::setWiFiCredentials(const String &ssid, const String &password) {
    Preferences preferences;
    preferences.begin(WIFI_NS, false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("WiFi credentials set");
}

void AutoWiFi::handleDisconnected() {
    Serial.println("[AutoWiFi] No connection. Attempting to reconnect...");
    connect();
    if (_state == State::NOT_CONNECTED) {
        Serial.println("[AutoWiFi] Reconnection failed. Restarting in 10 seconds...");
        restartDevice(10000);
    }
}

void AutoWiFi::restartDevice(unsigned long delayMs) {
    delay(delayMs);
    ESP.restart();
}

bool AutoWiFi::waitForWiFiConnection() {
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void AutoWiFi::checkForDeviceReset() {
    Preferences bootPrefs;
    bootPrefs.begin(BOOT_NS, false);
    int bootCount = bootPrefs.getInt("boot_count", 0);
    bootCount += 1;
    bootPrefs.putInt("boot_count", bootCount);
    bootPrefs.end();

    Serial.printf("[AutoWiFi] current boot count: %d\n", bootCount);

    if (bootCount >= 4) {
        Serial.println("[AutoWiFi] Detected 4 fast reboots. Clearing WiFi credentials.");

        Preferences prefs;
        prefs.begin(WIFI_NS, false);
        bool wifiResult = prefs.clear();
        prefs.end();

        prefs.begin(BOOT_NS, false);
        bool bootResult = prefs.clear();
        prefs.end();

        Serial.printf("[AutoWiFi] WiFi Preferences clear result: %s.\n", wifiResult ? "success" : "failure");
        Serial.printf("[AutoWiFi] Boot Preferences clear result: %s.\n", bootResult ? "success" : "failure");
        Serial.println("Restarting...");
        ESP.restart();
    }

    xTaskCreatePinnedToCore(
        bootResetTask,
        "BootResetTask",
        2048,
        NULL,
        1,
        NULL,
        1
    );
}

void AutoWiFi::bootResetTask(void* parameter) {
    vTaskDelay(4000 / portTICK_PERIOD_MS);

    Preferences prefs;
    prefs.begin(BOOT_NS, false);
    prefs.putInt("boot_count", 0);
    prefs.end();

    Serial.println("[AutoWiFi] boot count reset to 0");

    vTaskDelete(NULL);
}

AutoWiFi::OTAState AutoWiFi::setupOTA() {
    auto [hostName, password] = getOTACredentials();

    if (hostName.isEmpty() || password.length() < 8) {
        Serial.println("[AutoWiFi] OTA credentials missing or password too short. Cannot setup OTA.");
        return OTAState::NOT_CONNECTED;
    }

    ArduinoOTA.setHostname(hostName.c_str());
    ArduinoOTA.setPassword(password.c_str());
    ArduinoOTA.begin();
    Serial.printf("[AutoWiFi] OTA initialized for hostname %s\n", hostName.c_str());
    return OTAState::OTA_INITIALIZED;
}
