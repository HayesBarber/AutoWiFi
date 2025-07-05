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
    wifi.setWiFiCredentials("your_ssid", "your_ssid_password");

    Serial.println("Credentials written. You may now flash your main sketch.");
}

void loop() {}
```

### 2. Provisioning via Access Point + Script

If your device starts in Access Point mode, you can use the included `provision.sh` script to send WiFi credentials to it over HTTP.

This requires:

- The ESP32 to be running AutoWiFi in AP mode
- Your computer to connect to the ESP32's access point (e.g., `MyDeviceAP`)

Run the script as follows:

```bash
bash provision.sh
```

The script will prompt for the SSID and password and send them to the ESP32 at `192.168.4.1`.

Although the ESP32’s access point uses WPA2 encryption, credentials are still sent over plain HTTP. This means they could be intercepted by someone else connected to the AP. While safe for most personal use, consider serial provisioning.
