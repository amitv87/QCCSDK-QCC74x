#include "qcc74x_boot2.h"
#include "qcc74x_mtd.h"
#include "qcc74x_flash.h"

struct qcc74x_mtd_handle_priv {
    char name[16];
    int id;
    unsigned int offset;
    unsigned int size;
    void *xip_addr;
};
typedef struct qcc74x_mtd_handle_priv *qcc74x_mtd_handle_priv_t;

static void __dump_mtd_handle(qcc74x_mtd_handle_priv_t handle_prv)
{
    printf(  "[MTD] >>>>>> Hanlde info Dump >>>>>>\r\n");
    printf("      name %s\r\n", handle_prv->name);
    printf("      id %d\r\n", handle_prv->id);
    printf("      offset %p(%u)\r\n", (void*)handle_prv->offset, handle_prv->offset);
    printf("      size %p(%uKbytes)\r\n", (void*)handle_prv->size, handle_prv->size/1024);
    printf("      xip_addr %p\r\n", handle_prv->xip_addr);
    printf(  "[MTD] <<<<<< Hanlde info End <<<<<<\r\n");
}

static int _mtd_write_copy2ram(uint32_t addr, uint8_t *src, unsigned int len)
{
    uint8_t buf_tmp[64];
    unsigned int len_tmp;

    while (len > 0) {
        if (len >= 64) {
            len_tmp = 64;
        } else {
            len_tmp = len;
        }

        memcpy(buf_tmp, src, len_tmp);

        qcc74x_flash_write(addr, buf_tmp, len_tmp);

        addr += len_tmp;
        src += len_tmp;
        len -= len_tmp;
    }

    return 0;
}

static int _mtd_write(uint32_t addr, uint8_t *src, unsigned int len)
{

    qcc74x_flash_write(addr, src, len);

    return 0;
}

#if defined(QCC74x_undef) || defined(QCC74x_undef)
int qcc74x_sys_isxipaddr(uint32_t addr)
{
    if (((addr & 0xFF000000) == 0x23000000) || ((addr & 0xFF000000) == 0x43000000)) {
        return 1;
    }
    return 0;
}
#elif defined(QCC743)
static int qcc74x_sys_isxipaddr(uint32_t addr)
{
    if ( ((addr & 0xFF000000) == 0xA0000000) || ((addr & 0xFF000000) == 0xA1000000) ||
         ((addr & 0xFF000000) == 0xA2000000) || ((addr & 0xFF000000) == 0xA3000000) ||
         ((addr & 0xFF000000) == 0xA4000000) || ((addr & 0xFF000000) == 0xA5000000) ||
         ((addr & 0xFF000000) == 0xA6000000) || ((addr & 0xFF000000) == 0xA7000000) ) {
        return 1;
    }
    return 0;
}
#elif defined(QCC74x_undef)
int qcc74x_sys_isxipaddr(uint32_t addr)
{
    //XXX is 0xD000_0000 Address Range is really used
    if ( ((addr & 0xFF000000) == 0x58000000) || ((addr & 0xFF000000) == 0x5C000000) ||
         ((addr & 0xFF000000) == 0xD8000000) || ((addr & 0xFF000000) == 0xDC000000)) {
        return 1;
    }
    return 0;
}
#endif

void qcc74x_mtd_init(void)
{
    qcc74x_boot2_init();
}

int qcc74x_mtd_open(const char *name, qcc74x_mtd_handle_t *handle, unsigned int flags)
{
    uint32_t addr = 0;
    uint32_t size = 0;
    int ret;
    qcc74x_mtd_handle_priv_t handle_prv;

    handle_prv = (qcc74x_mtd_handle_priv_t)malloc(sizeof(struct qcc74x_mtd_handle_priv));

    if (NULL == handle_prv) {
        return -1;
    }
    memset(handle_prv, 0, sizeof(struct qcc74x_mtd_handle_priv));
    if(strlcpy(handle_prv->name, name, sizeof(handle_prv->name)) >= sizeof(handle_prv->name))
        printf("[OS]: strlcpy truncated \r\n");

    if (flags & QCC74x_MTD_OPEN_FLAG_BACKUP) {
        /* open backup mtd partition*/
        if (flags & QCC74x_MTD_OPEN_FLAG_BUSADDR) {
            if ((ret = qcc74x_boot2_partition_bus_addr_inactive(name, &addr, &size)) || 0 == addr) {
                printf("[MTD] [PART] [XIP] error when get %s partition %d\r\n", name, ret);
                printf("[MTD] [PART] [XIP] Dead Loop. Reason: no Valid %s partition found\r\n", name);
                while (1) {
                }
            }
            handle_prv->xip_addr = (void*)addr;
        } else {
            handle_prv->xip_addr = 0;
        }
        if ((ret = qcc74x_boot2_partition_addr_inactive(name, &addr, &size)) || 0 == addr) {
            printf("[MTD] [PART] [XIP] error when get %s partition %d\r\n", name, ret);
            printf("[MTD] [PART] [XIP] Dead Loop. Reason: no Valid %s partition found\r\n", name);
            while (1) {
            }
        }
        handle_prv->offset = addr;
        handle_prv->size = size;
    } else {
        /* open active mtd partition*/
        if (flags & QCC74x_MTD_OPEN_FLAG_BUSADDR) {
            if ((ret = qcc74x_boot2_partition_bus_addr_active(name, &addr, &size)) || 0 == addr) {
                printf("[MTD] [PART] [XIP] error when get %s partition %d\r\n", name, ret);
                printf("[MTD] [PART] [XIP] Dead Loop. Reason: no Valid %s partition found\r\n", name);
                while (1) {
                }
            }
            handle_prv->xip_addr = (void*)addr;
        } else {
            handle_prv->xip_addr = 0;
        }
        if ((ret = qcc74x_boot2_partition_addr_active(name, &addr, &size)) || 0 == addr) {
            printf("[MTD] [PART] [XIP] error when get %s partition %d\r\n", name, ret);
            printf("[MTD] [PART] [XIP] Dead Loop. Reason: no Valid %s partition found\r\n", name);
            while (1) {
            }
        }
        handle_prv->offset = addr;
        handle_prv->size = size;
    }
    __dump_mtd_handle(handle_prv);
    *handle = handle_prv;

    return 0;
}

int qcc74x_mtd_close(qcc74x_mtd_handle_t handle)
{
    free(handle);

    return 0;
}

int qcc74x_mtd_info(qcc74x_mtd_handle_t handle, qcc74x_mtd_info_t *info)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;

    if(strlcpy(info->name, handle_prv->name, sizeof(info->name)) >= sizeof(info->name))
        printf("[OS]: strlcpy truncated \r\n");
    info->offset = handle_prv->offset;
    info->size = handle_prv->size;
    info->xip_addr = handle_prv->xip_addr;

    return 0;
}

int qcc74x_mtd_erase(qcc74x_mtd_handle_t handle, unsigned int addr, unsigned int size)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;

    qcc74x_flash_erase(
            handle_prv->offset + addr,
            size
    );

    return 0;
}

int qcc74x_mtd_erase_all(qcc74x_mtd_handle_t handle)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;

    qcc74x_flash_erase(
            handle_prv->offset + 0,
            handle_prv->size
    );

    return 0;
}

int qcc74x_mtd_write(qcc74x_mtd_handle_t handle, unsigned int addr, unsigned int size, const uint8_t *data)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;
    uint32_t real_addr = handle_prv->offset + addr;

    if (qcc74x_sys_isxipaddr((uint32_t)data)) {
        printf("addr@%p is xip flash, size %d\r\n", data, size);
        _mtd_write_copy2ram(real_addr, (uint8_t*)data, size);
    } else {
        _mtd_write(real_addr, (uint8_t*)data, size);
    }

    return 0;
}

int qcc74x_mtd_read(qcc74x_mtd_handle_t handle,  unsigned int addr, unsigned int size, uint8_t *data)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;

    qcc74x_flash_read(
            handle_prv->offset + addr,
            data,
            size
    );

    return 0;
}

int qcc74x_mtd_size(qcc74x_mtd_handle_t handle, unsigned int *size)
{
    qcc74x_mtd_handle_priv_t handle_prv = (qcc74x_mtd_handle_priv_t)handle;
    if (NULL == handle) {
        return -1;
    }
    *size = handle_prv->size;

    return 0;
}
