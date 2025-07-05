# AutoWiFi

AutoWiFi is an ESP32 utility that simplifies network connectivity by automatically connecting to saved WiFi credentials, falling back to Access Point (AP) mode if none are available, and enabling OTA updates.

## Features

- Automatically connects to stored WiFi credentials
- Falls back to Access Point mode for user provisioning if credentials are missing
- Persists credentials using the built-in Preferences API
- Allows over-the-air (OTA) firmware updates using ArduinoOTA
- Detects multiple fast reboots and clears stored WiFi credentials for recovery
- Includes a REST-based provisioning beacon while in AP mode

## Getting Started

### 1. One-Time Credential Provisioning Sketch

To get started, flash a temporary sketch that sets your WiFi and OTA credentials using the provided methods. **Be sure not to commit this sketch to source control to avoid leaking credentials.**

```cpp
#include <AutoWiFi.h>

AutoWiFi wifi;

void setup() {
    Serial.begin(9600);

    // Set initial credentials
    wifi.setAccessPointCredentials("MyDeviceAP", "ap_password");
    wifi.setOTACredentials("esp32-device", "ota_secure_pw");

    Serial.println("Credentials written. You may now flash your main sketch.");
}

void loop() {}
```
