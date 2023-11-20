#ifndef QUEUE_H
#define QUEUE_H

#include <sid_api.h>

#define MSG_SIZE 256
#define LOG_QUEUE_SIZE 20

typedef struct{
        struct sid_msg queue_array[LOG_QUEUE_SIZE];
        int head, tail;
        int size;
        uint8_t msg_buffer[LOG_QUEUE_SIZE][MSG_SIZE];
} queue;

void queue_init(queue* q);
int queue_push(const struct sid_msg* message, queue* q);
struct sid_msg* queue_front(queue* q);
int queue_pop(queue* q);

#endif /* QUEUE_H */