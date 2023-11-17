#ifndef SIDEWALK_COMMON_H
#define SIDEWALK_COMMON_H

#include <zephyr/kernel.h>

#include <sid_api.h>
#include <queue.h>

typedef struct application_context {
	struct sid_event_callbacks event_callbacks;
	struct sid_config config;
	struct sid_handle *handle;
	queue message_queue;
	int8_t registered;
} app_ctx_t; 

extern app_ctx_t app_ctx;

extern struct k_work_q sid_q;
K_THREAD_STACK_DECLARE(sid_work_q_stack, CONFIG_SID_WORK_Q_STACK_SIZE);

#endif /* SIDEWALK_COMMON_H */