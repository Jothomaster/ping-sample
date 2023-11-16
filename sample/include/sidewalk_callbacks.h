#ifndef SIDEWALK_CALLBACKS_H
#define SIDEWALK_CALLBACKS_H

#include <sid_api.h>

sid_error_t sidewalk_callbacks_set(void *context, struct sid_event_callbacks *callbacks);

#endif /* SIDEWALK_CALLBACKS_H */