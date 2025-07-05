#!/bin/bash

read -p "Enter WiFi SSID: " ssid
read -s -p "Enter WiFi Password: " password
echo

esp_ip="192.168.4.1"

json_payload=$(jq -n --arg ssid "$ssid" --arg password "$password" '{ssid: $ssid, password: $password}')

response=$(curl -s -X POST "http://$esp_ip/message" \
    -H "Content-Type: application/json" \
    -d "$json_payload")

echo "Response from ESP: $response"

restart_payload=$(jq -n '{restart: "true"}')

restart_response=$(curl -s -X POST "http://$esp_ip/message" \
    -H "Content-Type: application/json" \
    -d "$restart_payload")

echo "Restart response from ESP: $restart_response"
