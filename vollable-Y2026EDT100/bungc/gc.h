#ifndef GC_H
#define GC_H

#include <stddef.h>
#include <stdint.h>

/*
 * bungc - Bun Garbage Collector
 * Incremental mark-and-sweep with write barriers
 * Designed for Vollable language
 */

/* Object types */
typedef enum {
    OBJ_INT,
    OBJ_FLOAT,
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_OBJECT
} ObjectType;

/* GC Object header */
typedef struct GCObject {
    ObjectType type;
    uint8_t marked;
    uint8_t visited;
    size_t size;
    struct GCObject *next;
    struct GCObject *next_work;
    void *data;
} GCObject;

/* Root set */
typedef struct GCRoot {
    struct GCRoot *next;
    GCObject **ptr;
} GCRoot;

/* GC state */
typedef struct GCState {
    GCObject *objects;
    GCObject *worklist;
    GCRoot *roots;
    size_t total_allocated;
    size_t threshold;
    size_t allocations_since_gc;
    int enabled;
    int running;
    int pause_requested;
} GCState;

extern GCState gc_state;

void gc_init(size_t threshold);
GCObject* gc_alloc(ObjectType type, size_t size);
void gc_add_root(GCObject **ptr);
void gc_remove_root(GCObject **ptr);
void gc_step(void);
void gc_collect(void);
void gc_maybe_collect(void);
void gc_enable(int enable);
size_t gc_memory_used(void);
size_t gc_object_count(void);

#endif