#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree/gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
// This value may be user configurable, it would be best siuted in Kconfig of the application
#define  MSG_SIZE 256
#define  BTN0_NODE DT_ALIAS(sw0)

//static
struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
//static
struct gpio_callback btn_callback;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

typedef struct litem litem;

// nice exercise, but zephyr has build in list structures 
// resources to read: https://docs.zephyrproject.org/latest/kernel/data_structures/slist.html
struct litem{
        litem * next;
        char text[256]; // You have define for that, do not use magic numbers.
};

typedef struct{
        struct k_work work;
        litem *head, *tail; // it is valid, but uncommon to define pointers like that. it would be better to create them in separate lines  (it is easy to omit one * and create an object instead of pointer)
} log_queue;

void log_queue_add(const char* str, log_queue* q){
        
        //  in embedded world malloc should be avoided if possible, we do not have so much RAM, and it is real possibility that malloc will fail to allocate next block. Furtheremore You do not handle its error, so it is possible that with next line You are now writing on stack, or other structures in heap, therefore corrupting the state of application.
        litem* it = k_malloc(sizeof(litem));
        strcpy(it->text, str); // strcpy is not safe function, you should cosider strncpy, it will coppy N bytes, unlike strcpy that will look for '\0' characterm and given corrupted input, it can write arbitrary amount of data, therefore overwriting data outside of the allocated array.
        it->next = NULL;
        if(q->head == NULL){ // ok, what if q is NULL  ?
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
              k_msleep(200);   // why sleep here ?
        }
        // commented code commited on main ? How did it pass the review? ( I'm joking I know how it got here, we all make mistakes, but we should avoid commiting commented code. The only exception is in the documentation comments where You show how to use the function that is being documented.)
        //k_free(msg);
        //LOG_DBG("%d",k_work_busy_get(item)&K_WORK_RUNNING);
}
// what is that ? we shouldn't name variables with single letter, it is harder to debug and refactor. (If it was called `log_queue_object` it would be easy to change its name, just use find and replace, thy this with variable 'q', and You will soon find out that You have issues with undefined references to functions that had letter 'q' in their name)
// This should be static, as it is private field of this module
log_queue q;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	for(int i=1; i<= 50; i++){
                char msg[256]; // again, there is macro for that
                // this callback may have a flaw that could be avoided if you didn't use malloc
                // in this line you coppy some data to the huge array stored on stack of this function
                // next line will allocate the new buffor, and copy this data again. so you have 3 heavy operations, 2 coppy, 1 allocation
                // This will take time, and now you are in the interrupt, your application is not running, it dropped everything to handle this interrupt.
                // and with those two lines You are holding the application for longer than it is necessary.

                //This is precisely why we use workq, You have a heavy operation to execute on some callback, so you delegate this heavy operation to a callback to exit from IRQ as fast as possible
                sprintf(msg, "Hello no. %d", i);
                // zephyr logs may be executed in IRQ, but I understand it is an exercise.
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


// Overall very good, It is nice that You tried to use workq and implement a list, I hope it was good learning experiance.
// I do not see any issues with the use of workq.

// This is a nice exercise, altho I probably would chose the last assingment with creating the binary count on LEDs to show the workq in action ( the same implementatio, but in stead of printing logs increment LED counter inside the worker), but this will also do. 
// overall I'm pleased with the code that has been created, it shows that you are capable to succed with the final project.