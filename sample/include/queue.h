#ifndef QUEUE_H
#define QUEUE_H

#include <sid_api.h>

typedef struct {
  struct sid_msg message;
  uint8_t msg_data[CONFIG_MSG_SIZE];
  sys_snode_t l_item;
} queue_item;

typedef struct {
  queue_item queue_buffer[CONFIG_LOG_QUEUE_SIZE];
  sys_slist_t queue_list;
  int size;
} queue;

void queue_init(queue* q);
int queue_push(const struct sid_msg* message, queue* q);
struct sid_msg* queue_front(queue* q);
int queue_pop(queue* q);

#endif /* QUEUE_H */