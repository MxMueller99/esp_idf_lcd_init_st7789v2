/*
Author: Max Müller
Task: LCD Driver Initialization for a 1.69 inch display with driver ST7789V2
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include <string.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lvgl_port.h"

#define TAG "ST7789_INIT"
#define HSPI_HOST SPI2_HOST
#define LCD_HOST    HSPI_HOST

// Define pin connections
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 16
#define PIN_NUM_RST 17
#define PIN_NUM_BCKL 4

#define LCD_WIDTH 240
#define LCD_HEIGHT 280

spi_device_handle_t spi;

esp_err_t init_screen(esp_lcd_panel_handle_t *panel_handle) {

    // Initialize SPI bus 
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),   //changed from * 2 + 16 to * sizeof
    };
    esp_err_t ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    //ESP_ERROR_CHECK((HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Allocate an LCD IO device handle
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 18 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
     ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) LCD_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) LCD_HOST, &io_config, &io_handle));

    // Initialize the LCD controller driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        //.rgb_ele_order = ESP_LCD_COLOR_SPACE_RGB, // LCD_RGB_ELEMENT_ORDER_BGR or , or LCD_RGB_ELEMENT_ORDER_RGB
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install LCD controller driver: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Initialize GPIO pins for control signals
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_BCKL, 1);

    // Initialize LCD Panel
    esp_lcd_panel_reset(*panel_handle);
    esp_lcd_panel_init(*panel_handle);
    
    // Debugging Color and gap error
    esp_lcd_panel_invert_color(*panel_handle, true);
    esp_lcd_panel_set_gap(*panel_handle, 0, 20); //Really important!

    //esp_lcd_panel_mirror(*panel_handle, true, false);  // Example for flipping/mirroring if needed
    //esp_lcd_panel_swap_xy(*panel_handle, true);        // Swap X and Y axes if needed for rotation
    //ESP_GOTO_ON_ERROR(esp_lcd_new_rgb_panel(&panel_config, &panel_handle), err, TAG, "RGB init failed"); // needed PERHAPS
    esp_lcd_panel_disp_on_off(*panel_handle, true);
    
    ESP_LOGI(TAG, "Initialization complete");

    return ESP_OK;
}

// Used to test the LCD screen
void fill_screen_with_color(esp_lcd_panel_handle_t panel_handle, uint16_t color) {
    // Create a buffer to hold the color data
    size_t buffer_size = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t);
    uint16_t *color_buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    if (color_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for color buffer");
        return;
    }

    // Fill Buffer with color
    for (int i=0; i < (LCD_WIDTH*LCD_HEIGHT); i++) {
        color_buffer[i] = color;
    }

    // Draw the buffer on the screen
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, color_buffer);

    // Free buffer
    heap_caps_free(color_buffer);
}

// Used to test the LCD screen
void color_switch_task(void *arg) {

    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) arg;

    while (1) {
        // Fill screen red
        fill_screen_with_color(panel_handle, 0xF800);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen white
        fill_screen_with_color(panel_handle, 0xFFFF);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen black
        fill_screen_with_color(panel_handle, 0x0000);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen random color
        fill_screen_with_color(panel_handle, 0xAFB0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen random color
        fill_screen_with_color(panel_handle, 0xFBFB);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen white
        fill_screen_with_color(panel_handle, 0xFFFF);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Main task for the application
void app_main() {

    esp_lcd_panel_handle_t panel_handle = NULL;

    // Initialization
    ESP_LOGI(TAG, "Starting Initialization");
    esp_err_t ret = init_screen(&panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Screen initialization failed");
        return;
    }

    // Fill Screen with color
    xTaskCreate(color_switch_task, "color_switch_task", 4096, (void *) panel_handle, 5, NULL);
}
