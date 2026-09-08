#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bar.h"
#include "gc.h"

typedef struct {
    int enabled;
    size_t dirty_count;
    size_t dirty_capacity;
    GCObject **dirty_list;
    size_t pre_writes;
    size_t post_writes;
    size_t cas_operations;
} BarrierState;

static BarrierState barrier_state = {0};

void barrier_init(void) {
    barrier_state.enabled = 1;
    barrier_state.dirty_count = 0;
    barrier_state.dirty_capacity = 1024;
    barrier_state.dirty_list = malloc(sizeof(GCObject*) * barrier_state.dirty_capacity);
    barrier_state.pre_writes = 0;
    barrier_state.post_writes = 0;
    barrier_state.cas_operations = 0;

    if (!barrier_state.dirty_list) {
        fprintf(stderr, "[BARRIER] Failed to allocate dirty list\n");
        exit(1);
    }
}

static void barrier_add_dirty(GCObject *obj) {
    if (!obj || !barrier_state.enabled) return;

    for (size_t i = 0; i < barrier_state.dirty_count; i++) {
        if (barrier_state.dirty_list[i] == obj) return;
    }

    if (barrier_state.dirty_count >= barrier_state.dirty_capacity) {
        barrier_state.dirty_capacity *= 2;
        barrier_state.dirty_list = realloc(barrier_state.dirty_list,
                                           sizeof(GCObject*) * barrier_state.dirty_capacity);
        if (!barrier_state.dirty_list) {
            fprintf(stderr, "[BARRIER] Failed to expand dirty list\n");
            exit(1);
        }
    }

    barrier_state.dirty_list[barrier_state.dirty_count++] = obj;
}

void barrier_pre_write(GCObject **slot, GCObject *old_value) {
    if (!barrier_state.enabled || !slot) return;
    barrier_state.pre_writes++;
    if (old_value) {
        barrier_add_dirty(old_value);
    }
}

void barrier_post_write(GCObject **slot, GCObject *new_value) {
    if (!barrier_state.enabled || !slot) return;
    barrier_state.post_writes++;
    if (new_value) {
        barrier_add_dirty(new_value);
    }
}

void barrier_cas(GCObject **slot, GCObject *old_value, GCObject *new_value) {
    if (!barrier_state.enabled || !slot) return;
    barrier_state.cas_operations++;
    barrier_pre_write(slot, old_value);
    barrier_post_write(slot, new_value);
}

void barrier_enable(int enable) {
    barrier_state.enabled = enable;
}

void barrier_stats(void) {
    printf("\n=== WRITE BARRIER STATISTICS ===\n");
    printf("Pre-writes:        %zu\n", barrier_state.pre_writes);
    printf("Post-writes:       %zu\n", barrier_state.post_writes);
    printf("CAS operations:    %zu\n", barrier_state.cas_operations);
    printf("Dirty list size:   %zu / %zu\n", barrier_state.dirty_count, barrier_state.dirty_capacity);
    printf("Barriers enabled:  %s\n", barrier_state.enabled ? "YES" : "NO");
    printf("================================\n");
}

void barrier_cleanup(void) {
    if (barrier_state.dirty_list) {
        free(barrier_state.dirty_list);
        barrier_state.dirty_list = NULL;
    }
}