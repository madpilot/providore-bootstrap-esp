#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "configuration.h"

#ifdef CONFIG_SECURED_SHARED_KEY
#include "esp32s2/rom/efuse.h"
#endif

// HMAC
#include "providore.h"

static const char *TAG = "APP";

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (providore_self_test_required())
    {
        providore_confirm_upgrade();
    }

    char device_id[33];
    size_t device_id_len;
    char psk[33];
    size_t psk_len;

    bzero(&device_id, 33);
    bzero(&psk, 33);

    ESP_ERROR_CHECK(get_device_id((char *)&device_id, &device_id_len));
    ESP_ERROR_CHECK(get_psk((char *)&psk, &psk_len));

    // Force truncate if the strings are too long
    device_id[33] = 0;
    psk[33] = 0;

    wifi_init_sta(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSKEY);
    time_t now = time(&now);
    time_t until = now + (15 * 60);

    if (now == -1)
    {
        ESP_LOGE(TAG, "Unable to generate current time");
        return;
    }
    if (until == -1)
    {
        ESP_LOGE(TAG, "Unable to generate expiry time");
        return;
    }

    providore_err_t upgrade_result = providore_firmware_upgrade(CONFIG_DEVICE_ID, CONFIG_SHARED_KEY);
    if (upgrade_result == PROVIDORE_OK)
    {
        ESP_LOGI(TAG, "Firmware upgraded!");
        ESP_LOGI(TAG, "Rebooting...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "Firmware upgrade failed.");
    }

    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
