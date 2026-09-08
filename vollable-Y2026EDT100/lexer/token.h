#ifndef TOKEN_H
#define TOKEN_H

/*
 * token.h - Vollable Lexer Token Definitions
 *
 * This file defines all token types and the Token structure used
 * by the lexer. Each token represents a meaningful unit in the
 * Vollable programming language.
 *
 * Vollable is a mid-level language with keywords, types,
 * block syntax (buns/bun), and explicit typing via **.
 */

/*
 * TokenType - enumeration of all possible token kinds
 *
 * The lexer categorizes every input character sequence into one
 * of these types. This allows the parser to work with structured
 * tokens rather than raw text.
 */
typedef enum {
    TOKEN_EOF,              /* End of file marker */
    TOKEN_ID,               /* Identifier: variable names, labels, keywords */
    TOKEN_NUMBER,           /* Numeric literal: 42, 3.14, -17, 0xFF */
    TOKEN_STRING,           /* String literal: "Hello world" */
    TOKEN_BYTES,            /* Byte string literal: b"raw bytes" */
    TOKEN_TYPE,             /* Type keyword: number, letter, operator */
    TOKEN_START,            /* start keyword */
    TOKEN_SET,              /* set keyword */
    TOKEN_KPRINTK,          /* kprintk keyword (output) */
    TOKEN_WREADW,           /* wreadw keyword (input) */
    TOKEN_GOTO,             /* goto keyword */
    TOKEN_STOP,             /* stop keyword */
    TOKEN_SETEXCEPTION,     /* setexpception keyword */
    TOKEN_REPEAT,           /* repeat keyword */
    TOKEN_IN,               /* in keyword (condition) */
    TOKEN_BUNS,             /* buns - block open */
    TOKEN_BUN,              /* bun - block close */
    TOKEN_COLON,            /* : colon (obsolete, kept for compatibility) */
    TOKEN_EQUALS,           /* = assignment operator */
    TOKEN_PLUS,             /* + addition */
    TOKEN_MINUS,            /* - subtraction */
    TOKEN_STAR,             /* * multiplication */
    TOKEN_SLASH,            /* / division */
    TOKEN_PERCENT,          /* % modulo */
    TOKEN_ERROR             /* Invalid token (lexical error) */
} TokenType;

/*
 * Token - structure representing a single lexical unit
 *
 * Each token stores:
 * - type: the kind of token (from TokenType)
 * - value: the actual text of the token (as a string)
 * - line: source line number where the token appears
 * - col: source column number where the token starts
 *
 * This information is used for error reporting and debugging.
 */
typedef struct {
    TokenType type;         /* Type of this token */
    char value[128];        /* Raw text of the token */
    int line;               /* Source line number */
    int col;                /* Source column number */
} Token;

#endif