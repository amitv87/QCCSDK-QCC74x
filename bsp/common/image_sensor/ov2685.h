#ifndef __OV2685_H__
#define __OV2685_H__

#include "image_sensor.h"

static struct image_sensor_command_s ov2685_init_list[] = {
    {0x0103, 0x0001},
    {0xffff, 0x0005},
    {0x3002, 0x0000},
    {0x3016, 0x001c},
    {0x3018, 0x0084},
    {0x301d, 0x00f0},
    {0x3020, 0x0000},
    {0x3082, 0x002c},
    {0x3083, 0x0003},
    {0x3084, 0x0007},
    {0x3085, 0x0003},
    {0x3086, 0x0000},
    {0x3087, 0x0000},
    {0x3501, 0x004e},
    {0x3502, 0x00e0},
    {0x3503, 0x0003},
    {0x350b, 0x0036},
    {0x3600, 0x00b4},
    {0x3603, 0x0035},
    {0x3604, 0x0024},
    {0x3605, 0x0000},
    {0x3620, 0x0024},
    {0x3621, 0x0034},
    {0x3622, 0x0003},
    {0x3628, 0x0010},
    {0x3705, 0x003c},
    {0x370a, 0x0021},
    {0x370c, 0x0050},
    {0x370d, 0x00c0},
    {0x3717, 0x0058},
    {0x3718, 0x0080},
    {0x3720, 0x0000},
    {0x3721, 0x0009},
    {0x3722, 0x0006},
    {0x3723, 0x0059},
    {0x3738, 0x0099},
    {0x3781, 0x0080},
    {0x3784, 0x000c},
    {0x3789, 0x0060},
    {0x3800, 0x0000},
    {0x3801, 0x0000},
    {0x3802, 0x0000},
    {0x3803, 0x0000},
    {0x3804, 0x0006},
    {0x3805, 0x004f},
    {0x3806, 0x0004},
    {0x3807, 0x00bf},
    {0x3808, 0x0006},
    {0x3809, 0x0040},
    {0x380a, 0x0004},
    {0x380b, 0x00b0},
    {0x380c, 0x0006},
    {0x380d, 0x00a4},
    {0x380e, 0x0005},
    {0x380f, 0x000e},
    {0x3810, 0x0000},
    {0x3811, 0x0008},
    {0x3812, 0x0000},
    {0x3813, 0x0008},
    {0x3814, 0x0011},
    {0x3815, 0x0011},
    {0x3819, 0x0004},
    {0x3820, 0x00c0},
    {0x3821, 0x0000},
    {0x3a06, 0x0001},
    {0x3a07, 0x0084},
    {0x3a08, 0x0001},
    {0x3a09, 0x0043},
    {0x3a0a, 0x0024},
    {0x3a0b, 0x0060},
    {0x3a0c, 0x0028},
    {0x3a0d, 0x0060},
    {0x3a0e, 0x0004},
    {0x3a0f, 0x008c},
    {0x3a10, 0x0005},
    {0x3a11, 0x000c},
    {0x4000, 0x0081},
    {0x4001, 0x0040},
    {0x4008, 0x0002},
    {0x4009, 0x0009},
    {0x4300, 0x0032},//output format
    {0x430e, 0x0000},
    {0x4602, 0x0002},
    {0x4837, 0x001e},
    {0x5000, 0x00ff},
    {0x5001, 0x0005},
    {0x5002, 0x0032},
    {0x5003, 0x0004},
    {0x5004, 0x00ff},
    {0x5005, 0x0012},
    //{0x5080, 0x0092},//test pattern
    {0x0100, 0x0001},
    {0x0101, 0x0001},
    {0x1000, 0x0003},
    {0x0129, 0x0010},
    {0x5180, 0x00f4},
    {0x5181, 0x0011},
    {0x5182, 0x0041},
    {0x5183, 0x0042},
    {0x5184, 0x0078},
    {0x5185, 0x0058},
    {0x5186, 0x00b5},
    {0x5187, 0x00b2},
    {0x5188, 0x0008},
    {0x5189, 0x000e},
    {0x518a, 0x000c},
    {0x518b, 0x004c},
    {0x518c, 0x0038},
    {0x518d, 0x00f8},
    {0x518e, 0x0004},
    {0x518f, 0x007f},
    {0x5190, 0x0040},
    {0x5191, 0x005f},
    {0x5192, 0x0040},
    {0x5193, 0x00ff},
    {0x5194, 0x0040},
    {0x5195, 0x0007},
    {0x5196, 0x0004},
    {0x5197, 0x0004},
    {0x5198, 0x0000},
    {0x5199, 0x0005},
    {0x519a, 0x00d2},
    {0x519b, 0x0010},
    {0x5200, 0x0009},
    {0x5201, 0x0000},
    {0x5202, 0x0006},
    {0x5203, 0x0020},
    {0x5204, 0x0041},
    {0x5205, 0x0016},
    {0x5206, 0x0000},
    {0x5207, 0x0005},
    {0x520b, 0x0030},
    {0x520c, 0x0075},
    {0x520d, 0x0000},
    {0x520e, 0x0030},
    {0x520f, 0x0075},
    {0x5210, 0x0000},
    {0x5280, 0x0014},
    {0x5281, 0x0002},
    {0x5282, 0x0002},
    {0x5283, 0x0004},
    {0x5284, 0x0006},
    {0x5285, 0x0008},
    {0x5286, 0x000c},
    {0x5287, 0x0010},
    {0x5300, 0x00c5},
    {0x5301, 0x00a0},
    {0x5302, 0x0006},
    {0x5303, 0x000a},
    {0x5304, 0x0030},
    {0x5305, 0x0060},
    {0x5306, 0x0090},
    {0x5307, 0x00c0},
    {0x5308, 0x0082},
    {0x5309, 0x0000},
    {0x530a, 0x0026},
    {0x530b, 0x0002},
    {0x530c, 0x0002},
    {0x530d, 0x0000},
    {0x530e, 0x000c},
    {0x530f, 0x0014},
    {0x5310, 0x001a},
    {0x5311, 0x0020},
    {0x5312, 0x0080},
    {0x5313, 0x004b},
    {0x5380, 0x0001},
    {0x5381, 0x0052},
    {0x5382, 0x0000},
    {0x5383, 0x004a},
    {0x5384, 0x0000},
    {0x5385, 0x00b6},
    {0x5386, 0x0000},
    {0x5387, 0x008d},
    {0x5388, 0x0000},
    {0x5389, 0x003a},
    {0x538a, 0x0000},
    {0x538b, 0x00a6},
    {0x538c, 0x0000},
    {0x5400, 0x000d},
    {0x5401, 0x0018},
    {0x5402, 0x0031},
    {0x5403, 0x005a},
    {0x5404, 0x0065},
    {0x5405, 0x006f},
    {0x5406, 0x0077},
    {0x5407, 0x0080},
    {0x5408, 0x0087},
    {0x5409, 0x008f},
    {0x540a, 0x00a2},
    {0x540b, 0x00b2},
    {0x540c, 0x00cc},
    {0x540d, 0x00e4},
    {0x540e, 0x00f0},
    {0x540f, 0x00a0},
    {0x5410, 0x006e},
    {0x5411, 0x0006},
    {0x5480, 0x0019},
    {0x5481, 0x0000},
    {0x5482, 0x0009},
    {0x5483, 0x0012},
    {0x5484, 0x0004},
    {0x5485, 0x0006},
    {0x5486, 0x0008},
    {0x5487, 0x000c},
    {0x5488, 0x0010},
    {0x5489, 0x0018},
    {0x5500, 0x0002},
    {0x5501, 0x0003},
    {0x5502, 0x0004},
    {0x5503, 0x0005},
    {0x5504, 0x0006},
    {0x5505, 0x0008},
    {0x5506, 0x0000},
    {0x5600, 0x0002},
    {0x5603, 0x0040},
    {0x5604, 0x0028},
    {0x5609, 0x0020},
    {0x560a, 0x0060},
    {0x5800, 0x0003},
    {0x5801, 0x0024},
    {0x5802, 0x0002},
    {0x5803, 0x0040},
    {0x5804, 0x0034},
    {0x5805, 0x0005},
    {0x5806, 0x0012},
    {0x5807, 0x0005},
    {0x5808, 0x0003},
    {0x5809, 0x003c},
    {0x580a, 0x0002},
    {0x580b, 0x0040},
    {0x580c, 0x0026},
    {0x580d, 0x0005},
    {0x580e, 0x0052},
    {0x580f, 0x0006},
    {0x5810, 0x0003},
    {0x5811, 0x0028},
    {0x5812, 0x0002},
    {0x5813, 0x0040},
    {0x5814, 0x0024},
    {0x5815, 0x0005},
    {0x5816, 0x0042},
    {0x5817, 0x0006},
    {0x5818, 0x000d},
    {0x5819, 0x0040},
    {0x581a, 0x0004},
    {0x581b, 0x000c},
    {0x3a03, 0x004c},
    {0x3a04, 0x0040},
    {0x3503, 0x0000},
};

static struct image_sensor_config_s ov2685_config = {
    .name = "OV2685",
    .output_format = IMAGE_SENSOR_FORMAT_YUV422_YUYV,
    .slave_addr = 0x3c,
    .id_size = 2,
    .reg_size = 2,
    .h_blank = 0xde,
    .resolution_x = 1600,
    .resolution_y = 1200,
    .id_addr = 0x300a300b,
    .id_value = 0x2685,
    .pixel_clock = 66000000,
    .init_list_len = sizeof(ov2685_init_list)/sizeof(ov2685_init_list[0]),
    .init_list = ov2685_init_list,
};

#endif /* __OV2685_H__ */
