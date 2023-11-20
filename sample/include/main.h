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
	struct k_work* sidewalk_event;
	struct k_work* sidewalk_start;
	struct k_work* sidewalk_conn_request;
	struct k_work* sidewalk_send_message;
	struct k_work* sidewalk_process_event;
	struct k_work_q sid_q;
	bool is_init;
} app_ctx_t; 

#endif /* SIDEWALK_COMMON_H */