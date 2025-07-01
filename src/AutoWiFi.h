#ifndef AUTOWIFI_H
#define AUTOWIFI_H

#include <WiFi.h>

class AutoWiFi {
public:
    AutoWiFi();
    void connect();
    void update();
    bool isConnected() const;
    IPAddress getIP() const;
};

#endif
