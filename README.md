# ESP-IDF LCD Initialization (ST7789v2)
This project demonstrates the initialization of a 1.69-inch LCD display using the ESP-IDF and SPI bus.

## Requirements
- **ESP-IDF Version:** 5.2.1
- **ESP LVGL Port Version:** 1.4.0
- **LVGL Version:** 8.3.11

## Installation Instructions
1. Install the specified version of ESP-IDF.
2. Add the ESP LVGL Port as a dependency:
   - Option 1: Install it directly via the ESP-IDF dependency management.
   - Option 2: Manually download the LVGL Port and place it in your ESP-IDF installation directory. 
     For example, add it under:  
     `frameworks/esp-idf-v5.2.1/components/esp_lvgl_port`
	 
## Extra
- Make sure that vscode is setup and the paths in `c_cpp_properties.json` are set up correctly for intellisense.