# [M5Camera (B Model)](https://docs.m5stack.com/#/zh_CN/unit/m5camera) / M5CameraX ファームウェア

[English](https://github.com/m5stack/m5stack-cam-psram/blob/master/README.md) | [中文](https://github.com/m5stack/m5stack-cam-psram/blob/master/README_zh_CN.md) | 日本語

## ファームウェア概要

[esp32-camera](https://github.com/espressif/esp32-camera.git)ベースの[M5Camera (B Model)](https://docs.m5stack.com/#/ja/unit/m5camera) と `M5CameraX`向けファームウェアリポジトリです。またそれ以外に、キャプチャしたフレームデータを、`BMP`や`JPEG`フォーマットに変換する為のツールを含みます。

## ノート

現在、M5Stack向けには、4タイプ５種類のカメラユニットが存在します。 それぞれ [ESP32CAM](https://docs.m5stack.com/#/ja/unit/esp32cam)、[M5Camera (A Model)](https://docs.m5stack.com/#/ja/unit/m5camera)、[M5Camera (B Model)](https://docs.m5stack.com/#/ja/unit/m5camera)、M5CameraX、[M5CameraF](https://docs.m5stack.com/#/ja/unit/m5camera_f)です。

それぞれのカメラユニットの主な仕様の違いは **メモリ**, **インターフェース**, **レンズ**, **オプションのハードウェア**、そして**ケース**です。

**各カメラユニットに対応したリポジトリのブランチは以下の通りです:**

- [master](https://github.com/m5stack/m5stack-cam-psram/tree/master) -> M5Camera (B Model) / M5CameraX

- [ModeA](https://github.com/m5stack/m5stack-cam-psram/tree/ModeA) -> M5Camera (A Model)

- [NoPsram](https://github.com/m5stack/m5stack-cam-psram/tree/NoPsram) -> ESP32CAM

- [FishEye](https://github.com/m5stack/m5stack-cam-psram/tree/FishEye) -> M5CameraF

### カメラユニットの比較

比較表を以下に示します。

- **直接みる**場合は[こちら](https://shimo.im/sheets/gP96C8YTdyjGgKQC)。

- **download**する場合は[こちら](https://github.com/m5stack/M5-Schematic/blob/master/Units/m5camera/M5%20Camera%20Detailed%20Comparison.xlsx)。

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/camera_main_comparison_en.png">

####  A Model と B Model

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/diff_A_B.png">

### インターフェース比較

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/CameraPinComparison_en.png">

#### インターフェース差異（異なる部分のみ抜粋）

`Interface Comparison`の中からピン配列が異なるものだけ抜粋して、以下に示します。

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/CameraPinDifference_en.png">

## 重要なポイント

- JPEGまたはCIF以下の解像度の場合を除き、ドライバでPSRAMをアクティブにする必要があります。
- PSRAMへの書き込みは高速ではないため、YUVまたはRGBを使用するとチップに大きな負担がかかります。その結果、画像データが失われる可能性があります。WiFiが有効になっている場合、特に顕著です。RGBデータが必要な場合は、先にJPEGとしてキャプチャしてから、`fmt2rgb888`/`fmt2bmp`/`frame2bmp`を使用してRGBに変換することをお勧めします。
- 1フレームバッファを使用する場合、ドライバは現在のフレームが終了するのを待って（VSYNC）、I2S DMAを開始します。フレームが取得された後、I2Sは停止し、フレームバッファはアプリケーションに返されます。この方法ではシステムをより細かく制御できますが、フレームを取得する時間が長くなります。
- 2つ以上のフレームバッファが使用されている場合、I2Sは連続モードで動作しており、各フレームはアプリケーションがアクセスできるキューにプッシュされます。この方法では、CPU/メモリに負担がかかりますが、フレームレートを2倍にすることができます。 注意点としてJPEGのみで使用してください。

## インストール手順

- あなたのESP-IDFプロジェクトのコンポーネントフォルダーにリポジトリをクローンまたはダウンロードして解凍します。
- `Make`

## API

### Get img data

```c
camera_fb_t * fb = NULL;
// img frame を取得します。
fb = esp_camera_fb_get();
// img buf
uint8_t *buf = fb->buf;
// img bufのサイズlen
unit32_t buf_len = fb->len;

/* --- do something --- */

// need return img buf
esp_camera_fb_return(fb);
```

### Set ov2640 config

```c
sensor_t *s = esp_camera_sensor_get();
s->set_framesize(s, FRAMESIZE_VGA);
s->set_quality(s, 10);
...
```

詳細はこちら[sensor.h](components/esp32-camera/driver/include/sensor.h)

## Examples

### Initialization

```c
#include "esp_camera.h"

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
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_UXGA,//QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

esp_err_t camera_init(){
    //power up the camera if PWDN pin is defined
    if(CAM_PIN_PWDN != -1){
        pinMode(CAM_PIN_PWDN, OUTPUT);
        digitalWrite(CAM_PIN_PWDN, LOW);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

esp_err_t camera_capture(){
    //acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }
    //replace this with your own function
    process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);

    //return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return ESP_OK;
}
```

### JPEG HTTP Capture

```c
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t jpg_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}
```

### JPEG HTTP Stream

```c
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t jpg_stream_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
            if(fb->format != PIXFORMAT_JPEG){
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if(!jpeg_converted){
                    ESP_LOGE(TAG, "JPEG compression failed");
                    esp_camera_fb_return(fb);
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    return res;
}
```

### BMP HTTP Capture

```c
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

esp_err_t bmp_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    uint8_t * buf = NULL;
    size_t buf_len = 0;
    bool converted = frame2bmp(fb, &buf, &buf_len);
    esp_camera_fb_return(fb);
    if(!converted){
        ESP_LOGE(TAG, "BMP conversion failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    res = httpd_resp_set_type(req, "image/x-windows-bmp")
       || httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp")
       || httpd_resp_send(req, (const char *)buf, buf_len);
    free(buf);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "BMP: %uKB %ums", (uint32_t)(buf_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}
```
