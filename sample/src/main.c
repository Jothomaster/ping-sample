#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree/gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#define  MSG_SIZE 256
#define  BTN0_NODE DT_ALIAS(sw0)

struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
struct gpio_callback btn_callback;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

typedef struct litem litem;

struct litem{
        litem * next;
        char text[256];
};

typedef struct{
        struct k_work work;
        litem *head, *tail;
} log_queue;

void log_queue_add(const char* str, log_queue* q){
        litem* it = k_malloc(sizeof(litem));
        strcpy(it->text, str);
        it->next = NULL;
        if(q->head == NULL){
                q->head = it;
                q->tail = it;
                return;
        }
        q->tail->next = it;
        q->tail = it;
}


void print_msg(struct k_work *item)
{
        log_queue *q = CONTAINER_OF(item, log_queue, work);
        while (q->head != NULL)
        {
              LOG_DBG("%s",q->head->text);
              litem *tmp = q->head;
              q->head = q->head->next;
              k_free(tmp);
              k_msleep(200);  
        }
        //k_free(msg);
        //LOG_DBG("%d",k_work_busy_get(item)&K_WORK_RUNNING);
}

log_queue q;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	for(int i=1; i<= 50; i++){
                char msg[256];
                sprintf(msg, "Hello no. %d", i);
                log_queue_add(msg, &q);              
        }
        k_work_submit_to_queue(&k_sys_work_q, &q.work);
}

int main(void)
{
        int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
        gpio_init_callback(&btn_callback, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &btn_callback);
        k_work_init(&q.work, print_msg);
        return 0;
}
