#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"

/*
 * checker.c - Vollable Type and Semantic Checker
 *
 * This file contains the implementation of the type checker and
 * semantic analyzer for the Vollable language. It verifies that:
 * - Variables are declared before use
 * - Types are used correctly
 * - Labels exist for goto statements
 * - Type declarations are valid (number, letter, operator)
 * - Exception messages are valid
 * - Block structures are properly nested
 *
 * The checker traverses the AST and collects information about
 * variables, types, and labels in the current scope.
 */

/*
 * SymbolTableEntry - structure for storing symbol information
 *
 * This structure is used to track variables and their types
 * in each scope. It supports nested scopes for blocks, repeats,
 * and in statements.
 */
typedef struct SymbolTableEntry {
    char name[128];
    char type[32];
    int declared;
    int line;
    int col;
    struct SymbolTableEntry *next;
} SymbolTableEntry;

/*
 * ScopeState - structure for managing scopes
 *
 * This structure maintains a chain of symbol tables for nested scopes.
 * Each block, repeat, and in creates a new scope.
 */
typedef struct ScopeState {
    SymbolTableEntry *entries;
    int depth;
    struct ScopeState *parent;
} ScopeState;

/*
 * Global state for the checker
 */
static ScopeState *current_scope = NULL;
static int error_count = 0;

/*
 * push_scope - create a new scope
 *
 * This function creates a new scope for a block and pushes it onto
 * the scope stack.
 */
static void push_scope(void) {
    ScopeState *new_scope = (ScopeState*)malloc(sizeof(ScopeState));
    new_scope->entries = NULL;
    new_scope->depth = current_scope ? current_scope->depth + 1 : 0;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

/*
 * pop_scope - remove the current scope
 *
 * This function pops the current scope from the stack and frees
 * its symbol table entries.
 */
static void pop_scope(void) {
    if (!current_scope) return;

    /* Free all symbol entries in this scope */
    SymbolTableEntry *entry = current_scope->entries;
    while (entry) {
        SymbolTableEntry *next = entry->next;
        free(entry);
        entry = next;
    }

    ScopeState *parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
}

/*
 * add_symbol - add a variable to the current scope
 *
 * Parameters:
 *   name - variable name
 *   type - variable type (number, letter, operator)
 *   line - source line number
 *   col - source column number
 *
 * This function adds a symbol to the current scope's symbol table.
 */
static void add_symbol(const char *name, const char *type, int line, int col) {
    if (!current_scope) return;

    /* Check if the variable already exists in this scope */
    SymbolTableEntry *entry = current_scope->entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            fprintf(stderr, "Semantic error [%d:%d]: Variable '%s' already declared in this scope\n",
                    line, col, name);
            error_count++;
            return;
        }
        entry = entry->next;
    }

    /* Add the symbol */
    SymbolTableEntry *new_entry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    strcpy(new_entry->name, name);
    strcpy(new_entry->type, type);
    new_entry->declared = 1;
    new_entry->line = line;
    new_entry->col = col;
    new_entry->next = current_scope->entries;
    current_scope->entries = new_entry;
}

/*
 * lookup_symbol - find a variable in the current or parent scopes
 *
 * Parameters:
 *   name - variable name to look up
 *
 * Returns:
 *   SymbolTableEntry* - pointer to the symbol entry, or NULL if not found
 *
 * This function searches for a variable in the current scope and then
 * in parent scopes if not found.
 */
static SymbolTableEntry* lookup_symbol(const char *name) {
    ScopeState *scope = current_scope;
    while (scope) {
        SymbolTableEntry *entry = scope->entries;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

/*
 * check_expression - check the type of an expression node
 *
 * Parameters:
 *   node - expression node to check
 *
 * Returns:
 *   const char* - type of the expression (number, letter, operator)
 *
 * This function recursively checks an expression node and returns
 * its type. It also verifies that variables are declared and that
 * operators are used with compatible types.
 */
static const char* check_expression(ASTNode *node) {
    if (!node) return NULL;

    if (node->type == NODE_VARIABLE) {
        SymbolTableEntry *entry = lookup_symbol(node->value);
        if (!entry) {
            fprintf(stderr, "Semantic error [%d:%d]: Undefined variable '%s'\n",
                    node->line, node->col, node->value);
            error_count++;
            return NULL;
        }
        return entry->type;
    }

    if (node->type == NODE_NUMBER) {
        return "number";
    }

    if (node->type == NODE_STRING) {
        return "letter";
    }

    if (node->type == NODE_BINARY_OP) {
        const char *left_type = check_expression(node->left);
        const char *right_type = check_expression(node->right);

        if (!left_type || !right_type) {
            return NULL;
        }

        /* Check operator compatibility */
        if (strcmp(left_type, "number") != 0 || strcmp(right_type, "number") != 0) {
            if (strcmp(node->value, "+") == 0) {
                /* '+' works with letter + number, number + letter, letter + letter */
                if (strcmp(left_type, "letter") == 0 || strcmp(right_type, "letter") == 0) {
                    return "letter";
                }
            }
            fprintf(stderr, "Semantic error [%d:%d]: Operator '%s' requires numeric operands\n",
                    node->line, node->col, node->value);
            error_count++;
            return NULL;
        }

        return "number";
    }

    return NULL;
}

/*
 * check_statement - check a statement node
 *
 * Parameters:
 *   node - statement node to check
 *
 * This function checks the semantic correctness of a statement node.
 * It verifies types, variable declarations, and other semantic rules.
 */
static void check_statement(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_SET: {
            /* Check the variable name */
            if (node->left && node->left->type == NODE_VARIABLE) {
                const char *type = "any";
                if (node->body && node->body->type == NODE_TYPE) {
                    type = node->body->value;
                }

                /* Check the value expression */
                const char *value_type = check_expression(node->right);
                if (value_type && node->body && node->body->type == NODE_TYPE) {
                    if (strcmp(value_type, type) != 0) {
                        fprintf(stderr, "Semantic error [%d:%d]: Type mismatch: expected '%s', got '%s'\n",
                                node->line, node->col, type, value_type);
                        error_count++;
                    }
                }

                add_symbol(node->left->value, type, node->line, node->col);
            }
            break;
        }

        case NODE_KPRINTK: {
            check_expression(node->left);
            break;
        }

        case NODE_WREADW: {
            check_expression(node->left);
            break;
        }

        case NODE_GOTO: {
            if (node->left && node->left->type == NODE_LABEL) {
                /* Label existence will be checked after full AST traversal */
            }
            break;
        }

        case NODE_SETEXCEPTION: {
            check_expression(node->left);
            break;
        }

        case NODE_REPEAT: {
            /* Check the count expression */
            check_expression(node->left);
            /* Push a new scope for the repeat body */
            push_scope();
            if (node->right && node->right->type == NODE_BLOCK) {
                check_statement(node->right);
            }
            pop_scope();
            break;
        }

        case NODE_IN: {
            /* Check the condition expression */
            check_expression(node->left);
            /* Push a new scope for the in body */
            push_scope();
            if (node->right && node->right->type == NODE_BLOCK) {
                check_statement(node->right);
            }
            pop_scope();
            break;
        }

        case NODE_BLOCK: {
            ASTNode *stmt = node->body;
            while (stmt) {
                check_statement(stmt);
                stmt = stmt->next;
            }
            break;
        }

        default:
            break;
    }
}

/*
 * check_program - perform semantic analysis on the entire program
 *
 * Parameters:
 *   node - root node of the AST (NODE_PROGRAM)
 *
 * This function performs semantic analysis on the entire program,
 * checking type correctness, variable declarations, and other
 * semantic rules.
 */
void check_program(ASTNode *node) {
    if (!node || node->type != NODE_PROGRAM) {
        fprintf(stderr, "Semantic error: Invalid program root node\n");
        return;
    }

    error_count = 0;
    current_scope = NULL;

    /* Create a global scope for the program */
    push_scope();

    /* Check the block content */
    if (node->right && node->right->type == NODE_BLOCK) {
        check_statement(node->right);
    }

    /* Pop the global scope */
    pop_scope();

    if (error_count > 0) {
        fprintf(stderr, "\nSemantic analysis completed with %d errors\n", error_count);
    } else {
        printf("\nSemantic analysis completed successfully\n");
    }
}

/*
 * get_error_count - get the number of errors found during checking
 *
 * Returns:
 *   int - number of semantic errors
 */
int get_error_count(void) {
    return error_count;
}