#include "image_sensor.h"
#include "bf2013.h"
#include "gc0308.h"
#include "gc0328.h"
#include "gc2053.h"
#include "gc2145.h"
#include "ov2685.h"
#include "sc101iot.h"
#include "qcc74x_i2c.h"

static struct image_sensor_config_s *sensor_list[] = {
    NULL, &bf2013_config, &gc0308_config, &gc0328_config, &gc2053_config, &gc2145_config, &ov2685_config, &sc101iot_config
};

void image_sensor_read(struct qcc74x_device_s *i2c, uint32_t sensor_index, struct image_sensor_command_s *read_list, uint32_t list_len)
{
    uint32_t i;
    struct qcc74x_i2c_msg_s msgs[2];
    uint8_t buffer[2];
    
    msgs[0].addr = sensor_list[sensor_index]->slave_addr;
    msgs[0].flags = I2C_M_NOSTOP;
    msgs[0].buffer = NULL;
    msgs[0].length = sensor_list[sensor_index]->reg_size;

    msgs[1].addr = sensor_list[sensor_index]->slave_addr;
    msgs[1].flags = I2C_M_READ;
    msgs[1].buffer = NULL;
    msgs[1].length = 1;
    
    for(i=0;i<list_len;i++){
        if (sensor_list[sensor_index]->reg_size == 1) {
            buffer[0] = read_list[i].address & 0xff;
        } else {
            buffer[0] = read_list[i].address >> 8;
            buffer[1] = read_list[i].address & 0xff;
        }
        
        msgs[0].buffer = buffer;
        msgs[1].buffer = &read_list[i].paramete;
        qcc74x_i2c_transfer(i2c, msgs, 2);
    }
}

void image_sensor_write(struct qcc74x_device_s *i2c, uint32_t sensor_index, struct image_sensor_command_s *read_list, uint32_t list_len)
{
    uint32_t i;
    struct qcc74x_i2c_msg_s msgs[2];
    uint8_t buffer[2];
    
    msgs[0].addr = sensor_list[sensor_index]->slave_addr;
    msgs[0].flags = I2C_M_NOSTOP;
    msgs[0].buffer = NULL;
    msgs[0].length = sensor_list[sensor_index]->reg_size;

    msgs[1].addr = sensor_list[sensor_index]->slave_addr;
    msgs[1].flags = 0;
    msgs[1].buffer = NULL;
    msgs[1].length = 1;
    
    for(i=0;i<list_len;i++){
        if (sensor_list[sensor_index]->reg_size == 1) {
            buffer[0] = read_list[i].address & 0xff;
        } else {
            buffer[0] = read_list[i].address >> 8;
            buffer[1] = read_list[i].address & 0xff;
        }
        
        msgs[0].buffer = buffer;
        msgs[1].buffer = &read_list[i].paramete;
        qcc74x_i2c_transfer(i2c, msgs, 2);
    }
}

uint32_t image_sensor_scan(struct qcc74x_device_s *i2c, struct image_sensor_config_s **config)
{
    uint32_t i, j;
    uint32_t sensor_match_flag = 0;
    struct image_sensor_command_s read_id = {0, 0};
    
    qcc74x_i2c_init(i2c, 100000);
    
    for(i=1;i<sizeof(sensor_list)/sizeof(sensor_list[0]);i++){
        for(j=0;j<sensor_list[i]->id_size;j++){
            if(sensor_list[i]->reg_size == 1){
                read_id.address = sensor_list[i]->id_addr>>(8*j)&0xff;
            }else{
                read_id.address = sensor_list[i]->id_addr>>(16*j)&0xffff;
            }
            image_sensor_read(i2c, i, &read_id, 1);
            if(read_id.paramete != (sensor_list[i]->id_value>>(8*j)&0xff)){
                sensor_match_flag = 0;
                break;
            }
            sensor_match_flag = 1;
        }
        
        if(sensor_match_flag == 1){
            *config = sensor_list[i];
            image_sensor_write(i2c, i, sensor_list[i]->init_list, sensor_list[i]->init_list_len);
            return(i);
        }
    }
    
    return 0;
}

