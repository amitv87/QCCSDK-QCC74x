#if defined(QCC743) || defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undefP)

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "sdh_sdcard.h"
#include "qcc74x_irq.h"

static sd_card_t gSDCardInfo;

int MMC_disk_status()
{
    return 0;
}

int MMC_disk_initialize()
{
    static uint8_t inited = 0;

    if (inited == 0) {
        if (SDH_Init(SDH_DATA_BUS_WIDTH_4BITS, &gSDCardInfo) == SD_OK) {
            inited = 1;
            return 0;
        } else {
            return -1;
        }
    }

    return 0;
}

int MMC_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    if (SD_OK == SDH_ReadMultiBlocks(buff, sector, gSDCardInfo.blockSize, count)) {
        return 0;
    } else {
        return -1;
    }
}

int MMC_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    status_t ret;

_retry:
    ret = SDH_WriteMultiBlocks((uint8_t *)buff, sector, gSDCardInfo.blockSize, count);

    if (Status_Success == ret) {
        return 0;
    } else if (Status_Timeout == ret) {
        goto _retry;
    } else {
        return -1;
    }
}

int MMC_disk_ioctl(BYTE cmd, void *buff)
{
    switch (cmd) {
        // Get R/W sector size (WORD)
        case GET_SECTOR_SIZE:
            *(WORD *)buff = gSDCardInfo.blockSize;
            break;

        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1;
            break;

        case GET_SECTOR_COUNT:
            *(DWORD *)buff = gSDCardInfo.blockCount;
            break;

        case CTRL_SYNC:
            break;

        default:
            break;
    }

    return 0;
}

DSTATUS Translate_Result_Code(int result)
{
    return result;
}

void fatfs_sdh_driver_register(void)
{
    FATFS_DiskioDriverTypeDef SDH_DiskioDriver = { NULL };

    SDH_DiskioDriver.disk_status = MMC_disk_status;
    SDH_DiskioDriver.disk_initialize = MMC_disk_initialize;
    SDH_DiskioDriver.disk_write = MMC_disk_write;
    SDH_DiskioDriver.disk_read = MMC_disk_read;
    SDH_DiskioDriver.disk_ioctl = MMC_disk_ioctl;
    SDH_DiskioDriver.error_code_parsing = Translate_Result_Code;

    disk_driver_callback_init(DEV_SD, &SDH_DiskioDriver);
}

#endif
