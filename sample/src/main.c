#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree/gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <sid_api.h>
#include <app_ble_config.h>
#include <app_subGHz_config.h>
#include <sidewalk_version.h>
#include <pal_init.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

typedef struct application_context {
	struct sid_event_callbacks event_callbacks;
	struct sid_config config;
	struct sid_handle *handle;
	bool connection_request;
} app_ctx_t;

app_ctx_t app_ctx;

struct k_work sidewalk_event;

void sidewalk_work(struct k_work *item){
        LOG_DBG("PROCESSING");
        sid_process(app_ctx.handle);
}

static void on_sidewalk_event(bool in_isr, void *context)
{
	LOG_DBG("on event, from %s, context %p", in_isr ? "ISR" : "App", context);
        k_work_submit_to_queue(&k_sys_work_q, &sidewalk_event);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
    LOG_DBG("received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type,
    (int)msg_desc->link_mode, msg_desc->id, msg->size);
	LOG_HEXDUMP_INF((uint8_t *)msg->data, msg->size, "Message data: ");
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
	LOG_DBG("on message sent");
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
				   void *context)
{
	LOG_DBG("on send error");
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
	LOG_DBG("on status changed");
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

int main(void)
{
        PRINT_SIDEWALK_VERSION();
        k_work_init(&sidewalk_event, sidewalk_work);
        if (application_pal_init()) {
		LOG_ERR("Failed to initialze PAL layer for sidewalk applicaiton.");
		return 0;
	}
        sid_error_t err;
        if((err = sidewalk_callbacks_set(&app_ctx, &app_ctx.event_callbacks)) != SID_ERROR_NONE){
                LOG_ERR("SETTING CALLBACKS FAILED: %d", err);
                return 0;
        }
        app_ctx.config = (struct sid_config){
		.link_mask = SID_LINK_TYPE_1,
		.time_sync_periodicity_seconds = 7200,
		.callbacks = &app_ctx.event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = NULL,
	};
        if((err = sid_init(&app_ctx.config, &app_ctx.handle)) != SID_ERROR_NONE){
                LOG_ERR("INITIALIZATION FAILED: %d", err);
                return 0;
        }
        LOG_INF("SIDEWALK STARTED %d", err);
        return 0;
}