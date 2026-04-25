#include "NetworkService.h"
#include "config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <cstdio>

static const char* TAG = "NETWORK";

// Біти для FreeRTOS Event Group
#define WIFI_CONNECTED_BIT BIT0
#define MQTT_CONNECTED_BIT BIT1

static EventGroupHandle_t s_network_event_group;
static esp_mqtt_client_handle_t mqtt_client = nullptr;

// --- Обробник подій WiFi ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Got IP address");
        xEventGroupSetBits(s_network_event_group, WIFI_CONNECTED_BIT);
    }
}

// --- Обробник подій MQTT ---
static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    if (event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Connected to ThingsBoard");
        xEventGroupSetBits(s_network_event_group, MQTT_CONNECTED_BIT);
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        ESP_LOGW(TAG, "MQTT Disconnected");
    }
}

// --- Імплементація класу ---
bool NetworkService::Connect() {
    s_network_event_group = xEventGroupCreate();

    // 1. Ініціалізація NVS (необхідно для збереження калібрувань WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Ініціалізація WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {};
    snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", WIFI_SSID);
    snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Чекаємо підключення до WiFi (максимум 10 секунд)
    EventBits_t bits = xEventGroupWaitBits(s_network_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));
    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
        return false;
    }

    // 3. Ініціалізація MQTT для ThingsBoard
    // ThingsBoard використовує token у полі username, без пароля
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = MQTT_BROKER_URI;
    mqtt_cfg.credentials.username = MQTT_TOKEN;
    mqtt_cfg.credentials.authentication.password = NULL; // ThingsBoard не потребує пароля

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // Чекаємо підключення до MQTT (максимум 10 секунд)
    bits = xEventGroupWaitBits(s_network_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));
    if (!(bits & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to connect to MQTT Broker");
        return false;
    }

    return true;
}

bool NetworkService::PublishBuffer(SensorData* buffer, int count) {
    if (!mqtt_client) return false;

    char payload[128];
    // Оскільки ми відправляємо по 1 запису (count завжди 1), робимо простий JSON
    snprintf(payload, sizeof(payload), "{\"temp1\":%.2f, \"temp2\":%.2f}", buffer[0].temp1, buffer[0].temp2);

    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, payload, 0, 1, 0);
    
    if (msg_id == -1) {
        ESP_LOGE(TAG, "Failed to publish");
        return false;
    } else {
        ESP_LOGI(TAG, "Published: %s", payload);
        return true;
    }
}

void NetworkService::Disconnect() {
    if (mqtt_client) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = nullptr;
    }
    esp_wifi_stop();
    esp_wifi_deinit();
    if (s_network_event_group) {
        vEventGroupDelete(s_network_event_group);
    }
    ESP_LOGI(TAG, "Network disconnected. Ready for sleep.");
}
