#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpretator.h"

/*
 * environment.c - Variable Management with Scopes
 *
 * This file implements a scope-based variable system.
 * Each scope is a linked list of variables.
 * Scopes are nested: inner scopes can access outer scopes.
 */

/* Scope structure */
typedef struct Scope {
    VariableEntry *variables;
    struct Scope *parent;
    int depth;
} Scope;

/* Global scope stack */
static Scope *current_scope = NULL;
static int scope_depth = 0;

/*
 * push_scope - create a new scope
 */
void push_scope(void) {
    Scope *new_scope = (Scope*)malloc(sizeof(Scope));
    new_scope->variables = NULL;
    new_scope->parent = current_scope;
    new_scope->depth = ++scope_depth;
    current_scope = new_scope;
}

/*
 * pop_scope - destroy the current scope
 */
void pop_scope(void) {
    if (!current_scope) return;

    /* Free all variables in this scope */
    VariableEntry *entry = current_scope->variables;
    while (entry) {
        VariableEntry *next = entry->next;
        free(entry);
        entry = next;
    }

    Scope *parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
    scope_depth--;
}

/*
 * get_variable - find variable in current or parent scopes
 */
Value* get_variable(const char *name) {
    Scope *scope = current_scope;
    while (scope) {
        VariableEntry *entry = scope->variables;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return &entry->value;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

/*
 * set_variable - set variable in current scope
 */
void set_variable(const char *name, Value *value) {
    if (!current_scope) {
        /* Global scope — create if not exists */
        push_scope();
    }

    /* Check if variable exists in current scope */
    VariableEntry *entry = current_scope->variables;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            entry->value = *value;
            entry->defined = 1;
            return;
        }
        entry = entry->next;
    }

    /* Create new variable in current scope */
    VariableEntry *new_entry = (VariableEntry*)malloc(sizeof(VariableEntry));
    strcpy(new_entry->name, name);
    new_entry->value = *value;
    new_entry->defined = 1;
    new_entry->next = current_scope->variables;
    current_scope->variables = new_entry;
}

/*
 * init_environment - initialize the environment
 */
void init_environment(void) {
    current_scope = NULL;
    scope_depth = 0;
    push_scope(); /* Global scope */
}

/*
 * cleanup_environment - free all scopes
 */
void cleanup_environment(void) {
    while (current_scope) {
        pop_scope();
    }
}