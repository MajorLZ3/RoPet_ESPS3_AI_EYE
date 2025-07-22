/*
 * Board configuration – LingXi Breadboard (Wi-Fi + ILI9341 LCD)
 * 2025-05-XX
 */
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>
#include <esp_lcd_panel_types.h>  // for LCD_RGB_ELEMENT_ORDER_*

/*────────────────────────  Audio  ────────────────────────*/
#define AUDIO_INPUT_SAMPLE_RATE   24000
#define AUDIO_OUTPUT_SAMPLE_RATE  24000

/* ES8311 I²S */
#define AUDIO_I2S_GPIO_MCLK   GPIO_NUM_21
#define AUDIO_I2S_GPIO_WS     GPIO_NUM_11
#define AUDIO_I2S_GPIO_BCLK   GPIO_NUM_13
#define AUDIO_I2S_GPIO_DIN    GPIO_NUM_12   // from mic to ESP32
#define AUDIO_I2S_GPIO_DOUT   GPIO_NUM_10   // from ESP32 to codec
#define AUDIO_CODEC_PA_PIN    GPIO_NUM_46   // power-amp enable (high=on)

/* ES8311 I²C */
#define AUDIO_CODEC_I2C_SDA_PIN   GPIO_NUM_14
#define AUDIO_CODEC_I2C_SCL_PIN   GPIO_NUM_45

#define AUDIO_CODEC_ES8311_ADDR   ES8311_CODEC_DEFAULT_ADDR
#define AUDIO_CODEC_ES7210_ADDR   0x82                // keep for future
/* If you really use PCA9557 as GPIO expander, keep this macro */
#define AUDIO_CODEC_USE_PCA9557

/*────────────────────────  Buttons  ──────────────────────*/
#define BOOT_BUTTON_GPIO          GPIO_NUM_9           // active-low

/*────────────────────────  ILI9341 LCD  ──────────────────*/
/* 分辨率（竖屏） */
#define DISPLAY_WIDTH             240
#define DISPLAY_HEIGHT            320      // ILI9341 全高，应用可裁 240
/* 画面方向（根据实际排线选择） */
#define DISPLAY_SWAP_XY           false
#define DISPLAY_MIRROR_X          false
#define DISPLAY_MIRROR_Y          true     // 常见竖屏需要翻转 Y
/* 颜色设置 */
#define DISPLAY_RGB_ORDER         LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_INVERT_COLOR      false
/* 可视窗口偏移 */
#define DISPLAY_OFFSET_X          0
#define DISPLAY_OFFSET_Y          0
/* SPI3 引脚 */
#define DISPLAY_MOSI_PIN          GPIO_NUM_47
#define DISPLAY_CLK_PIN           GPIO_NUM_21
#define DISPLAY_CS_PIN            GPIO_NUM_41
#define DISPLAY_DC_PIN            GPIO_NUM_40
#define DISPLAY_RST_PIN           GPIO_NUM_45
/* 背光（高亮） */
#define DISPLAY_BACKLIGHT_PIN         GPIO_NUM_42
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
/* SPI 时钟 ≤ 40 MHz */
#define DISPLAY_SPI_SCLK_HZ       (40 * 1000 * 1000)

/*────────────────────────  Reserved  ────────────────────*/
/* 旧 GC9A01 双屏宏全部移除；如后续需要可放到单独板型配置 */

/*────────────────────────────────────────────────────────*/
#endif // _BOARD_CONFIG_H_
