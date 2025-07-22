/*
 * compact_wifi_board_lcd_lingxi.cc
 * —— “灵犀·面包板” (ESP32-S3 + ES8311 + ILI9341) 基础板定义
 *
 * 仅完成硬件初始化：I²C / Audio Codec / 单 ILI9341 SPI 屏。
 * UI、魔眼动画由应用层（LVGL 或自绘）实现。
 */

#include "wifi_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "iot/thing_manager.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_ili9341.h>
#include "esp_lvgl_port.h"

#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_log.h>

/*──────────────────────────── 宏 & 字体 ───────────────────────────*/
#define TAG "LingXiBoard"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);

/*─────────────────────── LingXiBoard 类定义 ───────────────────────*/
class LingXiBoard : public WifiBoard {
private:
    /* Hard-I²C (ES8311) */
    i2c_master_bus_handle_t codec_i2c_bus_{};

    /* LCD 相关句柄 */
    esp_lcd_panel_io_handle_t lcd_io_{};
    esp_lcd_panel_handle_t    lcd_panel_{};

    /* 周边对象 */
    Button      boot_button_{BOOT_BUTTON_GPIO};
    LcdDisplay* display_{};

    /*───────────────── 初始化分块 ─────────────────*/

    /* 1. I²C 给 ES8311 用 */
    void InitI2C()
    {
        i2c_master_bus_config_t cfg = {
            .i2c_port            = I2C_NUM_0,
            .sda_io_num          = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num          = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source          = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt   = 7,
            .intr_priority       = 0,
            .trans_queue_depth   = 0,
            .flags = { .enable_internal_pullup = 1 },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &codec_i2c_bus_));
    }

    /* 2. SPI3 Host 用于 ILI9341 */
    void InitSpiBus()
    {
        spi_bus_config_t buscfg = {
            .mosi_io_num = DISPLAY_MOSI_PIN,
            .miso_io_num = GPIO_NUM_NC,        // ILI9341 写屏即可
            .sclk_io_num = DISPLAY_CLK_PIN,
            .quadwp_io_num = GPIO_NUM_NC,
            .quadhd_io_num = GPIO_NUM_NC,
            .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2,
        };
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    /* 3. ILI9341 Panel 驱动 */
    void InitILI9341()
    {
        ESP_LOGI(TAG, "Init ILI9341 display");

        /* 3.1 IO handle */
        esp_lcd_panel_io_spi_config_t io_cfg = {
            .cs_gpio_num     = DISPLAY_CS_PIN,
            .dc_gpio_num     = DISPLAY_DC_PIN,
            .spi_mode        = 0,
            .pclk_hz         = DISPLAY_SPI_SCLK_HZ,  // 例如 40 MHz
            .trans_queue_depth = 10,
            .lcd_cmd_bits    = 8,
            .lcd_param_bits  = 8,
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
            (esp_lcd_spi_bus_handle_t)SPI3_HOST, &io_cfg, &lcd_io_));

        /* 3.2 panel handle */
        esp_lcd_panel_dev_config_t pan_cfg = {
            .reset_gpio_num  = DISPLAY_RST_PIN,
            .color_space     = ESP_LCD_COLOR_SPACE_RGB,
            .bits_per_pixel  = 16,
        };
        pan_cfg.rgb_endian = DISPLAY_RGB_ORDER;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(lcd_io_, &pan_cfg, &lcd_panel_));

        /* 3.3 init sequence */
        esp_lcd_panel_reset(lcd_panel_);
        esp_lcd_panel_init(lcd_panel_);
        if (DISPLAY_INVERT_COLOR) esp_lcd_panel_invert_color(lcd_panel_, true);
        esp_lcd_panel_disp_on_off(lcd_panel_, true);
    }

    /* 4. 背光 */
    void InitBacklight()
    {
        if (DISPLAY_BACKLIGHT_PIN == GPIO_NUM_NC) return;

        gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << DISPLAY_BACKLIGHT_PIN,
            .mode         = GPIO_MODE_OUTPUT,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
        };
        gpio_config(&cfg);
        gpio_set_level(DISPLAY_BACKLIGHT_PIN,
                       DISPLAY_BACKLIGHT_OUTPUT_INVERT ? 0 : 1);
    }

    /* 5. 按钮 */
    void InitButtons()
    {
        boot_button_.OnClick([this] {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    /* 6. IOT Thing */
    void InitIot()
    {
        auto& mgr = iot::ThingManager::GetInstance();
        mgr.AddThing(iot::CreateThing("Speaker"));
        mgr.AddThing(iot::CreateThing("Screen"));   // 屏幕魔眼
    }

    /* 7. Display 对象封装 （给应用层使用） */
    void CreateDisplayObject()
    {
        display_ = new SpiLcdDisplay(
            lcd_io_, lcd_panel_,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
            {
                .text_font  = &font_puhui_20_4,
                .icon_font  = &font_awesome_20_4,
                .emoji_font = font_emoji_64_init(),
            });
    }

public:
    LingXiBoard()
    {
        /* PA 引脚下拉，防开机 POP */
        gpio_set_direction(AUDIO_CODEC_PA_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(AUDIO_CODEC_PA_PIN, 0);

        InitI2C();
        InitSpiBus();
        InitILI9341();
        InitBacklight();
        InitButtons();
        InitIot();
        CreateDisplayObject();
    }

    /*========== Board 接口实现 ==========*/
    virtual AudioCodec* GetAudioCodec() override
    {
        static Es8311AudioCodec codec(
            codec_i2c_bus_, I2C_NUM_0,
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK,  AUDIO_I2S_GPIO_BCLK,
            AUDIO_I2S_GPIO_WS,    AUDIO_I2S_GPIO_DOUT,
            AUDIO_I2S_GPIO_DIN,   AUDIO_CODEC_PA_PIN,
            AUDIO_CODEC_ES8311_ADDR);
        return &codec;
    }

    virtual Display* GetDisplay() override
    {
        return display_;
    }
};

/* 向框架注册此板 */
DECLARE_BOARD(LingXiBoard);
