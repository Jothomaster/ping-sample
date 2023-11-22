#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <sid_api.h>

#include <queue.h>

LOG_MODULE_REGISTER(log_queue, LOG_LEVEL_DBG);

void queue_init(queue* q){
        sys_slist_init(&q->queue_list);
        q->size = 0;
}

static queue_item* get_item(queue* q){
        static int index = 0;
        index %= CONFIG_LOG_QUEUE_SIZE;
        return &q->queue_buffer[index++];
}

int queue_push(const struct sid_msg* message, queue* q){
        if(q->size >= CONFIG_LOG_QUEUE_SIZE){ return 1; }
        queue_item* new_node = get_item(q);
        if(message->size > CONFIG_MSG_SIZE){
                LOG_ERR("MESSAGE TOO LONG TO PLACE, SIZE: %d", message->size);
                return 1;
        }
        memcpy(new_node->msg_data, message->data, message->size);
        new_node->message.data = new_node->msg_data;
        new_node->message.size = message->size;
        sys_slist_append(&q->queue_list, &new_node->l_item);
        q->size++;
        return 0;
}

struct sid_msg* queue_front(queue* q){
        if(q->size == 0) return NULL;
        queue_item* front_item = CONTAINER_OF(sys_slist_peek_head(&q->queue_list), queue_item, l_item);
        return &front_item->message;
}

int queue_pop(queue* q){
        if(q->size <= 0) return 1;
        sys_slist_get(&q->queue_list);
        q->size--;
        return 0;
}