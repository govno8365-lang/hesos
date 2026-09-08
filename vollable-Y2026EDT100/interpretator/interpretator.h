#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include "../parcers/parser.h"
#include "../bungc/gc.h"

/*
 * interpretator.h - Vollable Interpreter Interface
 *
 * This file defines the interpreter structures and functions.
 * All I/O uses raw syscalls, no standard C library I/O.
 */

/* Value types */
typedef enum {
    VAL_NUMBER,
    VAL_LETTER,
    VAL_OPERATOR,
    VAL_UNDEFINED
} ValueType;

/* Value union */
typedef struct {
    ValueType type;
    union {
        double number;
        char letter[256];
        char operator[8];
    };
} Value;

/* Environment state */
typedef struct {
    int running;
    int error_code;
    char error_message[256];
} Environment;

extern Environment env;

/* --- Environment --- */
void init_environment(void);
void cleanup_environment(void);
void push_scope(void);
void pop_scope(void);
Value* get_variable(const char *name);
void set_variable(const char *name, Value *value);

/* --- Labels --- */
void register_label(const char *name, ASTNode *node);
ASTNode* get_label(const char *name);
int is_label_defined(const char *name);
void clear_labels(void);
void collect_labels(ASTNode *node);

/* --- Execution --- */
void interpret(ASTNode *root);
int execute_node(ASTNode *node);
int execute_block(ASTNode *block);
Value* evaluate_expression(ASTNode *node);

/* --- I/O --- */
void print_value(Value *value);
char* read_input(const char *prompt);
void throw_exception(const char *message);

/* --- Syscalls --- */
long syscall_write(int fd, const void *buffer, size_t count);
long syscall_read(int fd, void *buffer, size_t count);
void syscall_exit(int code);

#endif