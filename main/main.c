#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"

#ifdef CONFIG_SECURED_SHARED_KEY
#include "esp32s2/rom/efuse.h"
#endif

// HMAC
#include "providore.h"

static const char *TAG = "HTTP_CLIENT";

void app_main(void)
{
#ifdef CONFIG_SECURED_SHARED_KEY
    // Write the shared key to EFUSE
    ets_status_t ets_status = ets_efuse_write_key(ETS_EFUSE_BLOCK_KEY4,
                                                  ETS_EFUSE_KEY_PURPOSE_HMAC_UP,
                                                  CONFIG_SHARED_KEY, strlen(CONFIG_SHARED_KEY));

    if (ets_status == ESP_OK)
    {
        ESP_LOGI(TAG, "Key written!");
    }
    else
    {
        // writing key failed, maybe written already
        ESP_LOGI(TAG, "Key not written: %i", ets_status);
    }
#endif

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (providore_self_test_required())
    {
        providore_confirm_upgrade();
    }

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

    providore_err_t upgrade_result = providore_firmware_upgrade(CONFIG_DEVICE_ID);
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
