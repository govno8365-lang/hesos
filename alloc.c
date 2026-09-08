#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"

GCObject* gc_new_int(int value) {
    GCObject *obj = gc_alloc(OBJ_INT, sizeof(int));
    if (!obj) return NULL;
    *(int*)obj->data = value;
    return obj;
}

GCObject* gc_new_float(double value) {
    GCObject *obj = gc_alloc(OBJ_FLOAT, sizeof(double));
    if (!obj) return NULL;
    *(double*)obj->data = value;
    return obj;
}

GCObject* gc_new_string(const char *str) {
    size_t len = strlen(str) + 1;
    GCObject *obj = gc_alloc(OBJ_STRING, len);
    if (!obj) return NULL;
    memcpy(obj->data, str, len);
    return obj;
}

GCObject* gc_new_array(size_t capacity) {
    size_t size = sizeof(GCObject*) * capacity + sizeof(size_t) * 2;
    GCObject *obj = gc_alloc(OBJ_ARRAY, size);
    if (!obj) return NULL;

    size_t *header = (size_t*)obj->data;
    header[0] = 0;
    header[1] = capacity;

    GCObject **elements = (GCObject**)(header + 2);
    for (size_t i = 0; i < capacity; i++) {
        elements[i] = NULL;
    }

    return obj;
}

size_t gc_array_length(GCObject *obj) {
    if (!obj || obj->type != OBJ_ARRAY) return 0;
    size_t *header = (size_t*)obj->data;
    return header[0];
}

GCObject* gc_array_get(GCObject *obj, size_t index) {
    if (!obj || obj->type != OBJ_ARRAY) return NULL;
    size_t *header = (size_t*)obj->data;
    size_t length = header[0];
    if (index >= length) return NULL;
    GCObject **elements = (GCObject**)(header + 2);
    return elements[index];
}

void gc_array_set(GCObject *obj, size_t index, GCObject *value) {
    if (!obj || obj->type != OBJ_ARRAY) return;
    size_t *header = (size_t*)obj->data;
    size_t capacity = header[1];
    if (index >= capacity) return;
    GCObject **elements = (GCObject**)(header + 2);
    if (index >= header[0]) {
        header[0] = index + 1;
    }
    elements[index] = value;
}