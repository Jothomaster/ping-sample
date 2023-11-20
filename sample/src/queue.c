#include <zephyr/kernel.h>

#include <sid_api.h>

#include <queue.h>

void queue_init(queue* q){
        q->head = 0;
        q->tail = -1;
        q->size = 0;
        for(int i=0; i<LOG_QUEUE_SIZE; i++){
            q->queue_array[i].data = q->msg_buffer[i];
        }
}

int queue_push(const struct sid_msg* message, queue* q){
        if(q->size >= LOG_QUEUE_SIZE){ return 1; }
        q->tail = (q->tail + 1) % LOG_QUEUE_SIZE;
        memcpy(q->queue_array[q->tail].data, message->data, message->size);
        q->queue_array[q->tail].size = message->size;
        q->size++;
        return 0;
}

struct sid_msg* queue_front(queue* q){
        if(q->size == 0) return NULL;
        return &q->queue_array[q->head];
}

int queue_pop(queue* q){
        if(q->size <= 0) return 1;
        q->head = (q->head + 1) % LOG_QUEUE_SIZE;
        q->size--;
        return 0;
}