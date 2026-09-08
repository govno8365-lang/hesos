#ifndef PARSER_H
#define PARSER_H

/*
 * parser.h - Vollable Parser Interface
 *
 * This file defines the parser state, AST node types, and public
 * functions for parsing Vollable source code. The parser consumes
 * tokens from the lexer and produces an Abstract Syntax Tree (AST).
 *
 * The parser is responsible for:
 * - Checking the correct order of keywords (start, buns, stop, etc.)
 * - Building the AST from the token stream
 * - Reporting syntax errors with line/column information
 *
 * The parser is a recursive descent parser, meaning it uses a set
 * of mutually recursive functions that correspond to the grammar
 * rules of the Vollable language.
 */

#include "../parcers/token.h"

/*
 * ASTNodeType - enumeration of all possible AST node types
 *
 * Each node type represents a different language construct:
 * - Program nodes: NODE_PROGRAM (root of the tree)
 * - Block nodes: NODE_BLOCK (buns...bun)
 * - Statement nodes: NODE_SET, NODE_KPRINTK, NODE_WREADW, etc.
 * - Control flow nodes: NODE_IN, NODE_REPEAT, NODE_GOTO
 * - Expression nodes: NODE_BINARY_OP (+, -, *, /, %)
 * - Leaf nodes: NODE_VARIABLE, NODE_NUMBER, NODE_STRING
 * - Type nodes: NODE_TYPE (number, letter, operator)
 * - Exception nodes: NODE_SETEXCEPTION
 */
typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_SET,
    NODE_KPRINTK,
    NODE_WREADW,
    NODE_GOTO,
    NODE_STOP,
    NODE_SETEXCEPTION,
    NODE_REPEAT,
    NODE_IN,
    NODE_BINARY_OP,
    NODE_VARIABLE,
    NODE_NUMBER,
    NODE_STRING,
    NODE_TYPE,
    NODE_LABEL,
    NODE_EMPTY
} ASTNodeType;

/*
 * ASTNode - structure representing a node in the Abstract Syntax Tree
 *
 * Each node stores:
 * - type: the kind of node (from ASTNodeType)
 * - value: string value (for identifiers, numbers, strings)
 * - line/col: source position for error reporting
 * - left/right: child nodes (for binary operations and expressions)
 * - body: block content (for repeat and in)
 * - next: pointer to the next node in a list (for sequences of statements)
 *
 * The AST is built by the parser and then consumed by the checker
 * and interpreter or code generator.
 */
typedef struct ASTNode {
    ASTNodeType type;
    char value[128];
    int line;
    int col;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *body;
    struct ASTNode *next;
} ASTNode;

/*
 * ParserState - state structure for the parser
 *
 * This structure holds all the information needed to parse the
 * token stream:
 * - tokens: array of tokens from the lexer
 * - pos: current position in the token array
 * - count: total number of tokens
 * - current: the current token being processed
 * - error_count: number of syntax errors encountered
 *
 * The parser maintains this state and updates it as it consumes
 * tokens and builds the AST.
 */
typedef struct {
    Token *tokens;
    int pos;
    int count;
    Token current;
    int error_count;
} ParserState;

/*
 * Global parser state - used throughout the parsing process
 */
extern ParserState parser;

/*
 * parse_program - main entry point for the parser
 *
 * This function parses the entire token stream and builds the AST.
 * It expects the program to start with 'start' and end with 'stop'.
 *
 * Returns:
 *   ASTNode* - pointer to the root node of the AST (NODE_PROGRAM)
 *
 * If a syntax error is encountered, the function will report it
 * and continue parsing (error recovery) to find multiple errors.
 */
ASTNode* parse_program(void);

/*
 * parse_block - parse a block of statements between buns and bun
 *
 * This function parses a block of statements enclosed in 'buns' and 'bun'.
 * It is used for top-level program blocks, repeat bodies, and in bodies.
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_BLOCK node containing the statements
 */
ASTNode* parse_block(void);

/*
 * parse_statement - parse a single statement
 *
 * This function determines the type of statement based on the current
 * token and dispatches to the appropriate parsing function.
 *
 * Returns:
 *   ASTNode* - pointer to the parsed statement node
 */
ASTNode* parse_statement(void);

/*
 * parse_set - parse a variable declaration or assignment
 *
 * Syntax: set name [** type] = value
 *   - name: variable name (identifier)
 *   - type: optional type declaration (number, letter, operator)
 *   - value: expression or literal value
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_SET node
 */
ASTNode* parse_set(void);

/*
 * parse_kprintk - parse an output statement
 *
 * Syntax: kprintk expression
 *   - expression: value to print (variable, number, string, or expression)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_KPRINTK node
 */
ASTNode* parse_kprintk(void);

/*
 * parse_wreadw - parse an input statement
 *
 * Syntax: wreadw prompt
 *   - prompt: string literal or variable containing the prompt text
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_WREADW node
 */
ASTNode* parse_wreadw(void);

/*
 * parse_goto - parse a jump statement
 *
 * Syntax: goto label
 *   - label: identifier marking the target location
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_GOTO node
 */
ASTNode* parse_goto(void);

/*
 * parse_stop - parse a stop statement
 *
 * Syntax: stop
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_STOP node
 */
ASTNode* parse_stop(void);

/*
 * parse_setexpception - parse an exception throw
 *
 * Syntax: setexpception message
 *   - message: string literal or variable containing the exception message
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_SETEXCEPTION node
 */
ASTNode* parse_setexpception(void);

/*
 * parse_repeat - parse a loop statement
 *
 * Syntax: repeat N buns ... bun
 *   - N: number of iterations (constant or variable)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_REPEAT node with the body block
 */
ASTNode* parse_repeat(void);

/*
 * parse_in - parse a conditional statement
 *
 * Syntax: in condition buns ... bun
 *   - condition: expression to evaluate (comparison or value)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_IN node with the body block
 */
ASTNode* parse_in(void);

/*
 * parse_expression - parse an arithmetic expression
 *
 * Syntax: left op right
 *   - left: left operand (variable, number, string, or expression)
 *   - op: operator (+, -, *, /, %)
 *   - right: right operand (variable, number, string, or expression)
 *
 * This function handles operator precedence and associativity.
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_BINARY_OP node
 */
ASTNode* parse_expression(void);

/*
 * parse_type - parse a type declaration
 *
 * Syntax: ** type
 *   - type: number, letter, or operator
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_TYPE node
 */
ASTNode* parse_type(void);

/*
 * error - report a syntax error with position information
 *
 * Parameters:
 *   msg - error message to display
 *
 * This function prints the error message along with the line and
 * column of the current token, then increments the error count.
 */
void error(const char *msg);

/*
 * print_ast - print the AST in a readable format
 *
 * Parameters:
 *   node - root node of the AST to print
 *   indent - current indentation level (0 for root)
 *
 * This function is primarily used for debugging and development.
 * It recursively traverses the AST and prints each node with
 * appropriate indentation to show the tree structure.
 */
void print_ast(ASTNode *node, int indent);

/*
 * free_ast - recursively free all nodes in the AST
 *
 * Parameters:
 *   node - root node of the AST to free
 *
 * This function frees all nodes in the AST to prevent memory leaks.
 * It uses a post-order traversal to ensure children are freed first.
 */
void free_ast(ASTNode *node);

#endif