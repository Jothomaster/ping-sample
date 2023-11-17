#ifndef SIDEWALK_WORKITEMS_H
#define SIDEWALK_WORKITEMS_H

#include <zephyr/kernel.h>

extern struct k_work sidewalk_event, sidewalk_start, sidewalk_conn_request, sidewalk_send_message;

void sidewalk_workqueue_init();

#endif /* SIDEWALK_WORKITEMS_H */