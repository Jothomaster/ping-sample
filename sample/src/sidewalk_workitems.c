#include <zephyr/kernel.h>

#include <sidewalk_common.h>
#include <sidewalk_workitems.h>

static void sidewalk_work(struct k_work *item){
    sid_process(app_ctx.handle);
}

void sidewalk_workqueue_init(){
    k_work_queue_init(&sid_q);
    k_work_queue_start(&sid_q, sid_work_q_stack, K_THREAD_STACK_SIZEOF(sid_work_q_stack), CONFIG_SID_WORK_Q_PRIORITY, NULL);
    k_work_init(&sidewalk_event, sidewalk_work);
}