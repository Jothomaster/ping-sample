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
#include <sidewalk_callbacks.h>
#include <pal_init.h>

#define  BTN0_NODE DT_ALIAS(sw0)
#define MSG_SIZE 256
#define LOG_QUEUE_SIZE 20

static struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
struct gpio_callback btn_callback;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

typedef struct{
        char queue_array[LOG_QUEUE_SIZE][MSG_SIZE];
        int head, tail;
        int size;
} queue;

typedef struct{
        struct k_work work;
        queue message_queue;
} log_queue;

void queue_init(queue* q){
        q->head = 0;
        q->tail = -1;
        q->size = 0;
}

int queue_push(const char* str, queue* q){
        if(q->size >= LOG_QUEUE_SIZE){ return 1; }
        q->tail = (q->tail + 1) % LOG_QUEUE_SIZE;
        strncpy(q->queue_array[q->tail], str, MSG_SIZE);
        q->size++;
        return 0;
}

char* queue_front(queue* q){
        if(q->size == 0) return NULL;
        return q->queue_array[q->head];
}

int queue_pop(queue* q){
        if(q->size <= 0) return 1;
        q->head = (q->head + 1) % LOG_QUEUE_SIZE;
        q->size--;
        return 0;
}

void log_queue_add(const char* str, log_queue* q){
        if(queue_push(str, &q->message_queue)){
                LOG_ERR("LOG QUEUE FULL\n");
        }
}


void print_msg(struct k_work *item)
{
        log_queue *q = CONTAINER_OF(item, log_queue, work);
        if(queue_front(&q->message_queue) != NULL){
                LOG_DBG("%s\n", queue_front(&q->message_queue));
                queue_pop(&q->message_queue);
                k_work_submit_to_queue(&k_sys_work_q, &q->work);
        }
}

log_queue main_log_queue;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	for(int i=1; i<= 5; i++){
                char msg[256];
                sprintf(msg, "Hello no. %d", i);
                log_queue_add(msg, &main_log_queue);              
        }
        k_work_submit_to_queue(&k_sys_work_q, &main_log_queue.work);
}

struct sid_event_callbacks event_callbacks;

struct sid_config default_config = {
        .link_mask = SID_LINK_TYPE_1,
        .time_sync_periodicity_seconds = 7200,
        .callbacks = &event_callbacks,
};

static struct sid_handle *handle;

int main(void)
{
        PRINT_SIDEWALK_VERSION();
        if (application_pal_init()) {
		LOG_ERR("Failed to initialze PAL layer for sidewalk applicaiton.");
		//application_state_error(&global_state_notifier, true);
		return 0;
	}
        default_config.link_config = app_get_ble_config();
        //default_config.sub_ghz_link_config = app_get_sub_ghz_config();
        sid_error_t err;
        if((err = sidewalk_callbacks_set(NULL, &event_callbacks)) != SID_ERROR_NONE){
                LOG_ERR("SETTING CALLBACKS FAILED: %d", err);
                return 0;
        }
        if((err = sid_init(&default_config, &handle)) != SID_ERROR_NONE){
                LOG_ERR("INITIALIZATION FAILED: %d", err);
                return 0;
        }
        int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}
        queue_init(&main_log_queue.message_queue);
	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
        gpio_init_callback(&btn_callback, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &btn_callback);
        k_work_init(&main_log_queue.work, print_msg);
        return 0;
}
