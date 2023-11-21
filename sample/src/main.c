#include <sidewalk_workitems.h>
#include <sidewalk_callbacks.h>
#include <main.h>
#include <zephyr/logging/log.h>

#include <pal_init.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static app_ctx_t main_ctx;

int main(void)
{
    main_ctx.is_init = false;
    if (application_pal_init()) {
        LOG_ERR("Failed to initialze PAL layer for sidewalk applicaiton.");
        return 0;
    }
    sid_error_t err;
    if((err = sidewalk_callbacks_set(&main_ctx, &main_ctx.event_callbacks)) != SID_ERROR_NONE){
        LOG_ERR("SETTING CALLBACKS FAILED: %d", err);
        return 0;
    }
	sidewalk_workqueue_init(&main_ctx);
    return 0;
}