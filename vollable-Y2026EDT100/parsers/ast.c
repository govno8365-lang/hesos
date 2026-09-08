#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/*
 * ast.c - Abstract Syntax Tree Management Implementation
 *
 * This file contains the implementation of AST node creation,
 * management, and utility functions.
 */

/*
 * create_node - create a new AST node
 *
 * This function allocates memory for a new node and initializes all
 * fields to default values (zero or NULL). The node's type field is
 * set to the specified type.
 *
 * Returns:
 *   ASTNode* - pointer to the newly created node, or NULL on failure
 */
ASTNode* create_node(ASTNodeType type) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for AST node\n");
        return NULL;
    }

    node->type = type;
    node->value[0] = '\0';
    node->line = 0;
    node->col = 0;
    node->left = NULL;
    node->right = NULL;
    node->body = NULL;
    node->next = NULL;

    return node;
}

/*
 * create_node_with_value - create a new AST node with a value
 *
 * This function is a convenience wrapper around create_node that also
 * copies a string value into the node's value field.
 *
 * Returns:
 *   ASTNode* - pointer to the newly created node, or NULL on failure
 */
ASTNode* create_node_with_value(ASTNodeType type, const char *value) {
    ASTNode *node = create_node(type);
    if (node && value) {
        strncpy(node->value, value, sizeof(node->value) - 1);
        node->value[sizeof(node->value) - 1] = '\0';
    }
    return node;
}

/*
 * print_ast - print the AST in a readable format
 *
 * This function recursively traverses the AST and prints each node
 * with appropriate indentation to show the tree structure.
 *
 * Parameters:
 *   node - root node of the AST to print
 *   indent - current indentation level (0 for root)
 */
void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    /* Print indentation */
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    /* Print the node type and value */
    const char *type_names[] = {
        "PROGRAM", "BLOCK", "SET", "KPRINTK", "WREADW",
        "GOTO", "STOP", "SETEXCEPTION", "REPEAT", "IN",
        "BINARY_OP", "VARIABLE", "NUMBER", "STRING", "TYPE",
        "LABEL", "EMPTY"
    };
    printf("%s", type_names[node->type]);

    if (node->value[0]) {
        printf(": %s", node->value);
    }

    if (node->line > 0) {
        printf(" (line %d)", node->line);
    }

    printf("\n");

    /* Print child nodes with increased indentation */
    if (node->left) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("LEFT:\n");
        print_ast(node->left, indent + 2);
    }

    if (node->right) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("RIGHT:\n");
        print_ast(node->right, indent + 2);
    }

    if (node->body) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("BODY:\n");
        print_ast(node->body, indent + 2);
    }

    if (node->next) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("NEXT:\n");
        print_ast(node->next, indent + 2);
    }
}

/*
 * free_ast - recursively free all nodes in the AST
 *
 * This function frees all nodes in the AST using a post-order traversal
 * to ensure children are freed before the parent.
 *
 * Parameters:
 *   node - root node of the AST to free
 */
void free_ast(ASTNode *node) {
    if (!node) return;

    /* Recursively free child nodes */
    if (node->left) {
        free_ast(node->left);
        node->left = NULL;
    }
    if (node->right) {
        free_ast(node->right);
        node->right = NULL;
    }
    if (node->body) {
        free_ast(node->body);
        node->body = NULL;
    }
    if (node->next) {
        free_ast(node->next);
        node->next = NULL;
    }

    /* Free the node itself */
    free(node);
}