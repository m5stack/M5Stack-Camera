#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "string.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_camera.h"

#include "config.h"
/* Static var --------------------------------------------------------------- */
static const char* TAG = "camera";
static const int RX_BUF_SIZE = 1024;

/* Static fun --------------------------------------------------------------- */
static void uart_init();
extern void led_brightness(int duty);

static camera_config_t camera_config = {
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz
    .xclk_freq_hz = CAM_XCLK_FREQ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_HQVGA,//QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 3, //0-63 lower number means higher quality
    .fb_count = 3 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

void app_main()
{
    esp_log_level_set("wifi", ESP_LOG_INFO);
    
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }
    
    uart_init();

    err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
         for(;;) {
            vTaskDelay(10);
        }
    
    } else{
        led_brightness(20);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    char tx_buffer[200] = { '\0' };
    uint32_t data_len = 0;
    
    tx_buffer[0] = 0xFF;
    tx_buffer[1] = 0xD8;
    tx_buffer[2] = 0xEA;
    tx_buffer[3] = 0x01;

#ifdef FISH_EYE_CAM
    // flip img, other cam setting view sensor.h
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
	s->set_hmirror(s, 1);
#endif
  
    while(true){
        camera_fb_t * fb = esp_camera_fb_get();
       
        tx_buffer[4] = (uint8_t)((fb->len & 0xFF0000) >> 16) ;
        tx_buffer[5] = (uint8_t)((fb->len & 0x00FF00) >> 8 ) ;
        tx_buffer[6] = (uint8_t)((fb->len & 0x0000FF) >> 0 );

        
        uart_write_bytes(UART_NUM_1, (char *)tx_buffer, 7);
        uart_write_bytes(UART_NUM_1, (char *)fb->buf, fb->len);
        //uart_write_bytes(UART_NUM_1, (char *)buf, buf_len);
        data_len =(uint32_t)(tx_buffer[4] << 16) | (tx_buffer[5] << 8) | tx_buffer[6];
        printf("should %d, print a image, len: %d\r\n",fb->len, data_len);

        esp_camera_fb_return(fb);
    }
}


static void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);

    uart_set_pin(UART_NUM_1, CAM_PIN_TX, CAM_PIN_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}
