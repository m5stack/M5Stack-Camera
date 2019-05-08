#include <stdio.h>
#include <string.h>
#include "quirc_internal.h"
#include "qr_recoginize.h"

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

void dump_data(const struct quirc_data *data) {
	//printf("    Version: %d\n", data->version);
	//printf("    ECC level: %c\n", "MLHQ"[data->ecc_level]);
	//printf("    Mask: %d\n", data->mask);
	//printf("    Data type: %d (%s)\n", data->data_type,
	//		data_type_str(data->data_type));
	//printf("    Length: %d\n", data->payload_len);
	//printf("    Payload: %s\n", data->payload);
    printf("%s\n",data->payload);
	//if (data->eci)
		//printf("    ECI: %d\n", data->eci);

    //printf("\n");
}

static void dump_info(struct quirc *q) {
	int count = quirc_count(q);
	int i;
	for (i = 0; i < count; i++) {
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;

		quirc_extract(q, i, &code);
		err = quirc_decode(&code, &data);

		//dump_cells(&code);
		//printf("\n");

		if (err) {
			//printf("Decoding FAILED: %s\n", quirc_strerror(err));
		} else {
			printf("%d QR-codes found\n", count);
			//printf("Decoding successful:\n");
			dump_data(&data);
		}

		//printf("\n");
	}
}

void qr_recoginze(void *pdata) {

	camera_fb_t *camera_config = pdata;

	if(pdata==NULL)
	{
		ESP_LOGI(TAG,"Camera Size err");
		return;
	}

	struct quirc *q;
	struct quirc_data qd;
	uint8_t *image;
	q = quirc_new();
	
	if (!q) {
		printf("can't create quirc object\r\n");
		vTaskDelete(NULL) ;
	}
	//printf("begin to quirc_resize\r\n");
	
	if (quirc_resize(q, camera_config->width, camera_config->height)< 0)
	{
		printf("quirc_resize err\r\n");
		quirc_destroy(q);
		vTaskDelete(NULL) ;
	}

	image = quirc_begin(q, NULL, NULL);
	memcpy(image, camera_config->buf, camera_config->len);
	quirc_end(q);
	
	int id_count = quirc_count(q);
	if (id_count == 0) {
		quirc_destroy(q);
		return;
	}

	struct quirc_code code;
	quirc_extract(q, 0, &code);
	quirc_decode(&code, &qd);
	dump_info(q);
	quirc_destroy(q);
}


