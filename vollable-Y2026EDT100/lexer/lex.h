#ifndef LEX_H
#define LEX_H

/*
 * lex.h - Vollable Lexer Interface
 *
 * This file declares the lexer state structure and the public
 * functions used to tokenize a Vollable source file.
 *
 * The lexer reads the entire source code into memory and then
 * processes it character by character, producing a stream of
 * tokens that can be consumed by the parser.
 */

#include "token.h"

/*
 * Lexer - state structure for the lexical analyzer
 *
 * This structure holds all the state needed to scan a source file:
 * - source: pointer to the entire source code string
 * - pos: current reading position in the source
 * - len: total length of the source string
 * - line: current line number (for error reporting)
 * - col: current column number (for error reporting)
 *
 * The lexer maintains this state and updates it as it consumes
 * characters from the input.
 */
typedef struct {
    char *source;           /* Full source code string */
    int pos;                /* Current read position */
    int len;                /* Total source length */
    int line;               /* Current line number (1-based) */
    int col;                /* Current column number (1-based) */
} Lexer;

/*
 * Global lexer state - holds the current scanning context
 * This is used internally by the lexer functions.
 */
extern Lexer lexer;

/*
 * Global token array - stores all tokens after lexing
 * This is populated by lex_init() and used by the parser.
 */
extern Token tokens[];

/*
 * Global token count - number of tokens in the tokens array
 */
extern int token_count;

/*
 * lex_init - initialize the lexer with a source file
 *
 * Parameters:
 *   filename - path to the source file to tokenize
 *
 * This function reads the entire source file into memory,
 * initializes the lexer state, and then tokenizes the entire
 * source code, filling the tokens array.
 */
void lex_init(const char *filename);

/*
 * next_token - get the next token from the source stream
 *
 * Returns:
 *   Token - the next token parsed from the source code
 *
 * This function scans characters from the current position
 * and constructs the next token. It handles whitespace,
 * comments, keywords, identifiers, numbers, strings, and
 * all other token types.
 */
Token next_token(void);

/*
 * print_tokens - print all tokens for debugging purposes
 *
 * This function prints every token in the tokens array with
 * its type, value, and source position. Useful for debugging
 * the lexer or understanding how the source code is tokenized.
 */
void print_tokens(void);

#endif