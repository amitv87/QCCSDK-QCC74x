#include "qcc74x_mtimer.h"
#include "qcc74x_i2c.h"
#include "qcc74x_cam.h"
#include "image_sensor.h"
#include "board.h"

#define CAM_FRAME_COUNT_USE 50

static struct qcc74x_device_s *i2c0;
static struct qcc74x_device_s *cam0;

int main(void)
{
    uint32_t i, pic_size;
    uint8_t *pic;
    struct qcc74x_cam_config_s cam_config;
    struct image_sensor_config_s *sensor_config;

    board_init();
    board_dvp_gpio_init();

    i2c0 = qcc74x_device_get_by_name("i2c0");
    cam0 = qcc74x_device_get_by_name("cam0");

    if (image_sensor_scan(i2c0, &sensor_config)) {
        printf("\r\nSensor name: %s\r\n", sensor_config->name);
    } else {
        printf("\r\nError! Can't identify sensor!\r\n");
        while (1) {
        }
    }

    memcpy(&cam_config, sensor_config, IMAGE_SENSOR_INFO_COPY_SIZE);
    cam_config.with_mjpeg = false;
    cam_config.input_source = CAM_INPUT_SOURCE_DVP;
    cam_config.output_format = CAM_OUTPUT_FORMAT_AUTO;
    cam_config.output_bufaddr = QCC74x_PSRAM_BASE;
    cam_config.output_bufsize = cam_config.resolution_x * cam_config.resolution_y * 8;

    qcc74x_cam_init(cam0, &cam_config);
    qcc74x_cam_start(cam0);

    for (i = 0; i < CAM_FRAME_COUNT_USE; i++) {
        while (qcc74x_cam_get_frame_count(cam0) == 0) {
        }
        pic_size = qcc74x_cam_get_frame_info(cam0, &pic);
        qcc74x_cam_pop_one_frame(cam0);
        printf("pop picture %d: 0x%08x, len: %d\r\n", i, (uint32_t)pic, pic_size);
    }

    qcc74x_cam_stop(cam0);

    //for (i = 0; i < pic_size; i++) {
    //    printf("%02x", pic[i]);
    //}
    //printf("\r\n");

    printf("end\r\n");

    while (1) {
        qcc74x_mtimer_delay_ms(1000);
    }
}
