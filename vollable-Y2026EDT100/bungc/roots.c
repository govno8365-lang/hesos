#include <stdlib.h>
#include <string.h>
#include "gc.h"

typedef struct RootStack {
    struct RootStack *next;
    GCObject *object;
} RootStack;

static RootStack *root_stack = NULL;

void gc_push_root(GCObject *obj) {
    RootStack *node = malloc(sizeof(RootStack));
    node->object = obj;
    node->next = root_stack;
    root_stack = node;
}

GCObject* gc_pop_root(void) {
    if (!root_stack) return NULL;
    RootStack *node = root_stack;
    GCObject *obj = node->object;
    root_stack = node->next;
    free(node);
    return obj;
}

GCObject* gc_peek_root(void) {
    if (!root_stack) return NULL;
    return root_stack->object;
}

void gc_scan_roots(void) {
    RootStack *cur = root_stack;
    while (cur) {
        if (cur->object) {
            gc_add_root(&cur->object);
        }
        cur = cur->next;
    }
}

void gc_clear_roots(void) {
    RootStack *cur = root_stack;
    while (cur) {
        if (cur->object) {
            gc_remove_root(&cur->object);
        }
        cur = cur->next;
    }
}

int gc_is_root(GCObject *obj) {
    RootStack *cur = root_stack;
    while (cur) {
        if (cur->object == obj) return 1;
        cur = cur->next;
    }
    return 0;
}