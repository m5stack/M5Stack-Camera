#include <stdio.h>
#include <string.h>
#include "quirc_internal.h"
#include "qr_recoginize.h"
#include "driver/uart.h"
#include "esp_camera.h"
#include "quirc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
static char* TAG="QR";
static const char *data_type_str(int dt) {
	switch (dt) {
	case QUIRC_DATA_TYPE_NUMERIC:
		return "NUMERIC";
	case QUIRC_DATA_TYPE_ALPHA:
		return "ALPHA";
	case QUIRC_DATA_TYPE_BYTE:
		return "BYTE";
	case QUIRC_DATA_TYPE_KANJI:
		return "KANJI";
	}

	return "unknown";
}

void dump_cells(const struct quirc_code *code) {
	int u, v;

	printf("    %d cells, corners:", code->size);
	for (u = 0; u < 4; u++)
		printf(" (%d,%d)", code->corners[u].x, code->corners[u].y);
	printf("\n");

	for (v = 0; v < code->size; v++) {
		printf("\033[0m    ");
		for (u = 0; u < code->size; u++) {
			int p = v * code->size + u;

			if (code->cell_bitmap[p >> 3] & (1 << (p & 7)))
				printf("\033[40m  ");
			else
				printf("\033[47m  ");
		}
		printf("\033[0m\n");
	}
}
char tx_buffer1[200]= {0};
void dump_data(const struct quirc_data *data) {
	sprintf(tx_buffer1,"%s\n",data->payload);
    printf("%s",tx_buffer1);
    uart_write_bytes(UART_NUM_1, (char *)tx_buffer1, strlen(tx_buffer1));
    memset(tx_buffer1,0,strlen(tx_buffer1));
}
char tx_buffer[200] = {0};
static void dump_info(struct quirc *q) {
	int count = quirc_count(q);
	int i;
	for (i = 0; i < count; i++) {
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;

		quirc_extract(q, i, &code);
		err = quirc_decode(&code, &data);

		if(err) {
			//printf("Decoding FAILED: %s\n", quirc_strerror(err));
		} else {
			dump_data(&data);
		}
	}
}

void qr_recoginze(void *pdata, uint8_t *buf) {

	camera_fb_t *camera_config = pdata;

	if(pdata==NULL)
	{
		ESP_LOGI(TAG,"Camera Size err");
		return;
	}
    //printf("camera_config->format = %d len = %d\r\n",camera_config->format,camera_config->len);
	struct quirc *q;
	struct quirc_data qd;
	uint8_t *image;
	q = quirc_new();
	
	if (!q) {
		printf("can't create quirc object\r\n");
		//vTaskDelete(NULL) ;
		return;
	}
	//printf("qr scanning...\r\n");
	//printf("camera_config->width = %d height = %d\r\n",camera_config->width,camera_config->height);
	if(quirc_resize(q, camera_config->width, camera_config->height)< 0)
	{
		printf("quirc_resize err\r\n");
		quirc_destroy(q);
		return;
		//vTaskDelete(NULL) ;
	}
	image = quirc_begin(q, NULL, NULL);
	//memcpy(image, camera_config->buf, camera_config->len);
	memcpy(image, buf, 160 * 120);
	quirc_end(q);
	//printf("image = %d\r\n",sizeof(image));
	int id_count = quirc_count(q);
	if (id_count == 0) {
		quirc_destroy(q);
		return;
	}
    //long loopTime = xTaskGetTickCount();
	struct quirc_code code;
	quirc_extract(q, 0, &code);
	quirc_decode(&code, &qd);
	dump_info(q);
	quirc_destroy(q);
	//printf("loopTime = %ld\r\n",xTaskGetTickCount() - loopTime);
}


