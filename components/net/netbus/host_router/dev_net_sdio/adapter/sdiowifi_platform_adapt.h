
#ifndef __SDIOWIFI_PLATFORM_ADAPT_H__
#define __SDIOWIFI_PLATFORM_ADAPT_H__

#include "rtos_al.h"
#include "platform_al.h"

typedef uint64_t         sdiowifi_tick_t;
typedef rtos_semaphore   sdiowifi_semaphore;
typedef rtos_mutex       sdiowifi_mutex;
typedef rtos_task_handle sdiowifi_task_t;
typedef rtos_queue       sdiowifi_queue_t;
typedef void*            sdiowifi_streambuf_handle_t;
typedef void*            sdiowifi_timer_handle_t;

#define SDIOWIFI_WAIT_FOREVER 0xffffffffUL

#define sdiowifi_queue_create   rtos_queue_create
#define sdiowifi_queue_delete   rtos_queue_delete
#define sdiowifi_queue_is_empty rtos_queue_is_empty
#define sdiowifi_queue_is_full  rtos_queue_is_full
#define sdiowifi_queue_cnt      rtos_queue_cnt
#define sdiowifi_queue_write    rtos_queue_write
#define sdiowifi_queue_read     rtos_queue_read

#define sdiowifi_delay_ms        rtos_task_suspend
#define sdiowifi_tick_now        rtos_now
#define sdiowifi_task_create     rtos_task_create
#define sdiowifi_task_delete     rtos_task_delete
#define sdiowifi_get_task_handle rtos_get_task_handle
#define sdiowifi_enter_critical  rtos_protect
#define sdiowifi_exit_critical   rtos_unprotect

#define sdiowifi_task_wait_notification rtos_task_wait_notification
#define sdiowifi_task_notify            rtos_task_notify

#define sdiowifi_mutex_create rtos_mutex_create
#define sdiowifi_mutex_delete rtos_mutex_delete
#define sdiowifi_mutex_lock   rtos_mutex_lock
#define sdiowifi_mutex_unlock rtos_mutex_unlock

#define sdiowifi_semaphore_create rtos_semaphore_create
#define sdiowifi_semaphore_delete rtos_semaphore_delete
#define sdiowifi_semaphore_wait   rtos_semaphore_wait
#define sdiowifi_semaphore_signal rtos_semaphore_signal

sdiowifi_tick_t sdiowifi_ms2ticks(sdiowifi_tick_t ms);
sdiowifi_timer_handle_t sdiowifi_timer_create(void (*fn)(void *, void *), void *arg, int ms, int repeat, unsigned char auto_run);
void sdiowifi_timer_free(sdiowifi_timer_handle_t timer);
int sdiowifi_timer_start(sdiowifi_timer_handle_t timer);
int sdiowifi_timer_stop(sdiowifi_timer_handle_t timer);
int sdiowifi_timer_change(sdiowifi_timer_handle_t timer, int ms);
int sdiowifi_timer_is_valid(sdiowifi_timer_handle_t timer);
int sdiowifi_timer_change_once(sdiowifi_timer_handle_t timer, int ms);

sdiowifi_streambuf_handle_t sdiowifi_streambuf_create(size_t buf_size, size_t trigger_bytes);
int sdiowifi_streambuf_send(sdiowifi_streambuf_handle_t handle, const void *data, size_t len, size_t timeout_ms);
int sdiowifi_streambuf_receive(sdiowifi_streambuf_handle_t handle, void *data, size_t len, size_t timeout_ms);
int sdiowifi_streambuf_spaces_available(sdiowifi_streambuf_handle_t handle);
 
#ifdef QCC74x_IOT_SDK 
#include <hal_boot2.h>
#include <qcc74x_mtd.h> 
#include <hal_sys.h>
#else
#include <qcc74x_boot2.h>
#include <qcc74x_mtd.h>
#include <qcc74x_ota.h>

#define iot_sha256_context sha256_context
#define HALPartition_Entry_Config qcc74x_partition_config_t
#define hal_reboot qcc74x_sys_reset_por

#define QCC74x_MTD_PARTITION_NAME_FW_DEFAULT QCC74x_MTD_PARTITION_NAME_FW_DEFAULT
#define QCC74x_MTD_OPEN_FLAG_BACKUP QCC74x_MTD_OPEN_FLAG_BACKUP

#define qcc74x_mtd_handle_t qcc74x_mtd_handle_t 
#define qcc74x_mtd_open qcc74x_mtd_open 
#define qcc74x_mtd_write qcc74x_mtd_write
#define qcc74x_mtd_read qcc74x_mtd_read
#define qcc74x_mtd_erase_all qcc74x_mtd_erase_all 
#define qcc74x_mtd_close qcc74x_mtd_close

#define hal_boot2_get_active_entries qcc74x_boot2_get_active_entries
#define hal_boot2_get_active_partition qcc74x_boot2_get_active_partition 
#define hal_boot2_update_ptable qcc74x_boot2_update_ptable
#endif

#endif 
