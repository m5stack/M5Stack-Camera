/* LEDC (LED Controller) fade example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "config.h"

void led_brightness(int duty) {

    #define LEDC_HS_TIMER          LEDC_TIMER_2
    #define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
    #define LEDC_HS_CH0_GPIO       (CAMERA_LED_GPIO)
    #define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_5    
    #define LEDC_TEST_DUTY         (10)

    duty = 1024 - duty;

    ledc_timer_config_t ledc_timer = {
        .bit_num = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 5000,              // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,   // timer mode
        .timer_num = LEDC_HS_TIMER    // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = 
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 900,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .timer_sel  = LEDC_HS_TIMER
    };
    ledc_channel_config(&ledc_channel);

    // Initialize fade service.
    // ledc_fade_func_install(0);

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}