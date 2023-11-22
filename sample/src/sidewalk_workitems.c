#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sidewalk_version.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sidewalk_version.h>

#include <main.h>
#include <sidewalk_callbacks.h>
#include <sidewalk_workitems.h>
#include <queue.h>

#define CREATE_WORKITEM(NAME, ...) static void NAME##_handler(struct k_work* work) __VA_ARGS__ \
                                    K_WORK_DEFINE(NAME, NAME##_handler)

#define PRINT_MACRO(X) #X

LOG_MODULE_REGISTER(work, LOG_LEVEL_DBG);

CREATE_WORKITEM(sidewalk_event, {
    app_ctx_t *app_ctx = CONTAINER_OF(work, app_ctx_t, sidewalk_event);

    sid_process(app_ctx->handle);
});

CREATE_WORKITEM(sidewalk_process_event, {
    app_ctx_t *app_ctx = CONTAINER_OF(work, app_ctx_t, sidewalk_process_event);

    struct sid_status status = { .state = SID_STATE_NOT_READY };
    sid_error_t err = sid_get_status(app_ctx->handle, &status);

    if(app_ctx->message_queue.size == 0) return;

	switch (err) {
	case SID_ERROR_NONE:
		break;
	case SID_ERROR_INVALID_ARGS:
		LOG_ERR("Sidewalk library is not initialzied!");
		return;
	default:
		LOG_ERR("Unknown error during sid_get_status() -> %d", err);
		return;
	}

	if (status.state != SID_STATE_READY && status.state != SID_STATE_SECURE_CHANNEL_READY) {
		k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_conn_request);
		return;
	}
    k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_send_message);
});

CREATE_WORKITEM(sidewalk_send_message, {
    app_ctx_t *app_ctx = CONTAINER_OF(work, app_ctx_t, sidewalk_send_message);

	static struct sid_msg_desc desc;

    struct sid_msg* msg = queue_front(&app_ctx->message_queue);

    if(msg == NULL) return;
    
	desc = (struct sid_msg_desc){
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_ANY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

    sid_error_t err = sid_put_msg(app_ctx->handle, msg, &desc);
	switch (err) {
	case SID_ERROR_NONE: {
		LOG_INF("queued data message id:%d", desc.id);
		break;
	}
	case SID_ERROR_TRY_AGAIN: {
		LOG_ERR("there is no space in the transmit queue, Try again.");
        k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_send_message);
		return;
	}
	default:
		LOG_ERR("Unknown error returned from sid_put_msg() -> %d", err);
        k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_send_message);
        return;
	}

    queue_pop(&app_ctx->message_queue);
    k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_send_message);
});

CREATE_WORKITEM(sidewalk_conn_request, {
    app_ctx_t *app_ctx = CONTAINER_OF(work, app_ctx_t, sidewalk_conn_request);

    struct sid_status status = { .state = SID_STATE_NOT_READY };
    sid_error_t err = sid_get_status(app_ctx->handle, &status);

    switch(err) {
        case SID_ERROR_NONE:
            break;
        case SID_ERROR_INVALID_ARGS:
            LOG_ERR("ERROR_VALUE = %d; LINE: %d", err, __LINE__);
            return;
        default:
            LOG_ERR("ERROR_VALUE = %d; LINE: %d", err, __LINE__);
            return;
    }

	if (status.state != SID_STATE_READY && status.state != SID_STATE_SECURE_CHANNEL_READY) { 
		err = sid_ble_bcn_connection_request(app_ctx->handle, true);
    }

    if(err != SID_ERROR_NONE) {
        LOG_ERR("ERROR_VALUE = %d; LINE: %d", err, __LINE__);
    }
});


CREATE_WORKITEM(sidewalk_start, {
    app_ctx_t *app_ctx = CONTAINER_OF(work, app_ctx_t, sidewalk_start);
    PRINT_SIDEWALK_VERSION();
    app_ctx->config = (struct sid_config){
        .link_mask = SID_LINK_TYPE_1,
        .time_sync_periodicity_seconds = 7200,
        .callbacks = &app_ctx->event_callbacks,
        .link_config = app_get_ble_config(),
        .sub_ghz_link_config = NULL,
    };
    queue_init(&app_ctx->message_queue);
    struct sid_msg msg = (struct sid_msg){
        .data = "Hello World!",
        .size = 12,
    };
    queue_push(&msg, &app_ctx->message_queue);

    sid_error_t err;

    if((err = sid_init(&app_ctx->config, &app_ctx->handle)) != SID_ERROR_NONE){
        LOG_ERR("INITIALIZATION FAILED: %d", err);
        return;
    }
    if((err = sid_start(app_ctx->handle, SID_LINK_TYPE_1)) != SID_ERROR_NONE){
        LOG_ERR("STARTING FAILED: %d", err);
        return;
    }
    LOG_INF("SIDEWALK STARTED %d", err);
});

K_THREAD_STACK_DEFINE(sid_work_q_stack, CONFIG_SID_WORK_Q_STACK_SIZE);

void sidewalk_workqueue_init(app_ctx_t* context){
    context->sidewalk_event = sidewalk_event;
    context->sidewalk_start = sidewalk_start;
    context->sidewalk_conn_request = sidewalk_conn_request;
    context->sidewalk_send_message = sidewalk_send_message;
    context->sidewalk_process_event = sidewalk_process_event;
    k_work_queue_init(&context->sid_q);
    k_work_queue_start(&context->sid_q, sid_work_q_stack, K_THREAD_STACK_SIZEOF(sid_work_q_stack), CONFIG_SID_WORK_Q_PRIORITY, NULL);
    k_work_submit_to_queue(&context->sid_q, &context->sidewalk_start);
}