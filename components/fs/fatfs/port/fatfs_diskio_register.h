#ifndef FATFS_DISKIO_REGISTER_H
#define FATFS_DISKIO_REGISTER_H

#if defined(QCC743) || defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undefP)

void fatfs_sdh_driver_register(void);
void fatfs_usbh_driver_register(void);

#endif

#endif