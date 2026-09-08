#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"
#include "utils.h"

/*
 * MAX_TOKEN - maximum length of a token value
 * MAX_TOKENS - maximum number of tokens in the token array
 */
#define MAX_TOKEN 128
#define MAX_TOKENS 4096

/*
 * Global lexer state - initialized by lex_init()
 */
Lexer lexer;

/*
 * Global token array - filled by lex_init()
 */
Token tokens[MAX_TOKENS];

/*
 * Global token count - number of valid tokens in the token array
 */
int token_count = 0;

/*
 * is_keyword - check if a token value is a reserved keyword
 *
 * Parameters:
 *   value - string to check
 *
 * Returns:
 *   1 if the value is a keyword, 0 otherwise
 *
 * Keywords in Vollable are case-sensitive and cannot be used
 * as variable names or labels.
 */
static int is_keyword(const char *value) {
    /* Block keywords */
    if (strcmp(value, "buns") == 0) return 1;
    if (strcmp(value, "bun") == 0) return 1;

    /* Control flow keywords */
    if (strcmp(value, "start") == 0) return 1;
    if (strcmp(value, "set") == 0) return 1;
    if (strcmp(value, "kprintk") == 0) return 1;
    if (strcmp(value, "wreadw") == 0) return 1;
    if (strcmp(value, "goto") == 0) return 1;
    if (strcmp(value, "stop") == 0) return 1;
    if (strcmp(value, "setexpception") == 0) return 1;
    if (strcmp(value, "repeat") == 0) return 1;
    if (strcmp(value, "in") == 0) return 1;

    /* Type keywords */
    if (strcmp(value, "number") == 0) return 1;
    if (strcmp(value, "letter") == 0) return 1;
    if (strcmp(value, "operator") == 0) return 1;

    return 0;
}

/*
 * get_keyword_type - convert a keyword string to its TokenType
 *
 * Parameters:
 *   value - keyword string to convert
 *
 * Returns:
 *   TokenType corresponding to the keyword, or TOKEN_ID if not a keyword
 */
static TokenType get_keyword_type(const char *value) {
    if (strcmp(value, "buns") == 0) return TOKEN_BUNS;
    if (strcmp(value, "bun") == 0) return TOKEN_BUN;
    if (strcmp(value, "start") == 0) return TOKEN_START;
    if (strcmp(value, "set") == 0) return TOKEN_SET;
    if (strcmp(value, "kprintk") == 0) return TOKEN_KPRINTK;
    if (strcmp(value, "wreadw") == 0) return TOKEN_WREADW;
    if (strcmp(value, "goto") == 0) return TOKEN_GOTO;
    if (strcmp(value, "stop") == 0) return TOKEN_STOP;
    if (strcmp(value, "setexpception") == 0) return TOKEN_SETEXCEPTION;
    if (strcmp(value, "repeat") == 0) return TOKEN_REPEAT;
    if (strcmp(value, "in") == 0) return TOKEN_IN;
    if (strcmp(value, "number") == 0) return TOKEN_TYPE;
    if (strcmp(value, "letter") == 0) return TOKEN_TYPE;
    if (strcmp(value, "operator") == 0) return TOKEN_TYPE;
    return TOKEN_ID;
}

/*
 * parse_ident - parse an identifier or keyword token
 *
 * This function scans a sequence of alphanumeric characters and
 * underscores. It then checks if the result is a reserved keyword
 * and returns the appropriate token type.
 *
 * Returns:
 *   Token - either TOKEN_ID or a specific keyword token type
 */
static Token parse_ident(void) {
    Token t;
    t.type = TOKEN_ID;
    t.line = lexer.line;
    t.col = lexer.col;

    char buffer[MAX_TOKEN];
    int i = 0;

    /* Collect alphanumeric characters and underscores */
    while (isalnum(lexer.source[lexer.pos]) || lexer.source[lexer.pos] == '_') {
        if (i < MAX_TOKEN - 1) {
            buffer[i++] = lexer.source[lexer.pos];
        }
        lexer.pos++;
        lexer.col++;
    }
    buffer[i] = '\0';

    /* Copy to token value */
    for (int j = 0; j < i && j < 127; j++) {
        t.value[j] = buffer[j];
    }
    t.value[i] = '\0';

    /* Check if this is a keyword */
    if (is_keyword(buffer)) {
        t.type = get_keyword_type(buffer);
    }

    return t;
}

/*
 * parse_number - parse a numeric literal
 *
 * This function handles:
 * - Decimal integers: 42, -17, 0
 * - Floating point numbers: 3.14, -0.5
 * - Hexadecimal numbers: 0xFF, 0x10
 *
 * Returns:
 *   Token with type TOKEN_NUMBER
 */
static Token parse_number(void) {
    Token t;
    t.type = TOKEN_NUMBER;
    t.line = lexer.line;
    t.col = lexer.col;

    char buffer[MAX_TOKEN];
    int i = 0;

    /* Collect digits, hex chars, minus sign, decimal point */
    while (isxdigit(lexer.source[lexer.pos]) ||
           lexer.source[lexer.pos] == 'x' ||
           lexer.source[lexer.pos] == 'X' ||
           lexer.source[lexer.pos] == '.' ||
           lexer.source[lexer.pos] == '-') {
        if (i < MAX_TOKEN - 1) {
            buffer[i++] = lexer.source[lexer.pos];
        }
        lexer.pos++;
        lexer.col++;
    }
    buffer[i] = '\0';

    for (int j = 0; j < i && j < 127; j++) {
        t.value[j] = buffer[j];
    }
    t.value[i] = '\0';

    return t;
}

/*
 * parse_string - parse a string literal "text"
 *
 * This function scans characters between double quotes.
 * It does NOT support escape sequences (like \n) for simplicity.
 *
 * Returns:
 *   Token with type TOKEN_STRING
 */
static Token parse_string(void) {
    Token t;
    t.type = TOKEN_STRING;
    t.line = lexer.line;
    t.col = lexer.col;

    char buffer[MAX_TOKEN];
    int i = 0;

    lexer.pos++; /* Skip the opening double quote */

    /* Read until closing quote or newline */
    while (lexer.source[lexer.pos] != '"' &&
           lexer.source[lexer.pos] != '\n' &&
           lexer.source[lexer.pos] != '\0') {
        if (i < MAX_TOKEN - 1) {
            buffer[i++] = lexer.source[lexer.pos];
        }
        lexer.pos++;
        lexer.col++;
    }

    /* Skip the closing double quote if present */
    if (lexer.source[lexer.pos] == '"') {
        lexer.pos++;
        lexer.col++;
    }

    buffer[i] = '\0';

    for (int j = 0; j < i && j < 127; j++) {
        t.value[j] = buffer[j];
    }
    t.value[i] = '\0';

    return t;
}

/*
 * parse_bytes - parse a byte string literal b"raw bytes"
 *
 * This function works like parse_string but expects a 'b' prefix
 * before the opening quote. It stores the raw bytes without any
 * interpretation (no character encoding applied).
 *
 * Returns:
 *   Token with type TOKEN_BYTES
 */
static Token parse_bytes(void) {
    Token t;
    t.type = TOKEN_BYTES;
    t.line = lexer.line;
    t.col = lexer.col;

    char buffer[MAX_TOKEN];
    int i = 0;

    lexer.pos += 2; /* Skip b" */

    /* Read until closing quote */
    while (lexer.source[lexer.pos] != '"' &&
           lexer.source[lexer.pos] != '\n' &&
           lexer.source[lexer.pos] != '\0') {
        if (i < MAX_TOKEN - 1) {
            buffer[i++] = lexer.source[lexer.pos];
        }
        lexer.pos++;
        lexer.col++;
    }

    if (lexer.source[lexer.pos] == '"') {
        lexer.pos++;
        lexer.col++;
    }

    buffer[i] = '\0';

    for (int j = 0; j < i && j < 127; j++) {
        t.value[j] = buffer[j];
    }
    t.value[i] = '\0';

    return t;
}

/*
 * parse_type_declaration - parse a type declaration after **
 *
 * This function handles the explicit type declaration syntax:
 *   set name ** type = value
 * It reads the type keyword (number, letter, operator) and returns
 * it as a TOKEN_TYPE token.
 *
 * Returns:
 *   Token with type TOKEN_TYPE
 */
static Token parse_type_declaration(void) {
    Token t;
    t.type = TOKEN_TYPE;
    t.line = lexer.line;
    t.col = lexer.col;

    char buffer[MAX_TOKEN];
    int i = 0;

    /* Skip whitespace after ** */
    while (is_whitespace(lexer.source[lexer.pos])) {
        lexer.pos++;
        lexer.col++;
    }

    /* Read the type name */
    while (isalnum(lexer.source[lexer.pos]) || lexer.source[lexer.pos] == '_') {
        if (i < MAX_TOKEN - 1) {
            buffer[i++] = lexer.source[lexer.pos];
        }
        lexer.pos++;
        lexer.col++;
    }
    buffer[i] = '\0';

    /* Only valid types are: number, letter, operator */
    if (strcmp(buffer, "number") != 0 &&
        strcmp(buffer, "letter") != 0 &&
        strcmp(buffer, "operator") != 0) {
        strcpy(t.value, "invalid_type");
    } else {
        for (int j = 0; j < i && j < 127; j++) {
            t.value[j] = buffer[j];
        }
        t.value[i] = '\0';
    }

    return t;
}

/*
 * next_token - main tokenization function
 *
 * This function implements a simple state machine that reads
 * characters from the source and identifies the next token.
 *
 * It handles:
 * - Whitespace (skipped)
 * - Comments (skipped until end of line)
 * - Block keywords (buns, bun)
 * - Control flow keywords (start, set, kprintk, wreadw, etc.)
 * - Type keywords (number, letter, operator)
 * - Identifiers
 * - Numeric literals
 * - String literals
 * - Byte string literals
 * - Symbols (+, -, *, /, %, =)
 * - Explicit type declarations (type after **)
 *
 * Returns:
 *   Token - the next token, or TOKEN_EOF at end of file
 */
Token next_token(void) {
    Token t;
    t.type = TOKEN_EOF;
    t.value[0] = '\0';
    t.line = lexer.line;
    t.col = lexer.col;

    while (lexer.pos < lexer.len) {
        char c = lexer.source[lexer.pos];

        /* --- Skip whitespace --- */
        if (is_whitespace(c)) {
            if (c == '\n') {
                lexer.line++;
                lexer.col = 0;
            }
            lexer.pos++;
            lexer.col++;
            continue;
        }

        /* --- Skip comments (; until end of line) --- */
        if (c == ';') {
            while (lexer.pos < lexer.len && lexer.source[lexer.pos] != '\n') {
                lexer.pos++;
            }
            continue;
        }

        /* --- Handle numeric literals --- */
        if (isdigit(c) || (c == '-' && isdigit(lexer.source[lexer.pos + 1]))) {
            return parse_number();
        }

        /* --- Handle string literals --- */
        if (c == '"') {
            return parse_string();
        }

        /* --- Handle byte string literals b"..." --- */
        if (c == 'b' && lexer.source[lexer.pos + 1] == '"') {
            return parse_bytes();
        }

        /* --- Handle identifiers and keywords --- */
        if (isalpha(c) || c == '_') {
            return parse_ident();
        }

        /* --- Handle explicit type declaration after ** --- */
        if (c == '*' && lexer.source[lexer.pos + 1] == '*') {
            lexer.pos += 2;
            lexer.col += 2;
            return parse_type_declaration();
        }

        /* --- Handle single-character symbols --- */
        if (c == '=') {
            t.type = TOKEN_EQUALS;
            t.value[0] = '=';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        if (c == '+') {
            t.type = TOKEN_PLUS;
            t.value[0] = '+';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        if (c == '-') {
            t.type = TOKEN_MINUS;
            t.value[0] = '-';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        if (c == '*') {
            t.type = TOKEN_STAR;
            t.value[0] = '*';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        if (c == '/') {
            t.type = TOKEN_SLASH;
            t.value[0] = '/';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        if (c == '%') {
            t.type = TOKEN_PERCENT;
            t.value[0] = '%';
            t.value[1] = '\0';
            t.line = lexer.line;
            t.col = lexer.col;
            lexer.pos++;
            lexer.col++;
            return t;
        }

        /* --- Unrecognized character --- */
        char msg[128];
        sprintf(msg, "Unknown character '%c'", c);
        fprintf(stderr, "Lexical error [%d:%d]: %s\n", lexer.line, lexer.col, msg);

        t.type = TOKEN_ERROR;
        for (int i = 0; msg[i] && i < 127; i++) {
            t.value[i] = msg[i];
        }
        t.line = lexer.line;
        t.col = lexer.col;
        lexer.pos++;
        return t;
    }

    /* End of file */
    t.type = TOKEN_EOF;
    char eof[] = "EOF";
    for (int i = 0; eof[i] && i < 127; i++) {
        t.value[i] = eof[i];
    }
    return t;
}

/*
 * lex_init - initialize the lexer and tokenize the entire source file
 *
 * Parameters:
 *   filename - path to the source file to tokenize
 *
 * This function reads the entire file into memory, initializes the
 * lexer state, and then tokenizes the entire source code. All tokens
 * are stored in the global tokens array.
 */
void lex_init(const char *filename) {
    /* Read the entire source file */
    lexer.source = read_file(filename);
    lexer.len = (int)strlen(lexer.source);
    lexer.pos = 0;
    lexer.line = 1;
    lexer.col = 0;
    token_count = 0;

    /* Tokenize the entire source */
    Token t;
    do {
        t = next_token();
        tokens[token_count++] = t;
    } while (t.type != TOKEN_EOF && t.type != TOKEN_ERROR);
}

/*
 * print_tokens - print all tokens for debugging
 *
 * This function is primarily used for debugging the lexer.
 * It prints each token's type, value, and source position.
 */
void print_tokens(void) {
    const char *type_names[] = {
        "EOF", "ID", "NUMBER", "STRING", "BYTES",
        "TYPE", "START", "SET", "KPRINTK", "WREADW",
        "GOTO", "STOP", "SETEXCEPTION", "REPEAT", "IN",
        "BUNS", "BUN", "COLON", "EQUALS", "PLUS",
        "MINUS", "STAR", "SLASH", "PERCENT", "ERROR"
    };

    printf("\n=== VOLLABLE TOKENS ===\n");
    for (int i = 0; i < token_count; i++) {
        printf("[%s] \"%s\" (line %d, col %d)\n",
               type_names[tokens[i].type],
               tokens[i].value,
               tokens[i].line,
               tokens[i].col);
    }
    printf("Total tokens: %d\n", token_count);
    printf("========================\n\n");
}