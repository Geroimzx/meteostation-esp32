#pragma once

#include "driver/gpio.h"

// --- Налаштування WiFi ---
#define WIFI_SSID "LimNet_8A10"
#define WIFI_PASS "87654321qq"

// --- Налаштування ThingsBoard Cloud ---
// ThingsBoard не потребує пароля - використовуємо тільки Authentication Token
#define MQTT_BROKER_URI "mqtt://thingsboard.cloud"
#define MQTT_TOKEN "NlZrFF9T7AIy5lFK7C97"
#define MQTT_TOPIC "v1/devices/me/telemetry"

// --- Налаштування GPIO ---
#define STATUS_LED_GPIO GPIO_NUM_2
#define NTC_POWER_PIN GPIO_NUM_4
#define SLEEP_TIME_MIN (5)
