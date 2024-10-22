#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>

#include "version.h"
#include "main.h"
#include "tty.h"

struct qcc74x_device gl_dev;

#ifdef QCC74x_PROCFS_ENABLE
struct proc_dir_entry *qcc74x_proc_dir = NULL;
#endif

int __init qcc74x_mod_init(void)
{
    int ret;

    pr_info("qcwnet version %d.%d\n",
            QCC74x_WNET_MAJOR_VERSION, QCC74x_WNET_MINOR_VERSION);
#ifdef QCC74x_PROCFS_ENABLE
    qcc74x_proc_dir = proc_mkdir("qceth_driver", NULL);
    if (!qcc74x_proc_dir)
        pr_warn("failed to mkdir /proc/qceth_driver\n");
#endif
    mutex_init(&gl_dev.mutex);

#ifdef QCC74x_INTF_SDIO
    if ((ret = qcc74x_tty_init())) {
        return ret;
    }
#endif
    if ((ret = qcc74x_register_ctl_port()) != 0) {
        return ret;
    }

    return qcc74x_register_eth_drv();
}

void __exit qcc74x_mod_exit(void)
{
    pr_info("removing qceth_driver\n");
    mutex_lock(&gl_dev.mutex);
    gl_dev.status |= QCC74x_DEVICE_STATUS_DRV_REMOVING;
    mutex_unlock(&gl_dev.mutex);
    qcc74x_release_ctl_port();
    qcc74x_unregister_eth_drv();
#ifdef QCC74x_INTF_SDIO
    qcc74x_tty_exit();
#endif
#ifdef QCC74x_PROCFS_ENABLE
    proc_remove(qcc74x_proc_dir);
#endif
}

bool qcc74x_card_ok(void)
{
    int nok = 0;
    if (!(gl_dev.status & QCC74x_DEVICE_STATUS_CARD_PRESENT)) {
        ++nok;
    }
#if CARD_DEAD_CHECK
    if (gl_dev.status & QCC74x_DEVICE_STATUS_CARD_DEAD) {
        ++nok;
    }
#endif
    if (gl_dev.status & QCC74x_DEVICE_STATUS_CARD_REMOVING) {
        ++nok;
    }
    if (gl_dev.status & QCC74x_DEVICE_STATUS_DRV_REMOVING) {
        ++nok;
    }
    return !nok;
}

module_init(qcc74x_mod_init);
module_exit(qcc74x_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("qcc74x Device net driver");
