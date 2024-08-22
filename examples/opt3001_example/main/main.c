#include <stdio.h>

#include "esp_check.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "opt3001.h"

#define I2C_SCL GPIO_NUM_5
#define I2C_SDA GPIO_NUM_4
#define I2C_PORT I2C_NUM_0
#define OPT3001_ADDR 0x45

static const char TAG[] = "OPT3001_sample";

i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
i2c_master_bus_handle_t i2c_bus_handle;

i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = OPT3001_ADDR,
    .scl_speed_hz = 100000,
    .flags.disable_ack_check = true,
};
i2c_master_dev_handle_t opt3001_dev_handle;

void app_main(void)
{
    while(1) {
        // Initialize bus and add device
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));
        ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &opt3001_dev_handle));

        // Configure opt3001 settings
        opt3001_settings_t opt3001_settings;
        opt3001_settings.device_handle = opt3001_dev_handle;
        opt3001_settings.configuration = OPT3001_CONFIG_AUTO_FULLSCALE_RANGE_MASK | OPT3001_CONFIG_CONVERSION_TIME_100MS_MASK | 
                                        OPT3001_CONFIG_CONV_MODE_SINGLESHOT_MASK | OPT3001_CONFIG_LATCH_LATCHED_MASK;
        ESP_ERROR_CHECK(OPT3001_Write_Configuration(&opt3001_settings));
        
        // Get lux from OPT3001
        float lux;
        if (OPT3001_Read_Lux(&opt3001_settings, &lux) == ESP_OK) {
            ESP_LOGI(TAG, "Lux: %.3f", lux);
        }

        // Deinitialize I2C bus (must be done for software resets)
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(opt3001_dev_handle));  // Should be redundant if deleting the bus
        ESP_ERROR_CHECK(i2c_del_master_bus(i2c_bus_handle));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}