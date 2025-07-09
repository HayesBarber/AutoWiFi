#ifndef AUTO_WIFI_H
#define AUTO_WIFI_H

#include <WiFi.h>
#include <RestBeacon.h>
#include <tuple>

class AutoWiFi {
public:
    enum class State {
        WIFI_CONNECTED,
        AP_MODE,
        NOT_CONNECTED
    };

    enum class OTAState {
        OTA_INITIALIZED,
        NOT_CONNECTED
    };

    AutoWiFi();

    /**
     * Attempts to connect to a saved WiFi network or starts an access point if credentials are missing.
     * Also sets up OTA if WiFi connection is successful.
     *
     * @return Current connection state after attempting to connect.
     */
    State connect();

    /**
     * Must be called periodically. Handles OTA updates, reconnection attempts, and access point logic.
     */
    void loop();

    /**
     * Returns the current IP address depending on the connection state.
     *
     * @return IPAddress representing the device's IP.
     */
    IPAddress getIP() const;

    /**
     * Returns the current connection state.
     *
     * @return Current state as a value of AutoWiFi::State.
     */
    State getState() const;

    /**
     * Stores access point credentials in persistent storage.
     *
     * @param ssid SSID for the access point.
     * @param password Password for the access point (must be at least 8 characters).
     */
    static void setAccessPointCredentials(const String& ssid, const String& password);

    /**
     * Stores OTA credentials in persistent storage.
     *
     * @param hostName Hostname to use for OTA.
     * @param password Password for OTA authentication (must be at least 8 characters).
     */
    static void setOTACredentials(const String& hostName, const String& password);

    /**
     * Stores WiFi credentials in persistent storage.
     *
     * @param ssid SSID for the access point.
     * @param password Password for WiFi (must be at least 8 characters).
     */
    static void setWiFiCredentials(const String& ssid, const String& password);

private:
    State connectToWiFi(const String& ssid, const String& password);
    State startAccessPoint();
    void checkForDeviceReset();
    static void bootResetTask(void* parameter);
    OTAState setupOTA();
    std::tuple<String, String> getOTACredentials();
    std::tuple<String, String> getAPCredentials();
    std::tuple<String, String> getWiFiCredentials();
    void handleDisconnected();
    void restartDevice(unsigned long delayMs);
    bool waitForWiFiConnection();
    static void setCredentials(const char* ns, const char* key1, const String& val1, const char* key2, const String& val2);
    static std::tuple<String, String> getCredentials(const char* ns, const String& key1, const String& key2);

    State _state;
    OTAState _otaState;
    RestBeacon _beacon;
};

#endif
