#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpretator.h"

/*
 * labels.c - Label Table with Hash Map
 *
 * This file implements a hash table for storing and looking up
 * labels in the program. Hash table size is fixed for simplicity.
 */

#define LABEL_TABLE_SIZE 256

/* Hash table entry */
typedef struct LabelEntry {
    char name[128];
    ASTNode *node;
    struct LabelEntry *next;
} LabelEntry;

/* Hash table */
static LabelEntry *label_table[LABEL_TABLE_SIZE] = {0};

/*
 * hash_label - simple hash function for label names
 */
static unsigned int hash_label(const char *name) {
    unsigned int hash = 0;
    while (*name) {
        hash = (hash * 31) + *name;
        name++;
    }
    return hash % LABEL_TABLE_SIZE;
}

/*
 * register_label - add a label to the hash table
 */
void register_label(const char *name, ASTNode *node) {
    unsigned int index = hash_label(name);

    /* Check if label already exists */
    LabelEntry *entry = label_table[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            entry->node = node;
            entry->defined = 1;
            return;
        }
        entry = entry->next;
    }

    /* Create new entry */
    LabelEntry *new_entry = (LabelEntry*)malloc(sizeof(LabelEntry));
    strcpy(new_entry->name, name);
    new_entry->node = node;
    new_entry->defined = 1;
    new_entry->next = label_table[index];
    label_table[index] = new_entry;
}

/*
 * get_label - find a label in the hash table
 */
ASTNode* get_label(const char *name) {
    unsigned int index = hash_label(name);
    LabelEntry *entry = label_table[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->node;
        }
        entry = entry->next;
    }
    return NULL;
}

/*
 * is_label_defined - check if a label exists
 */
int is_label_defined(const char *name) {
    unsigned int index = hash_label(name);
    LabelEntry *entry = label_table[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->defined;
        }
        entry = entry->next;
    }
    return 0;
}

/*
 * clear_labels - free all label entries
 */
void clear_labels(void) {
    for (int i = 0; i < LABEL_TABLE_SIZE; i++) {
        LabelEntry *entry = label_table[i];
        while (entry) {
            LabelEntry *next = entry->next;
            free(entry);
            entry = next;
        }
        label_table[i] = NULL;
    }
}

/*
 * collect_labels - traverse AST and collect all labels
 */
void collect_labels(ASTNode *node) {
    if (!node) return;

    /* If this is a label node, register it */
    if (node->type == NODE_LABEL) {
        register_label(node->value, node);
    }

    /* Traverse children */
    if (node->left) collect_labels(node->left);
    if (node->right) collect_labels(node->right);
    if (node->body) collect_labels(node->body);
    if (node->next) collect_labels(node->next);
}