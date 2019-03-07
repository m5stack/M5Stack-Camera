# [ESP32CAM](https://docs.m5stack.com/#/en/unit/esp32cam) Firmware

## Firmware Description

This repository based in [esp32-camera](https://github.com/espressif/esp32-camera.git) is the firmware for [ESP32CAM](https://docs.m5stack.com/#/en/unit/esp32cam). Additionally it provides a few tools, which allow converting the captured frame data to the more common BMP and JPEG formats.

## Note

Now, M5Stack has four types of camera units, there are respectively [ESP32CAM](https://docs.m5stack.com/#/en/unit/esp32cam), [M5Camera (A Model)](https://docs.m5stack.com/#/en/unit/m5camera), [M5Camera (B Model)](https://docs.m5stack.com/#/en/unit/m5camera), M5CameraX, [M5CameraF](https://docs.m5stack.com/#/en/unit/m5camera_f).

The main differences between these cameras are **memory**, **interface**, **lens**, **optional hardware** and **camera shell**。

**Different branches correspond to different versions of hardware:**

- [master](https://github.com/m5stack/m5stack-cam-psram/tree/master) -> M5Camera (B Model) / M5CameraX

- [ModeA](https://github.com/m5stack/m5stack-cam-psram/tree/ModeA) -> M5Camera (A Model)

- [NoPsram](https://github.com/m5stack/m5stack-cam-psram/tree/NoPsram) -> ESP32CAM

- [FishEye](https://github.com/m5stack/m5stack-cam-psram/tree/FishEye
) -> M5CameraF

### Comparison of different versions of cameras

The picture below is their comparison table. (<mark>Note:</mark> Because the interface has many different pins, so I have made a separate table to compare.)

- If you want to **view** the detailed defference with them, please click [here](https://shimo.im/sheets/gP96C8YTdyjGgKQC).

- If you want to **download** the detailed defference with them, please click [here](https://github.com/m5stack/M5-Schematic/blob/master/Units/m5camera/M5%20Camera%20Detailed%20Comparison.xlsx).

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/camera_main_comparison_en.png">

#### The picture of A model and B model

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/diff_A_B.png">

### Interface Comparison

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/CameraPinComparison_en.png">

#### Interface Difference

The following table shows interface difference between those camera boads based on the `Interface Comparison` table.

<img src="https://m5stack.oss-cn-shenzhen.aliyuncs.com/image/m5-docs_table/camera_comparison/CameraPinDifference_en.png">

## Important to Remember

- Except when using CIF or lower resolution with JPEG, the driver requires PSRAM to be installed and activated.
- Using YUV or RGB puts a lot of strain on the chip because writing to PSRAM is not particularly fast. The result is that image data might be missing. This is particularly true if WiFi is enabled. If you need RGB data, it is recommended that JPEG is captured and then turned into RGB using `fmt2rgb888` or `fmt2bmp`/`frame2bmp`.
- When 1 frame buffer is used, the driver will wait for the current frame to finish (VSYNC) and start I2S DMA. After the frame is acquired, I2S will be stopped and the frame buffer returned to the application. This approach gives more control over the system, but results in longer time to get the frame.
- When 2 or more frame bufers are used, I2S is running in continuous mode and each frame is pushed to a queue that the application can access. This approach puts more strain on the CPU/Memory, but allows for double the frame rate. Please use only with JPEG.

## Installation Instructions

- Clone or download and extract the repository to the components folder of your ESP-IDF project
- `Make`