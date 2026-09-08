#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpretator.h"

/*
 * executor.c - Command Execution Logic
 *
 * This file contains the implementation of all Vollable commands:
 * - set       : variable declaration/assignment
 * - kprintk   : output
 * - wreadw    : input
 * - goto      : jump to label
 * - stop      : stop execution
 * - setexpception : throw exception
 * - repeat    : loop N times
 * - in        : conditional execution
 * - arithmetic: + - * / %
 */

/*
 * execute_node - dispatch to the appropriate command handler
 */
int execute_node(ASTNode *node) {
    if (!node) return 1;
    if (!env.running) return 0;

    switch (node->type) {
        case NODE_SET:          return execute_set(node);
        case NODE_KPRINTK:      return execute_kprintk(node);
        case NODE_WREADW:       return execute_wreadw(node);
        case NODE_GOTO:         return execute_goto(node);
        case NODE_STOP:         return execute_stop(node);
        case NODE_SETEXCEPTION: return execute_setexpception(node);
        case NODE_REPEAT:       return execute_repeat(node);
        case NODE_IN:           return execute_in(node);
        case NODE_BLOCK:        return execute_block(node);
        default:                return 1;
    }
}

/* ============================================================
   COMMAND: set
   Syntax: set name = value
   Syntax: set name ** type = value
   ============================================================ */
int execute_set(ASTNode *node) {
    if (!node || !node->left) return 1;

    /* Get variable name */
    ASTNode *var_node = node->left;
    if (var_node->type != NODE_VARIABLE) return 1;

    /* Evaluate the value */
    Value *value = evaluate_expression(node->right);
    if (!value) {
        throw_exception("Failed to evaluate expression");
        return 0;
    }

    /* Store in current scope */
    set_variable(var_node->value, value);
    return 1;
}

/* ============================================================
   COMMAND: kprintk
   Syntax: kprintk expression
   ============================================================ */
int execute_kprintk(ASTNode *node) {
    if (!node || !node->left) return 1;

    Value *value = evaluate_expression(node->left);
    if (!value) return 1;

    print_value(value);
    return 1;
}

/* ============================================================
   COMMAND: wreadw
   Syntax: wreadw prompt
   ============================================================ */
int execute_wreadw(ASTNode *node) {
    if (!node || !node->left) return 1;

    /* Print prompt */
    Value *prompt = evaluate_expression(node->left);
    if (prompt) {
        print_value(prompt);
    }

    /* Read input using syscall */
    char *input = read_input(NULL);
    if (!input) {
        throw_exception("Failed to read input");
        return 0;
    }

    /* Store input as a letter value */
    Value val;
    val.type = VAL_LETTER;
    strncpy(val.letter, input, 255);
    val.letter[255] = '\0';

    /* Return the value (caller handles assignment) */
    /* In Vollable, wreadw returns a value that can be used in set */
    return 1;
}

/* ============================================================
   COMMAND: goto
   Syntax: goto label
   ============================================================ */
int execute_goto(ASTNode *node) {
    if (!node || !node->left) return 1;

    ASTNode *label_node = node->left;
    if (label_node->type != NODE_LABEL) return 1;

    /* Look up label in hash table */
    ASTNode *target = get_label(label_node->value);
    if (!target) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Undefined label: %s", label_node->value);
        throw_exception(msg);
        return 0;
    }

    /* Jump to the label */
    env.current_node = target;
    return 1;
}

/* ============================================================
   COMMAND: stop
   Syntax: stop
   ============================================================ */
int execute_stop(ASTNode *node) {
    (void)node;
    env.running = 0;
    return 0;
}

/* ============================================================
   COMMAND: setexpception
   Syntax: setexpception message
   ============================================================ */
int execute_setexpception(ASTNode *node) {
    if (!node || !node->left) return 0;

    Value *msg = evaluate_expression(node->left);
    if (msg && msg->type == VAL_LETTER) {
        throw_exception(msg->letter);
    } else {
        throw_exception("Exception thrown");
    }

    return 0;
}

/* ============================================================
   COMMAND: repeat
   Syntax: repeat N buns ... bun
   ============================================================ */
int execute_repeat(ASTNode *node) {
    if (!node || !node->left || !node->right) return 1;

    /* Evaluate the count */
    Value *count_val = evaluate_expression(node->left);
    if (!count_val) return 1;

    int count = (int)count_val->number;
    if (count <= 0) return 1;

    /* Execute the body count times */
    for (int i = 0; i < count && env.running; i++) {
        push_scope();  /* New scope for each iteration */
        execute_block(node->right);
        pop_scope();
    }

    return env.running;
}

/* ============================================================
   COMMAND: in
   Syntax: in condition buns ... bun
   ============================================================ */
int execute_in(ASTNode *node) {
    if (!node || !node->left || !node->right) return 1;

    /* Evaluate the condition */
    Value *cond = evaluate_expression(node->left);
    if (!cond) return 1;

    /* Check if condition is truthy */
    int is_true = 0;
    if (cond->type == VAL_NUMBER && cond->number != 0) {
        is_true = 1;
    } else if (cond->type == VAL_LETTER && cond->letter[0] != '\0') {
        is_true = 1;
    } else if (cond->type == VAL_OPERATOR) {
        is_true = 1; /* operators are truthy */
    }

    if (is_true) {
        push_scope();  /* New scope for the condition body */
        execute_block(node->right);
        pop_scope();
    }

    return env.running;
}

/* ============================================================
   BLOCK: execute a sequence of statements
   ============================================================ */
int execute_block(ASTNode *block) {
    if (!block || block->type != NODE_BLOCK) return 1;

    ASTNode *stmt = block->body;
    while (stmt && env.running) {
        execute_node(stmt);
        stmt = stmt->next;
    }

    return env.running;
}

/* ============================================================
   EXPRESSIONS: evaluate arithmetic and values
   ============================================================ */
Value* evaluate_expression(ASTNode *node) {
    static Value result;
    if (!node) return NULL;

    switch (node->type) {
        case NODE_VARIABLE: {
            return get_variable(node->value);
        }

        case NODE_NUMBER: {
            result.type = VAL_NUMBER;
            result.number = atof(node->value);
            return &result;
        }

        case NODE_STRING: {
            result.type = VAL_LETTER;
            strncpy(result.letter, node->value, 255);
            result.letter[255] = '\0';
            return &result;
        }

        case NODE_BINARY_OP: {
            Value *left = evaluate_expression(node->left);
            Value *right = evaluate_expression(node->right);
            if (!left || !right) return NULL;

            char op = node->value[0];

            /* Number + Number */
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER) {
                result.type = VAL_NUMBER;
                switch (op) {
                    case '+': result.number = left->number + right->number; break;
                    case '-': result.number = left->number - right->number; break;
                    case '*': result.number = left->number * right->number; break;
                    case '/':
                        if (right->number == 0) {
                            throw_exception("Division by zero");
                            return NULL;
                        }
                        result.number = left->number / right->number;
                        break;
                    case '%':
                        result.number = (int)left->number % (int)right->number;
                        break;
                    default: return NULL;
                }
                return &result;
            }

            /* String concatenation: letter + letter */
            if (op == '+' && left->type == VAL_LETTER && right->type == VAL_LETTER) {
                result.type = VAL_LETTER;
                snprintf(result.letter, 255, "%s%s", left->letter, right->letter);
                return &result;
            }

            /* Mixed: number + letter -> string */
            if (op == '+') {
                result.type = VAL_LETTER;
                char temp[64];
                if (left->type == VAL_NUMBER) {
                    snprintf(temp, 63, "%g", left->number);
                    snprintf(result.letter, 255, "%s%s", temp, right->letter);
                } else if (right->type == VAL_NUMBER) {
                    snprintf(temp, 63, "%g", right->number);
                    snprintf(result.letter, 255, "%s%s", left->letter, temp);
                } else {
                    return NULL;
                }
                return &result;
            }

            throw_exception("Invalid operation for types");
            return NULL;
        }

        default:
            return NULL;
    }
}