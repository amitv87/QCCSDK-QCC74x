#include "qcc74x_mtimer.h"
#include "qcc74x_mjpeg.h"
#include "board.h"
#include "jpeg_head.h"
#include "test_64x64.h"

#define X 64
#define Y 64

uint8_t jpg_head_buf[800] = { 0 };
uint32_t jpg_head_len;

uint8_t MJPEG_QUALITY = 50;

static __attribute__((aligned(32))) ATTR_NOINIT_PSRAM_SECTION uint8_t mjpeg_buffer[50 * 1024];

void qcc74x_mjpeg_dump_hex(uint8_t *data, uint32_t len)
{
    uint32_t i = 0;

    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\r\n");
        }

        printf("%02x ", data[i]);
    }

    printf("\r\n");
}

static struct qcc74x_device_s *mjpeg;

volatile uint32_t pic_count = 0;
volatile uint8_t *pic_addr;
volatile uint32_t pic_len;

void mjpeg_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_mjpeg_get_intstatus(mjpeg);
    if (intstatus & MJPEG_INTSTS_ONE_FRAME) {
        qcc74x_mjpeg_int_clear(mjpeg, MJPEG_INTCLR_ONE_FRAME);
        pic_len = qcc74x_mjpeg_get_frame_info(mjpeg, &pic_addr);
        pic_count = 1;
        qcc74x_mjpeg_pop_one_frame(mjpeg);
    }
}

void mjpeg_init(uint32_t x, uint32_t y, uint8_t *yuv, uint8_t *jpeg, uint32_t size, uint8_t quality)
{
    struct qcc74x_mjpeg_config_s config;

    mjpeg = qcc74x_device_get_by_name("mjpeg");

    config.format = MJPEG_FORMAT_YUV422_YUYV;
    config.quality = quality;
    config.rows = y;
    config.resolution_x = x;
    config.resolution_y = y;
    config.input_bufaddr0 = (uint32_t)yuv;
    config.input_bufaddr1 = 0;
    config.output_bufaddr = (uint32_t)jpeg;
    config.output_bufsize = size;
    config.input_yy_table = NULL; /* use default table */
    config.input_uv_table = NULL; /* use default table */

    qcc74x_mjpeg_init(mjpeg, &config);
    jpg_head_len = JpegHeadCreate(YUV_MODE_422, quality, x, y, jpg_head_buf);
    qcc74x_mjpeg_fill_jpeg_header_tail(mjpeg, jpg_head_buf, jpg_head_len);
    qcc74x_mjpeg_tcint_mask(mjpeg, false);
    qcc74x_irq_attach(mjpeg->irq_num, mjpeg_isr, NULL);
    qcc74x_irq_enable(mjpeg->irq_num);
}

void mjpeg_compress(uint8_t *yuv, uint8_t *jpeg, uint32_t size)
{
    qcc74x_mjpeg_update_input_output_buff(mjpeg, yuv, NULL, jpeg, size);
    qcc74x_mjpeg_sw_run(mjpeg, 1);
}

int main(void)
{
    board_init();

    mjpeg_init(X, Y, NULL, NULL, 0, MJPEG_QUALITY);

    /* compress one pic */
    mjpeg_compress(test_64x64, mjpeg_buffer, 50 * 1024);

    while (pic_count != 1) {
        qcc74x_mtimer_delay_ms(200);
    }

    printf("jpg addr:%08x ,jpg size:%d\r\n", pic_addr, pic_len);
    qcc74x_mjpeg_dump_hex(pic_addr, pic_len);

    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}