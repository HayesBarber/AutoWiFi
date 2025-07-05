#ifndef AUTOWIFI_H
#define AUTOWIFI_H

#include <WiFi.h>
#include <RestBeacon.h>

class AutoWiFi {
public:
    enum class State {
        WIFI_CONNECTED,
        AP_MODE,
        NOT_CONNECTED
    };

    AutoWiFi();
    State connect();
    void loop();
    IPAddress getIP() const;
    State getState() const;
    void setAccessPointCredentials(const String& ssid, const String& password);
    void setOTACredentials(const String& hostName, const String& password);

private:
    State connectToWiFi(const String& ssid, const String& password);
    State startAccessPoint();
    void checkForDeviceReset();
    static void bootResetTask(void* parameter);
    void setupOTA();

    State _state;
    RestBeacon _beacon;
};

#endif
