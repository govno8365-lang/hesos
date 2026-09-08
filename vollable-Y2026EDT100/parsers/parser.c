#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"

/*
 * parser.c - Vollable Parser Implementation
 *
 * This file contains the implementation of the recursive descent parser
 * for the Vollable programming language. It consumes tokens from the
 * lexer and builds an Abstract Syntax Tree (AST) according to the
 * grammar rules of the language.
 *
 * The parser is designed to be robust and provide helpful error messages
 * with line and column information for each syntax error.
 */

/*
 * Global parser state
 */
ParserState parser;

/*
 * current_token - get the current token without consuming it
 *
 * Returns:
 *   Token - the token at the current position in the token stream
 */
static Token current_token(void) {
    if (parser.pos < parser.count) {
        return parser.tokens[parser.pos];
    }
    Token eof = {TOKEN_EOF, "EOF", 0, 0};
    return eof;
}

/*
 * peek_token - look ahead at a token without consuming it
 *
 * Parameters:
 *   offset - number of tokens to look ahead (1 = next token)
 *
 * Returns:
 *   Token - the token at position parser.pos + offset
 */
static Token peek_token(int offset) {
    int idx = parser.pos + offset;
    if (idx < parser.count) {
        return parser.tokens[idx];
    }
    Token eof = {TOKEN_EOF, "EOF", 0, 0};
    return eof;
}

/*
 * advance - consume the current token and move to the next
 *
 * This function advances the parser position by one token.
 * It should be called after checking the current token type.
 */
static void advance(void) {
    if (parser.pos < parser.count) {
        parser.pos++;
        parser.current = current_token();
    }
}

/*
 * expect - check if the current token matches a specific type
 *
 * Parameters:
 *   type - expected token type
 *   msg - error message to display if the token does not match
 *
 * This function checks the current token type and advances if it matches.
 * If it does not match, it reports an error and continues.
 */
static void expect(TokenType type, const char *msg) {
    if (current_token().type == type) {
        advance();
    } else {
        error(msg);
        parser.error_count++;
    }
}

/*
 * match - check if the current token matches a specific type
 *
 * Parameters:
 *   type - token type to check for
 *
 * Returns:
 *   1 if the current token matches the type, 0 otherwise
 *
 * This function checks the current token without consuming it.
 * It is used for lookahead and optional parsing.
 */
static int match(TokenType type) {
    return current_token().type == type;
}

/*
 * error - report a syntax error with position information
 *
 * Parameters:
 *   msg - error message to display
 *
 * This function prints the error message along with the line and
 * column of the current token. It is used throughout the parser
 * to report syntax errors.
 */
void error(const char *msg) {
    Token tok = current_token();
    fprintf(stderr, "Syntax error [%d:%d]: %s\n", tok.line, tok.col, msg);
}

/*
 * parse_program - main entry point for the parser
 *
 * This function parses the entire program. It expects the program
 * to start with the 'start' keyword, followed by the program name,
 * then a block of statements, and finally the 'stop' keyword.
 *
 * Returns:
 *   ASTNode* - pointer to the root node (NODE_PROGRAM)
 */
ASTNode* parse_program(void) {
    parser.error_count = 0;
    parser.current = current_token();

    /* Create the root program node */
    ASTNode *program = create_node(NODE_PROGRAM);
    if (!program) {
        error("Failed to allocate program node");
        return NULL;
    }

    /* Expect 'start' keyword */
    expect(TOKEN_START, "Program must start with 'start'");
    if (parser.error_count > 0) return program;

    /* Expect program name (identifier) */
    if (match(TOKEN_ID)) {
        ASTNode *name_node = create_node(NODE_LABEL);
        if (name_node) {
            strcpy(name_node->value, current_token().value);
            name_node->line = current_token().line;
            name_node->col = current_token().col;
            advance();
            /* Add name to program node (left child) */
            program->left = name_node;
        }
    } else {
        error("Expected program name");
        parser.error_count++;
        return program;
    }

    /* Parse the main block */
    ASTNode *block = parse_block();
    if (block) {
        program->right = block;
    }

    /* Expect 'stop' keyword at the end */
    expect(TOKEN_STOP, "Program must end with 'stop'");

    return program;
}

/*
 * parse_block - parse a block of statements between buns and bun
 *
 * This function parses a block of statements enclosed in 'buns' and 'bun'.
 * It is used for the main program block, repeat bodies, and in bodies.
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_BLOCK node containing the statements
 */
ASTNode* parse_block(void) {
    ASTNode *block = create_node(NODE_BLOCK);
    if (!block) {
        error("Failed to allocate block node");
        return NULL;
    }

    /* Expect 'buns' to open the block */
    expect(TOKEN_BUNS, "Expected 'buns' to start block");
    if (parser.error_count > 0) return block;

    /* Parse statements until we hit 'bun' or end of file */
    ASTNode *first_stmt = NULL;
    ASTNode *last_stmt = NULL;

    while (!match(TOKEN_BUN) && !match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            if (!first_stmt) {
                first_stmt = stmt;
            } else {
                last_stmt->next = stmt;
            }
            last_stmt = stmt;
        }
    }

    /* Store the list of statements in the block's body */
    block->body = first_stmt;

    /* Expect 'bun' to close the block */
    expect(TOKEN_BUN, "Expected 'bun' to close block");

    return block;
}

/*
 * parse_statement - parse a single statement
 *
 * This function determines the type of statement based on the current
 * token and dispatches to the appropriate parsing function.
 *
 * Returns:
 *   ASTNode* - pointer to the parsed statement node
 */
ASTNode* parse_statement(void) {
    Token tok = current_token();

    if (tok.type == TOKEN_SET) {
        return parse_set();
    } else if (tok.type == TOKEN_KPRINTK) {
        return parse_kprintk();
    } else if (tok.type == TOKEN_WREADW) {
        return parse_wreadw();
    } else if (tok.type == TOKEN_GOTO) {
        return parse_goto();
    } else if (tok.type == TOKEN_STOP) {
        return parse_stop();
    } else if (tok.type == TOKEN_SETEXCEPTION) {
        return parse_setexpception();
    } else if (tok.type == TOKEN_REPEAT) {
        return parse_repeat();
    } else if (tok.type == TOKEN_IN) {
        return parse_in();
    } else {
        error("Unexpected token in statement");
        advance(); /* Skip the unexpected token */
        return NULL;
    }
}

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
ASTNode* parse_set(void) {
    advance(); /* consume 'set' */

    /* Expect variable name */
    ASTNode *var_node = NULL;
    if (match(TOKEN_ID)) {
        var_node = create_node(NODE_VARIABLE);
        if (var_node) {
            strcpy(var_node->value, current_token().value);
            var_node->line = current_token().line;
            var_node->col = current_token().col;
        }
        advance();
    } else {
        error("Expected variable name after 'set'");
        return NULL;
    }

    /* Check for optional type declaration '** type' */
    ASTNode *type_node = NULL;
    if (match(TOKEN_STAR) && peek_token(1).type == TOKEN_STAR) {
        advance(); /* consume first '*' */
        advance(); /* consume second '*' */
        type_node = parse_type();
    }

    /* Expect '=' */
    expect(TOKEN_EQUALS, "Expected '=' in set statement");
    if (parser.error_count > 0) return NULL;

    /* Parse the value expression */
    ASTNode *value_node = parse_expression();
    if (!value_node) {
        error("Expected value expression after '='");
        return NULL;
    }

    /* Create the SET node with left and right children */
    ASTNode *set_node = create_node(NODE_SET);
    if (set_node) {
        set_node->left = var_node;
        set_node->right = value_node;
        if (type_node) {
            set_node->body = type_node; /* Store type in body for later use */
        }
        set_node->line = var_node ? var_node->line : 0;
        set_node->col = var_node ? var_node->col : 0;
    }

    return set_node;
}

/*
 * parse_kprintk - parse an output statement
 *
 * Syntax: kprintk expression
 *   - expression: value to print (variable, number, string, or expression)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_KPRINTK node
 */
ASTNode* parse_kprintk(void) {
    advance(); /* consume 'kprintk' */

    ASTNode *expr_node = parse_expression();
    if (!expr_node) {
        error("Expected expression after 'kprintk'");
        return NULL;
    }

    ASTNode *print_node = create_node(NODE_KPRINTK);
    if (print_node) {
        print_node->left = expr_node;
        print_node->line = expr_node->line;
        print_node->col = expr_node->col;
    }

    return print_node;
}

/*
 * parse_wreadw - parse an input statement
 *
 * Syntax: wreadw prompt
 *   - prompt: string literal or variable containing the prompt text
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_WREADW node
 */
ASTNode* parse_wreadw(void) {
    advance(); /* consume 'wreadw' */

    ASTNode *prompt_node = parse_expression();
    if (!prompt_node) {
        error("Expected prompt after 'wreadw'");
        return NULL;
    }

    ASTNode *input_node = create_node(NODE_WREADW);
    if (input_node) {
        input_node->left = prompt_node;
        input_node->line = prompt_node->line;
        input_node->col = prompt_node->col;
    }

    return input_node;
}

/*
 * parse_goto - parse a jump statement
 *
 * Syntax: goto label
 *   - label: identifier marking the target location
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_GOTO node
 */
ASTNode* parse_goto(void) {
    advance(); /* consume 'goto' */

    if (!match(TOKEN_ID)) {
        error("Expected label name after 'goto'");
        return NULL;
    }

    ASTNode *label_node = create_node(NODE_LABEL);
    if (label_node) {
        strcpy(label_node->value, current_token().value);
        label_node->line = current_token().line;
        label_node->col = current_token().col;
    }
    advance();

    ASTNode *goto_node = create_node(NODE_GOTO);
    if (goto_node) {
        goto_node->left = label_node;
        goto_node->line = label_node ? label_node->line : 0;
        goto_node->col = label_node ? label_node->col : 0;
    }

    return goto_node;
}

/*
 * parse_stop - parse a stop statement
 *
 * Syntax: stop
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_STOP node
 */
ASTNode* parse_stop(void) {
    advance(); /* consume 'stop' */

    ASTNode *stop_node = create_node(NODE_STOP);
    if (stop_node) {
        stop_node->line = 0;
        stop_node->col = 0;
    }

    return stop_node;
}

/*
 * parse_setexpception - parse an exception throw
 *
 * Syntax: setexpception message
 *   - message: string literal or variable containing the exception message
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_SETEXCEPTION node
 */
ASTNode* parse_setexpception(void) {
    advance(); /* consume 'setexpception' */

    ASTNode *msg_node = parse_expression();
    if (!msg_node) {
        error("Expected exception message after 'setexpception'");
        return NULL;
    }

    ASTNode *exc_node = create_node(NODE_SETEXCEPTION);
    if (exc_node) {
        exc_node->left = msg_node;
        exc_node->line = msg_node->line;
        exc_node->col = msg_node->col;
    }

    return exc_node;
}

/*
 * parse_repeat - parse a loop statement
 *
 * Syntax: repeat N buns ... bun
 *   - N: number of iterations (constant or variable)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_REPEAT node with the body block
 */
ASTNode* parse_repeat(void) {
    advance(); /* consume 'repeat' */

    ASTNode *count_node = parse_expression();
    if (!count_node) {
        error("Expected iteration count after 'repeat'");
        return NULL;
    }

    ASTNode *body_node = parse_block();
    if (!body_node) {
        error("Expected body block after 'repeat'");
        return NULL;
    }

    ASTNode *repeat_node = create_node(NODE_REPEAT);
    if (repeat_node) {
        repeat_node->left = count_node;
        repeat_node->right = body_node;
        repeat_node->line = count_node->line;
        repeat_node->col = count_node->col;
    }

    return repeat_node;
}

/*
 * parse_in - parse a conditional statement
 *
 * Syntax: in condition buns ... bun
 *   - condition: expression to evaluate (comparison or value)
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_IN node with the body block
 */
ASTNode* parse_in(void) {
    advance(); /* consume 'in' */

    ASTNode *cond_node = parse_expression();
    if (!cond_node) {
        error("Expected condition after 'in'");
        return NULL;
    }

    ASTNode *body_node = parse_block();
    if (!body_node) {
        error("Expected body block after 'in'");
        return NULL;
    }

    ASTNode *in_node = create_node(NODE_IN);
    if (in_node) {
        in_node->left = cond_node;
        in_node->right = body_node;
        in_node->line = cond_node->line;
        in_node->col = cond_node->col;
    }

    return in_node;
}

/*
 * parse_expression - parse an arithmetic expression
 *
 * Syntax: left op right
 *   - left: left operand (variable, number, string, or expression)
 *   - op: operator (+, -, *, /, %)
 *   - right: right operand (variable, number, string, or expression)
 *
 * This function handles operator precedence and associativity.
 * It currently supports the following operators (in order of precedence):
 *   1. * / %
 *   2. + -
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_BINARY_OP node
 */
ASTNode* parse_expression(void) {
    /* Parse the first term (left operand) */
    ASTNode *left = parse_term();
    if (!left) return NULL;

    /* Parse additional terms with operators */
    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
        Token op = current_token();
        advance(); /* consume operator */

        ASTNode *right = parse_term();
        if (!right) {
            error("Expected expression after operator");
            return left;
        }

        ASTNode *bin_node = create_node(NODE_BINARY_OP);
        if (bin_node) {
            strcpy(bin_node->value, op.value);
            bin_node->line = op.line;
            bin_node->col = op.col;
            bin_node->left = left;
            bin_node->right = right;
        }
        left = bin_node;
    }

    return left;
}

/*
 * parse_term - parse a term (multiplicative operations)
 *
 * Syntax: left op right
 *   - op: *, /, %
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_BINARY_OP node
 */
static ASTNode* parse_term(void) {
    /* Parse the first factor */
    ASTNode *left = parse_factor();
    if (!left) return NULL;

    /* Parse additional factors with multiplicative operators */
    while (match(TOKEN_STAR) || match(TOKEN_SLASH) || match(TOKEN_PERCENT)) {
        Token op = current_token();
        advance(); /* consume operator */

        ASTNode *right = parse_factor();
        if (!right) {
            error("Expected expression after operator");
            return left;
        }

        ASTNode *bin_node = create_node(NODE_BINARY_OP);
        if (bin_node) {
            strcpy(bin_node->value, op.value);
            bin_node->line = op.line;
            bin_node->col = op.col;
            bin_node->left = left;
            bin_node->right = right;
        }
        left = bin_node;
    }

    return left;
}

/*
 * parse_factor - parse a factor (basic elements)
 *
 * This function handles the basic elements of expressions:
 *   - Identifiers (variables)
 *   - Numbers (integer or floating point)
 *   - Strings (literal text)
 *   - Parenthesized expressions
 *
 * Returns:
 *   ASTNode* - pointer to the parsed factor node
 */
static ASTNode* parse_factor(void) {
    Token tok = current_token();

    if (match(TOKEN_ID)) {
        ASTNode *node = create_node(NODE_VARIABLE);
        if (node) {
            strcpy(node->value, tok.value);
            node->line = tok.line;
            node->col = tok.col;
        }
        advance();
        return node;
    } else if (match(TOKEN_NUMBER)) {
        ASTNode *node = create_node(NODE_NUMBER);
        if (node) {
            strcpy(node->value, tok.value);
            node->line = tok.line;
            node->col = tok.col;
        }
        advance();
        return node;
    } else if (match(TOKEN_STRING) || match(TOKEN_BYTES)) {
        ASTNode *node = create_node(NODE_STRING);
        if (node) {
            strcpy(node->value, tok.value);
            node->line = tok.line;
            node->col = tok.col;
        }
        advance();
        return node;
    } else {
        error("Expected expression (variable, number, or string)");
        advance(); /* Skip the unexpected token */
        return NULL;
    }
}

/*
 * parse_type - parse a type declaration
 *
 * Syntax: ** type
 *   - type: number, letter, or operator
 *
 * Returns:
 *   ASTNode* - pointer to a NODE_TYPE node
 */
ASTNode* parse_type(void) {
    if (!match(TOKEN_TYPE)) {
        error("Expected type (number, letter, or operator)");
        return NULL;
    }

    ASTNode *node = create_node(NODE_TYPE);
    if (node) {
        strcpy(node->value, current_token().value);
        node->line = current_token().line;
        node->col = current_token().col;
    }
    advance();

    return node;
}