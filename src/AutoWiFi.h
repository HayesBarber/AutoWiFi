#ifndef AUTOWIFI_H
#define AUTOWIFI_H

#include <WiFi.h>

class AutoWiFi {
public:
    AutoWiFi(const char* apSSID = "AutoWiFi");
    bool connect();
    void update();
    bool isConnected() const;
    IPAddress getIP() const;

private:
    bool connectToWiFi(const String& ssid, const String& password);
    bool startAccessPoint();

    const char* _apSSID;
    bool _startupSuccess = false;
};

#endif
