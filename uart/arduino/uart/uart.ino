#include <Arduino.h>
#include "driver/uart.h"

#include "M5Stack.h"

typedef enum {
  WAIT_FRAME_1 = 0x00,
  WAIT_FRAME_2,
  WAIT_FRAME_3,
  GET_NUM,
  GET_LENGTH,
  GET_MSG,
  RECV_FINISH,
  RECV_ERROR,
} uart_rev_state_t;

typedef struct {
  uint8_t idle;
  uint32_t length;
  uint8_t *buf;
} jpeg_data_t;

/* Define ------------------------------------------------------------ */


/* Global Var ----------------------------------------------------------- */
uart_rev_state_t uart_rev_state;
static const int RX_BUF_SIZE = 1024*40;
static const uint8_t frame_data_begin[3] = { 0xFF, 0xD8, 0xEA };

jpeg_data_t jpeg_data_1;
jpeg_data_t jpeg_data_2;

/* Static fun ------------------------------------------------------------ */
static void uart_init();
static void uart_msg_task(void *pvParameters);

void setup() {
  delay(500);
  
  M5.begin(true, false, true);
  uart_init();
  jpeg_data_1.buf = (uint8_t *) malloc(sizeof(uint8_t) * 1024 * 37);
  if(jpeg_data_1.buf == NULL) {
    Serial.println("malloc jpeg buffer 1 error");
  }

  jpeg_data_2.buf = (uint8_t *) malloc(sizeof(uint8_t) * 1024 * 37);
  if(jpeg_data_2.buf == NULL) {
    Serial.println("malloc jpeg buffer 2 error");
  }

  xTaskCreatePinnedToCore(uart_msg_task, "uart_task", 3 * 1024, NULL, 1, NULL, 0);
}

void loop() {
    uint32_t data_len = 0;
    uint8_t rx_buffer[21] = { '\0' };
  
    delay(1);
    if(jpeg_data_1.idle == 1) {
      jpeg_data_1.idle = 2;
      M5.lcd.drawJpg(jpeg_data_1.buf, jpeg_data_1.length, 40, 30);
      jpeg_data_1.idle = 0;
    }
    
    if(jpeg_data_2.idle == 1) {
      jpeg_data_2.idle = 2;
      M5.lcd.drawJpg(jpeg_data_2.buf, jpeg_data_2.length, 40, 30);
      jpeg_data_2.idle = 0;
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
    // We change io in here, now use Serial2
    uart_set_pin(UART_NUM_1, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE, 0, 0, NULL, 0);
}

static void uart_msg_task(void *pvParameters) {
  uint32_t data_len = 0;
  uint32_t length = 0;
  uint8_t rx_buffer[10];
  uint8_t buffer_use = 0;
  while(true) {
    // delay(1);
    uart_get_buffered_data_len(UART_NUM_1, &data_len);
    if(data_len > 0) {
      switch(uart_rev_state) {
        case WAIT_FRAME_1:
          uart_read_bytes(UART_NUM_1, (uint8_t *)&rx_buffer, 1, 10);
          if(rx_buffer[0] == frame_data_begin[0]) {
            uart_rev_state = WAIT_FRAME_2; 
          }
          else {
            break ;
          }
          
        case WAIT_FRAME_2:
          uart_read_bytes(UART_NUM_1, (uint8_t *)&rx_buffer, 1, 10);
          if(rx_buffer[0] == frame_data_begin[1]){
            uart_rev_state = WAIT_FRAME_3; 
          } else {
            uart_rev_state = WAIT_FRAME_1;
            break ;
          }

        case WAIT_FRAME_3:
          uart_read_bytes(UART_NUM_1, (uint8_t *)&rx_buffer, 1, 10);
          if(rx_buffer[0] == frame_data_begin[2]){
            uart_rev_state = GET_NUM; 
          } else {
            uart_rev_state = WAIT_FRAME_1;
            break ;
          }

        case GET_NUM:
          uart_read_bytes(UART_NUM_1, (uint8_t *)&rx_buffer, 1, 10);
          Serial.printf("get number cam buf %d\t", rx_buffer[0]);
          uart_rev_state = GET_LENGTH;

        case GET_LENGTH:
          uart_read_bytes(UART_NUM_1, (uint8_t *)&rx_buffer, 3, 10);
          data_len =(uint32_t)(rx_buffer[0] << 16) | (rx_buffer[1] << 8) | rx_buffer[2];
          Serial.printf("data length %d\r\n", data_len);

        case GET_MSG:
          if(buffer_use == 0) {
            buffer_use = 1;
            if(jpeg_data_1.idle == 0) {
              if(uart_read_bytes(UART_NUM_1, jpeg_data_1.buf, data_len, 10) == -1) {
                uart_rev_state = RECV_ERROR;
                break ;
              }
              jpeg_data_1.length = data_len;
              jpeg_data_1.idle = 1;
            }
          } else {
            buffer_use = 0;
            if(jpeg_data_2.idle == 0) {
              if(uart_read_bytes(UART_NUM_1, jpeg_data_2.buf, data_len, 10) == -1) {
                uart_rev_state = RECV_ERROR;
                break ;
              }
              jpeg_data_2.length = data_len;
              jpeg_data_2.idle = 1;
            }
          }
          Serial.printf("get image %d buffer", buffer_use);
          uart_rev_state = RECV_FINISH;

        case RECV_FINISH:
          Serial.printf("get image finish...\r\n");
          uart_rev_state = WAIT_FRAME_1;          
          break ;

        case RECV_ERROR:
          Serial.printf("get image error\r\n");
          uart_rev_state = WAIT_FRAME_1;
          break ;
      }
    } else {
      vTaskDelay(10 / portTICK_RATE_MS);
    }

  }
  vTaskDelete(NULL);
}
