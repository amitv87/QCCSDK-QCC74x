/*
 * power_manager.h
 *
 *  Created on: Sep 20, 2024
 */

#ifndef INC_POWER_MANAGER_H_
#define INC_POWER_MANAGER_H_

#include "cmsis_os2.h"

/* Hooks. */
enum {
	PM_HOOK_PRE_SLEEP = 0,
	PM_HOOK_POST_SLEEP,
    PM_HOOK_NUM,
};

/* Hook priority. */
enum {
    PM_HOOK_PRIO_LOW = 0,
    PM_HOOK_PRIO_MID,
    PM_HOOK_PRIO_HIGH,
    PM_HOOK_PRIO_NUM,
};

/* Power manager events. */
enum {
    PM_EVT_SLEEP_REQ = 0x1,
    PM_EVT_SLEEP_ACK = 0x2,
    PM_EVT_WAKEUP_REQ = 0x4,
    PM_EVT_WAKEUP_ACK = 0x8,
    PM_EVT_EXIT = 0x10,
    PM_EVT_NONE = 0x20,
};

struct power_manager_hook {
    int (*hook)(void *userdata);
    void *userdata;
};

struct power_manager_observer {
    void (*callback)(int event, void *extra, void *userdata);
    void *userdata;
};

struct power_manager_state {
    const char *name;
    uint32_t events;
    int timeout_ms;
    struct power_manager *pm;
    int (*enter)(struct power_manager_state *state);
    int (*exit)(struct power_manager_state *state);
    int (*event_handler)(struct power_manager_state *state, uint32_t evt);
    int (*timeout_handler)(struct power_manager_state *state);
};

#define PM_HOOK_LIMIT 5
#define PM_OBSERVER_NUM 5

struct power_manager {
    /* FSM. */
    struct power_manager_state sm_sleep_req;
    struct power_manager_state sm_sleep_tw;
    struct power_manager_state sm_sleeping;
    struct power_manager_state sm_wakeup_req;
    struct power_manager_state sm_active;
    struct power_manager_state *sm_current;
    int (*request_peer_wakeup)(struct power_manager *pm);
    int (*request_peer_sleep)(struct power_manager *pm);
    /* Task and event handle. */
    int task_stop;
    osThreadId_t tsk_handle;
    /* Event handle. */
    osEventFlagsId_t evt;
    /* Hooks */
    struct power_manager_hook hooks[PM_HOOK_NUM][PM_HOOK_PRIO_NUM][PM_HOOK_LIMIT];
    /* Observers. */
    struct power_manager_observer observers[PM_OBSERVER_NUM];
};

struct power_manager;

struct power_manager_config {
    int (*request_peer_wakeup)(struct power_manager *pm);
    int (*request_peer_sleep)(struct power_manager *pm);
};

typedef struct power_manager *power_manager_t;

power_manager_t power_manager_create(struct power_manager_config *config);
int power_manager_destroy(power_manager_t pm);
int power_manager_request_sleep(power_manager_t pm);
int power_manager_request_wakeup(power_manager_t pm);
int power_manager_handle_sleep_ack(struct power_manager *pm);
int power_manager_handle_wakeup_ack(struct power_manager *pm);
int power_manager_register_hook(power_manager_t pm, int hook_num,
        int prio, struct power_manager_hook *hook);
int power_manager_unregister_hook(power_manager_t pm, int hook_num,
        int prio, struct power_manager_hook *hook);

#endif /* INC_POWER_MANAGER_H_ */
