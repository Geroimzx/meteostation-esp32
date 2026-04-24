idf_component_register(
    SRCS 
        "main.cpp"
        "infrastructure/AdcService.cpp"
        "infrastructure/NetworkService.cpp"
    INCLUDE_DIRS 
        "." 
        "domain" 
        "infrastructure"
    REQUIRES               
        "nvs_flash" 
        "esp_wifi" 
        "mqtt" 
        "mbedtls" 
        "esp_netif"
        "driver"          # Для driver/gpio.h
        "esp_adc"         # Для esp_adc/adc_oneshot.h
        "esp_hw_support"  # Для esp_sleep.h
)

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)