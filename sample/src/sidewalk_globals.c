#include <sidewalk_common.h>
#include <sidewalk_callbacks.h>
#include <sidewalk_workitems.h>

app_ctx_t app_ctx;

struct k_work_q sid_q;

K_THREAD_STACK_DEFINE(sid_work_q_stack, CONFIG_SID_WORK_Q_STACK_SIZE);

struct k_work sidewalk_event;