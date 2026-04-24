#pragma once

// --- Налаштування WiFi ---
#define WIFI_SSID "ВАШ_WIFI_SSID"
#define WIFI_PASS "ВАШ_WIFI_ПАРОЛЬ"

// --- Налаштування HiveMQ Cloud ---
// Обов'язково використовуйте префікс mqtts:// для TLS і порт 8883
#define MQTT_BROKER_URI "mqtts://ВАШ-КЛАСТЕР.s1.eu.hivemq.cloud:8883"
#define MQTT_USERNAME "ВАШ_MQTT_ЛОГІН"
#define MQTT_PASSWORD "ВАШ_MQTT_ПАРОЛЬ"

#define MQTT_TOPIC "meteo/sensors"
