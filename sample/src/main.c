#include <sidewalk_workitems.h>
#include <main.h>

static app_ctx_t main_ctx;

int main(void)
{
	sidewalk_workqueue_init(&main_ctx);
    return 0;
}