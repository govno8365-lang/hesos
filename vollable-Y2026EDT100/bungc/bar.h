#ifndef BAR_H
#define BAR_H

#include "gc.h"

void barrier_init(void);
void barrier_pre_write(GCObject **slot, GCObject *old_value);
void barrier_post_write(GCObject **slot, GCObject *new_value);
void barrier_cas(GCObject **slot, GCObject *old_value, GCObject *new_value);
void barrier_enable(int enable);
void barrier_stats(void);
void barrier_cleanup(void);

#endif