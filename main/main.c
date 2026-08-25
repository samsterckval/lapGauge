#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st77916.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "esp_log.h"
#include "pins.h"
#include "tca9554.h"
#include "imu/imu_ui.h"
#include "debug_ui.h"

static const char *TAG = "main";

static i2c_master_bus_handle_t s_i2c_bus;

static void init_i2c_and_expander(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_i2c_bus));
    ESP_ERROR_CHECK(tca9554_init(s_i2c_bus));
}

static void reset_lcd_via_expander(void)
{
    // ST77916 reset is active-low, held behind the expander (EXIO2).
    tca9554_set_level(EXIO_LCD_RST, false);
    vTaskDelay(pdMS_TO_TICKS(20));
    tca9554_set_level(EXIO_LCD_RST, true);
    vTaskDelay(pdMS_TO_TICKS(120)); // panel needs time to come out of reset
}

static esp_lcd_panel_handle_t init_display(esp_lcd_panel_io_handle_t *out_io_handle)
{
    reset_lcd_via_expander();

    // Backlight - plain push-pull GPIO, PWM later if you want dimming.
    gpio_config_t bl_cfg = {
        .pin_bit_mask = 1ULL << PIN_LCD_BL,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&bl_cfg);
    gpio_set_level(PIN_LCD_BL, 1);

    // Partial-buffer transfer size (matches the LVGL buffer below, not the
    // whole framebuffer - keeps the DMA descriptor list sane).
    const size_t max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t);

    spi_bus_config_t buscfg = ST77916_PANEL_BUS_QSPI_CONFIG(
        PIN_LCD_SCK, PIN_LCD_SDA0, PIN_LCD_SDA1, PIN_LCD_SDA2, PIN_LCD_SDA3,
        max_transfer_sz);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = ST77916_PANEL_IO_QSPI_CONFIG(
        PIN_LCD_CS, NULL, NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &io_handle));

    // RST is behind the expander, not a native GPIO, so we hand-toggled it
    // above; tell the panel driver -1 so it doesn't try to drive a GPIO too.
    // The QSPI mode flag has to be set explicitly via vendor_config.
    static st77916_vendor_config_t vendor_config = {
        .flags = { .use_qspi_interface = 1 },
    };
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = -1,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };
    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st77916(io_handle, &panel_cfg, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    *out_io_handle = io_handle;
    return panel_handle;
}

static void init_lvgl_and_ui(esp_lcd_panel_handle_t panel_handle, esp_lcd_panel_io_handle_t io_handle)
{
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * 80, // matches max_transfer_sz set at SPI bus init
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
    };
    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);

    debug_ui_start(disp);

    lvgl_port_lock(0);
    lv_obj_t *screen = lv_display_get_screen_active(disp);
    lvgl_port_unlock();

    if (!imu_ui_start(s_i2c_bus, screen)) {
        ESP_LOGE(TAG, "IMU init failed - no ball display");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "bringing up I2C + expander");
    init_i2c_and_expander();

    ESP_LOGI(TAG, "bringing up QSPI display");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_handle_t panel = init_display(&io_handle);

    ESP_LOGI(TAG, "starting LVGL");
    init_lvgl_and_ui(panel, io_handle);
}
