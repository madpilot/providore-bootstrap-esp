# Providore ESP bootstrap component

See [https://github.com/madpilot/providore](https://github.com/madpilot/providore) for the main project

## Configuration

Providore uses the built in non-volatile storage (NVS) to store the device id (and the shared private key if you aren't using secure storage). It is recommended to use an encrypted NVS partition to ensure the device WIFI passphrase and shared private key are kept private. There are two ways to do this:

1. Create a small helper firmware that writes the the required keys to NVS. This can be useful for one-off firmware, where you don't mind compiling for each device.

2. Build an NVS offline, and upload to the device. This is useful if you are flashing the bootstrap firmware to multiple devices, and you don't want to re-compile each time.

### Option 1: Helper firmware example

```c
const char *device_id = "Your Device ID"; 
const char *psk = "Your shared private key";

esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
{
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);
nvs_handle_t handle;
ESP_ERROR_CHECK(nvs_open("providore", NVS_READWRITE, &handle));
ESP_ERROR_CHECK(nvs_set_str(handle, "device_id", device_id));

#ifdef CONFIG_SECURED_SHARED_KEY
  ESP_ERROR_CHECK(ets_efuse_write_key(ETS_EFUSE_BLOCK_KEY4, ETS_EFUSE_KEY_PURPOSE_HMAC_UP psk, strlen(psk)));
#else
    ESP_ERROR_CHECK(nvs_set_str(handle, "psk", psk));
#endif
```

### Option 2: Creating a CSV file with the details

You can write these details to NVS without having to re-compile the bootstrap firmware by [generating a initial NVS filesystem offline](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_partition_gen.html) and uploading using the idf.py utility.

```csv
key,type,encoding,value
providore,namespace,,
device_id,data,string,your_device_id
psk,data,string,your_psk
```

### Generating the NVS partition

Ensure you have [setup your IDF environment variables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#step-4-set-up-the-environment-variables)

```bash
python ${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py encrypt partition.csv nvs.bin 0x3000 --keygen --keyfile keys.bin
```

### Uploading the NVS partition

