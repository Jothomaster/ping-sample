#ifndef SIDEWALK_COMMON_H
#define SIDEWALK_COMMON_H

#include <stdbool.h>

#include <zephyr/kernel.h>

#include <sid_api.h>
#include <queue.h>

typedef struct application_context {
	struct sid_event_callbacks event_callbacks;
	struct sid_config config;
	struct sid_handle *handle;
	queue message_queue;
	bool is_init;
} app_ctx_t; 

extern app_ctx_t app_ctx;

extern struct k_work_q sid_q;

#endif /* SIDEWALK_COMMON_H */