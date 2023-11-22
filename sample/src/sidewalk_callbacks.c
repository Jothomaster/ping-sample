#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sid_api.h>

#include <main.h>
#include <sidewalk_workitems.h>
#include <queue.h>

LOG_MODULE_REGISTER(sid_callbacks, LOG_LEVEL_DBG);

static void on_sidewalk_event(bool in_isr, void *context)
{
	app_ctx_t* app_ctx = (app_ctx_t*) context;
    k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_event);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
	app_ctx_t* app_ctx = (app_ctx_t*) context;
    LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", 
    (int)msg_desc->type, (int)msg_desc->link_mode, msg_desc->id, msg->size);
    LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
    queue_push(msg, &app_ctx->message_queue);
	
    k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_process_event);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("on message sent");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
	app_ctx_t* app_ctx = (app_ctx_t*) context;
	LOG_DBG("on send error");
	k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_process_event);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	app_ctx_t* app_ctx = (app_ctx_t*) context;
	LOG_DBG("on status changed: %d", status->state);
    if(status->state == 0 && !app_ctx->is_init){
        app_ctx->is_init = true;
        sid_ble_bcn_connection_request(app_ctx->handle, false);
        return;
    }
    if(!app_ctx->is_init) { return; }
    k_work_submit_to_queue(&app_ctx->sid_q, &app_ctx->sidewalk_process_event);
}

static void on_sidewalk_factory_reset(void *context)
{
	LOG_DBG("on factory reset");
}

sid_error_t sidewalk_callbacks_set(void *context, struct sid_event_callbacks *callbacks)
{
	if (!callbacks) {
		return SID_ERROR_INVALID_ARGS;
	}
	callbacks->context = context;
	callbacks->on_event = on_sidewalk_event;
	callbacks->on_msg_received = on_sidewalk_msg_received; /* Called from sid_process() */
	callbacks->on_msg_sent = on_sidewalk_msg_sent; /* Called from sid_process() */
	callbacks->on_send_error = on_sidewalk_send_error; /* Called from sid_process() */
	callbacks->on_status_changed = on_sidewalk_status_changed; /* Called from sid_process() */
	callbacks->on_factory_reset = on_sidewalk_factory_reset; /* Called from sid_process() */

	return SID_ERROR_NONE;
}