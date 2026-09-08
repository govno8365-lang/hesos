#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"
#include "bar.h"

GCState gc_state = {0};

void gc_init(size_t threshold) {
    gc_state.objects = NULL;
    gc_state.worklist = NULL;
    gc_state.roots = NULL;
    gc_state.total_allocated = 0;
    gc_state.threshold = threshold > 0 ? threshold : (1024 * 1024);
    gc_state.allocations_since_gc = 0;
    gc_state.enabled = 1;
    gc_state.running = 0;
    gc_state.pause_requested = 0;
    barrier_init();
}

GCObject* gc_alloc(ObjectType type, size_t size) {
    if (!gc_state.enabled) {
        GCObject *obj = malloc(sizeof(GCObject) + size);
        if (!obj) return NULL;
        obj->type = type;
        obj->size = size;
        obj->marked = 0;
        obj->visited = 0;
        obj->data = (void*)((char*)obj + sizeof(GCObject));
        return obj;
    }

    GCObject *obj = malloc(sizeof(GCObject) + size);
    if (!obj) {
        gc_collect();
        obj = malloc(sizeof(GCObject) + size);
        if (!obj) return NULL;
    }

    obj->type = type;
    obj->size = size;
    obj->marked = 0;
    obj->visited = 0;
    obj->data = (void*)((char*)obj + sizeof(GCObject));
    obj->next = gc_state.objects;
    obj->next_work = NULL;

    gc_state.objects = obj;
    gc_state.total_allocated += size;
    gc_state.allocations_since_gc += size;

    gc_maybe_collect();

    return obj;
}

void gc_add_root(GCObject **ptr) {
    GCRoot *root = malloc(sizeof(GCRoot));
    root->ptr = ptr;
    root->next = gc_state.roots;
    gc_state.roots = root;
}

void gc_remove_root(GCObject **ptr) {
    GCRoot *prev = NULL;
    GCRoot *cur = gc_state.roots;
    while (cur) {
        if (cur->ptr == ptr) {
            if (prev) prev->next = cur->next;
            else gc_state.roots = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static void gc_mark(GCObject *obj) {
    if (!obj || obj->marked) return;
    obj->marked = 1;
    obj->next_work = gc_state.worklist;
    gc_state.worklist = obj;
}

static void gc_mark_roots(void) {
    GCRoot *root = gc_state.roots;
    while (root) {
        if (root->ptr && *root->ptr) {
            gc_mark(*root->ptr);
        }
        root = root->next;
    }
}

static void gc_sweep(void) {
    GCObject *prev = NULL;
    GCObject *cur = gc_state.objects;

    while (cur) {
        GCObject *next = cur->next;
        if (!cur->marked) {
            free(cur);
            if (prev) prev->next = next;
            else gc_state.objects = next;
        } else {
            cur->marked = 0;
            prev = cur;
        }
        cur = next;
    }
}

void gc_step(void) {
    if (!gc_state.enabled) return;

    if (gc_state.worklist) {
        GCObject *obj = gc_state.worklist;
        gc_state.worklist = obj->next_work;
        return;
    }

    if (!gc_state.running) {
        gc_state.running = 1;
        gc_mark_roots();
    } else {
        gc_sweep();
        gc_state.running = 0;
        gc_state.allocations_since_gc = 0;
    }
}

void gc_collect(void) {
    if (!gc_state.enabled) return;

    barrier_enable(0);

    while (gc_state.worklist || gc_state.running) {
        gc_step();
    }

    gc_state.running = 1;
    gc_mark_roots();

    while (gc_state.worklist) {
        gc_step();
    }

    gc_sweep();
    gc_state.running = 0;
    gc_state.allocations_since_gc = 0;

    barrier_enable(1);
}

void gc_maybe_collect(void) {
    if (!gc_state.enabled) return;
    if (gc_state.total_allocated > gc_state.threshold) {
        gc_step();
    }
}

void gc_enable(int enable) {
    gc_state.enabled = enable;
}

size_t gc_memory_used(void) {
    return gc_state.total_allocated;
}

size_t gc_object_count(void) {
    size_t count = 0;
    GCObject *cur = gc_state.objects;
    while (cur) {
        count++;
        cur = cur->next;
    }
    return count;
}