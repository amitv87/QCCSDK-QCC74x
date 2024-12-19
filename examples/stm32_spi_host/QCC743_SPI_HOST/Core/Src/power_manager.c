/*
 * power_manager.c
 *
 *  Created on: Sep 20, 2024
 */

#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "power_manager.h"

struct power_manager_state_config {
    const char *name;
    uint32_t events;
    int timeout_ms;
    struct power_manager *pm;
};

int power_manager_register_hook(struct power_manager *pm, int hook_num,
        int prio, struct power_manager_hook *hook)
{
    int i;
    struct power_manager_hook *hooks;

    if (!pm || !hook || !hook->hook)
        return -1;
    if (hook_num < 0 || hook_num >= PM_HOOK_NUM)
        return -1;
    if (prio < 0 || prio >= PM_HOOK_PRIO_NUM)
        return -1;

    hooks = pm->hooks[hook_num][prio];
    for (i = 0; i < PM_HOOK_LIMIT; i++) {
        if (!hooks[i].hook) {
            hooks[i] = *hook;
            return 0;
        }
    }
    return -2;
}

int power_manager_unregister_hook(struct power_manager *pm, int hook_num,
        int prio, struct power_manager_hook *hook)
{
    int i;
    struct power_manager_hook *hooks;

    if (!pm || !hook || !hook->hook)
        return -1;
    if (hook_num < 0 || hook_num >= PM_HOOK_NUM)
        return -1;
    if (prio < 0 || prio >= PM_HOOK_PRIO_NUM)
        return -1;

    hooks = pm->hooks[hook_num][prio];
    for (i = 0; i < PM_HOOK_LIMIT; i++) {
        if (hooks[i].hook == hook->hook &&
            hooks[i].userdata == hook->userdata) {
            hooks[i].hook = NULL;
            hooks[i].userdata = NULL;
            return 0;
        }
    }
    return -2;
}

static int power_manager_exec_hook(struct power_manager *pm, int hook_num)
{
    int prio;
    struct power_manager_hook (*hooks)[PM_HOOK_LIMIT];

    hooks = pm->hooks[hook_num];
    for (prio = PM_HOOK_PRIO_LOW; prio < PM_HOOK_PRIO_NUM; prio++) {
        for (int i = 0; i < PM_HOOK_LIMIT; i++) {
            struct power_manager_hook *hook = &hooks[prio][i];

            if (hook->hook)
                hook->hook(hook->userdata);
        }
    }
    return 0;
}

static void power_manager_state_init(struct power_manager_state *state,
        struct power_manager_state_config *config)
{
    state->name = config->name;
    state->events = config->events;
    state->timeout_ms = config->timeout_ms;
    state->pm = config->pm;
}

static inline void pm_fsm_transition(struct power_manager *pm,
                                    struct power_manager_state *next)
{
    struct power_manager_state *curr = pm->sm_current;

    if (curr != next) {
        printf("power manager state from %s to %s\r\n",
        		curr ? curr->name : "NONE", next->name);
        if (curr && curr->exit)
            curr->exit(curr);
        pm->sm_current = next;
        if (next->enter)
            next->enter(next);
    }
}

static int pm_active_event_handler(struct power_manager_state *state, uint32_t evt)
{
    struct power_manager *pm = state->pm;

    if (evt & PM_EVT_SLEEP_REQ)
        pm_fsm_transition(pm, &pm->sm_sleep_req);
    return 0;
}

static int pm_active_on_enter(struct power_manager_state *state)
{
    /* TODO notify observers that we are active now. */
    return 0;
}

static int pm_sleep_req_event_handler(struct power_manager_state *state,
                                        uint32_t evt)
{
    struct power_manager *pm = state->pm;

    if (evt & PM_EVT_SLEEP_ACK)
        pm_fsm_transition(pm, &pm->sm_sleep_tw);
    return 0;
}

static int pm_sleep_req_timeout_handler(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    return pm->request_peer_sleep(pm);
}

static int pm_sleep_req_on_enter(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    return pm->request_peer_sleep(pm);
}

static int pm_sleep_tw_timeout_handler(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    pm_fsm_transition(pm, &pm->sm_sleeping);
    return 0;
}

static int pm_sleeping_event_handler(struct power_manager_state *state,
                                        uint32_t evt)
{
    struct power_manager *pm = state->pm;

    if (evt & PM_EVT_WAKEUP_REQ)
        pm_fsm_transition(pm, &pm->sm_wakeup_req);
    return 0;
}

static int pm_sleeping_on_enter(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    power_manager_exec_hook(pm, PM_HOOK_PRE_SLEEP);
    return 0;
}

static int pm_sleeping_on_exit(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    power_manager_exec_hook(pm, PM_HOOK_POST_SLEEP);
    return 0;
}

static int pm_wakeup_req_event_handler(struct power_manager_state *state,
                                        uint32_t evt)
{
    struct power_manager *pm = state->pm;

    if (evt & PM_EVT_WAKEUP_ACK)
        pm_fsm_transition(pm, &pm->sm_active);
    return 0;
}

static int pm_wakeup_req_timeout_handler(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    pm->request_peer_wakeup(pm);
    return 0;
}

static int pm_wakeup_req_on_enter(struct power_manager_state *state)
{
    struct power_manager *pm = state->pm;

    return pm->request_peer_wakeup(pm);
}

static int power_manager_state_machine_init(struct power_manager *pm)
{
    struct power_manager_state_config config;

    /* Active state. */
    config.pm = pm;
    config.name = "active";
    config.events = PM_EVT_SLEEP_REQ;
    config.timeout_ms = 500;
    power_manager_state_init(&pm->sm_active, &config);
    pm->sm_active.event_handler = pm_active_event_handler;
    pm->sm_active.timeout_handler = NULL;
    pm->sm_active.enter = pm_active_on_enter;
    pm->sm_active.exit = NULL;

    /* Sleep request. */
    config.pm = pm;
    config.name = "sleep_req";
    config.events = PM_EVT_SLEEP_ACK;
    config.timeout_ms = 300;
    power_manager_state_init(&pm->sm_sleep_req, &config);
    pm->sm_sleep_req.event_handler = pm_sleep_req_event_handler;
    pm->sm_sleep_req.timeout_handler = pm_sleep_req_timeout_handler;
    pm->sm_sleep_req.enter = pm_sleep_req_on_enter;
    pm->sm_sleep_req.exit = NULL;

    /* Sleep time-wait */
    config.pm = pm;
    config.name = "sleep_tw";
    config.events = PM_EVT_NONE;
    config.timeout_ms = 300;
    power_manager_state_init(&pm->sm_sleep_tw, &config);
    pm->sm_sleep_tw.event_handler = NULL;
    pm->sm_sleep_tw.timeout_handler = pm_sleep_tw_timeout_handler;
    pm->sm_sleep_tw.enter = NULL;
    pm->sm_sleep_tw.exit = NULL;

    /* Sleeping. */
    config.pm = pm;
    config.name = "sleeping";
    config.events = PM_EVT_WAKEUP_REQ;
    config.timeout_ms = 500;
    power_manager_state_init(&pm->sm_sleeping, &config);
    pm->sm_sleeping.event_handler = pm_sleeping_event_handler;
    pm->sm_sleeping.timeout_handler = NULL;
    pm->sm_sleeping.enter = pm_sleeping_on_enter;
    pm->sm_sleeping.exit = pm_sleeping_on_exit;

    /* Wakeup request. */
    config.pm = pm;
    config.name = "wakeup_req";
    config.events = PM_EVT_WAKEUP_ACK;
    config.timeout_ms = 500;
    power_manager_state_init(&pm->sm_wakeup_req, &config);
    pm->sm_wakeup_req.event_handler = pm_wakeup_req_event_handler;
    pm->sm_wakeup_req.timeout_handler = pm_wakeup_req_timeout_handler;
    pm->sm_wakeup_req.enter = pm_wakeup_req_on_enter;
    pm->sm_wakeup_req.exit = NULL;

    /* Start from active state. */
    pm->sm_current = NULL;
    pm_fsm_transition(pm, &pm->sm_active);
    return 0;
}

static void power_manager_task(void *arg)
{
    uint32_t ret;
    struct power_manager *pm = arg;
    struct power_manager_state *state;

    while (!pm->task_stop) {
        state = pm->sm_current;
        ret = osEventFlagsWait(pm->evt, state->events, osFlagsWaitAny, state->timeout_ms);
        if (ret & (1UL << 31)) {
        	if (ret == osErrorTimeout) {
        		if (state->timeout_handler)
        			state->timeout_handler(state);
        	} else {
        		printf("event wait failed, %lx\r\n", ret);
        	}
        } else {
        	if (state->event_handler)
        		state->event_handler(state, ret);
        }
    }
}

struct power_manager *power_manager_create(struct power_manager_config *config)
{
    struct power_manager *pm;
    osThreadAttr_t pm_tsk_attr = {
    		.name = "power_manager",
    		.priority = osPriorityRealtime,
    		.stack_size = 1024 * 4,
    };

    if (!config || !config->request_peer_wakeup || !config->request_peer_sleep)
    	return NULL;

    pm = pvPortMalloc(sizeof(*pm));
    if (!pm) {
    	printf("No mem for power manager\r\n");
    	return NULL;
    }
    pm->request_peer_sleep = config->request_peer_sleep;
    pm->request_peer_wakeup = config->request_peer_wakeup;
    /* Initialize finite state machine. */
    power_manager_state_machine_init(pm);
    /* Create event handle. */
    pm->evt = osEventFlagsNew(NULL);
    if (!pm->evt) {
    	printf("Failed to create power manager event\r\n");
    	goto err_evt;
    }
    /* Start its thread. */
    pm->task_stop = 0;
    pm->tsk_handle = osThreadNew(power_manager_task, pm, &pm_tsk_attr);
    if (!pm->tsk_handle) {
    	printf("Failed to create power manager task\r\n");
    	goto err_task;
    }
    return pm;

err_task:
	osEventFlagsDelete(pm->evt);
err_evt:
	vPortFree(pm);
	return NULL;
}

int power_manager_destroy(struct power_manager *pm)
{
	if (!pm)
		return -1;

	if (pm->tsk_handle) {
		pm->task_stop = 1;
		osEventFlagsSet(pm->evt, PM_EVT_EXIT);
		osThreadTerminate(pm->tsk_handle);
	}
	if (pm->evt) {
		osEventFlagsDelete(pm->evt);
		pm->evt = NULL;
	}
    return 0;
}

int power_manager_request_sleep(struct power_manager *pm)
{
    /* Sanity check. */
    if (!pm)
        return -1;

    /* Sleep request is only accepted when the power manager is active. */
    if (pm->sm_current != &pm->sm_active)
        return -2;

    osEventFlagsSet(pm->evt, PM_EVT_SLEEP_REQ);
    return 0;
}

int power_manager_request_wakeup(struct power_manager *pm)
{
    /* Sanity check. */
    if (!pm)
        return -1;

    /* Wakeup request is only accepted when the power manager is sleeping. */
    if (pm->sm_current != &pm->sm_sleeping)
        return -2;

    osEventFlagsSet(pm->evt, PM_EVT_WAKEUP_REQ);
    return 0;
}

int power_manager_handle_sleep_ack(struct power_manager *pm)
{
	if (!pm)
		return -1;

	osEventFlagsSet(pm->evt, PM_EVT_SLEEP_ACK);
	return 0;
}

int power_manager_handle_wakeup_ack(struct power_manager *pm)
{
	if (!pm)
		return -1;

	osEventFlagsSet(pm->evt, PM_EVT_WAKEUP_ACK);
	return 0;
}
