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

#include <sidewalk_common.h>
#include <sidewalk_callbacks.h>
#include <sidewalk_workitems.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void send_req(void) {
    struct sid_status status = { .state = SID_STATE_NOT_READY };
    sid_error_t err = sid_get_status(app_ctx.handle, &status);

    switch(err) {
        case SID_ERROR_NONE:
            break;
        case SID_ERROR_INVALID_ARGS:
            LOG_ERR("ERROR_VALUE = %d", err);
            return;
        default:
            LOG_ERR("ERROR_VALUE = %d", err);
            return;
    }

	if (status.state != SID_STATE_READY && status.state != SID_STATE_SECURE_CHANNEL_READY) { 
		err = sid_ble_bcn_connection_request(app_ctx.handle, true);
        LOG_WRN("Satus state is invalid: %d", status.state);
	}

    if(err != SID_ERROR_NONE) {
        LOG_ERR("ERROR_VALUE = %d", err);
    }
}

void send_mess(void) {
    struct sid_status status = { .state = SID_STATE_NOT_READY };
    sid_error_t err = sid_get_status(app_ctx.handle, &status);

    static struct sid_msg msg;
	static struct sid_msg_desc desc;

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
		LOG_ERR("Sidewalk Status is invalid!, expected SID_STATE_READY or SID_STATE_SECURE_CHANNEL_READY, got %d",
			status.state);
		return;
	}    

    msg = (struct sid_msg){ .data = (char*)"Hello World!" , .size = sizeof(char)*13 };
	desc = (struct sid_msg_desc){
		.type = SID_MSG_TYPE_NOTIFY,
		.link_type = SID_LINK_TYPE_ANY,
		.link_mode = SID_LINK_MODE_CLOUD,
	};

    err = sid_put_msg(app_ctx.handle, &msg, &desc);
	switch (err) {
	case SID_ERROR_NONE: {
		//application_state_sending(&global_state_notifier, true);
		LOG_INF("queued data message id:%d", desc.id);
		break;
	}
	case SID_ERROR_TRY_AGAIN: {
		LOG_ERR("there is no space in the transmit queue, Try again.");
		break;
	}
	default:
		LOG_ERR("Unknown error returned from sid_put_msg() -> %d", err);
	}
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
    static bool reg = false;
	LOG_DBG("on status changed: %d", status->state);
    if(status->state == 1 && !reg) {reg = !reg; return;};
    if(status->state) send_req(); else send_mess();
}


int main(void)
{
	sidewalk_workqueue_init();
    PRINT_SIDEWALK_VERSION();
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
    if((err = sid_start(app_ctx.handle, SID_LINK_TYPE_1)) != SID_ERROR_NONE){
        LOG_ERR("STARTING FAILED: %d", err);
        return 0;
    }
    LOG_INF("SIDEWALK STARTED %d", err);
    return 0;
}