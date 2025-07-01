#ifndef AUTOWIFI_H
#define AUTOWIFI_H

#include <WiFi.h>

class AutoWiFi {
public:
    AutoWiFi(const char* apSSID = "AutoWiFi");
    void connect();
    void update();
    bool isConnected() const;
    IPAddress getIP() const;

private:
    void connectToWiFi(const String& ssid, const String& password);
    void startAccessPoint();

    const char* _apSSID;
};

#endif
