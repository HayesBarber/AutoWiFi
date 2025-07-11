# AutoWiFi

AutoWiFi is an ESP32 utility that simplifies network connectivity by automatically connecting to saved WiFi credentials, falling back to Access Point (AP) mode if none are available, and enabling OTA updates.

## Features

- Automatically connects to stored WiFi credentials
- Falls back to Access Point mode for provisioning if credentials are missing
- Provides a REST-based provisioning interface while in AP mode
- Detects multiple fast reboots and clears stored WiFi credentials for re-provisioning
- Allows over-the-air (OTA) firmware updates using ArduinoOTA

## One-Time Credential Provisioning Sketch

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

## Provisioning via Access Point + Script

If your device starts in Access Point mode, you can use the included `provision.sh` script to send WiFi credentials to it over HTTP.

This requires:

- The ESP32 to be running AutoWiFi in AP mode
- Your computer to connect to the ESP32's access point (e.g., `MyDeviceAP`)

Run the script as follows:

```bash
bash provision.sh
```

The script will prompt for the SSID and password and send them to the ESP32 at `192.168.4.1`.

Although the ESP32’s access point is password protected and uses WPA2 encryption, credentials are still sent over plain HTTP. This means they could be intercepted by someone else who manages to also connect to the AP. While safe for most personal use, consider serial provisioning.

## Normal Usage

Once credentials have been provisioned (via a one-time sketch or the provisioning script), `AutoWiFi` can be used in your main application to manage network connectivity and OTA updates automatically.

Simply create an instance of `AutoWiFi` and call the following in your sketch:

- `connect()` once in `setup()` to attempt connection or fall back to AP mode
- `loop()` inside your `loop()` function to handle OTA and reconnection logic

Example:

```cpp
AutoWiFi wifi;

void setup() {
    Serial.begin(9600);
    wifi.connect();
}

void loop() {
    wifi.loop();
}
```

## Re-provisioning

If your WiFi credentials change and you want the device to return to access point mode, simply power cycle the device **4 times** with less than 4 seconds between each boot.

This will wipe any persisted WiFi credentials and automatically restart the device in access point mode, allowing it to be re-provisioned.
