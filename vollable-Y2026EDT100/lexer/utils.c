#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

/*
 * read_file - read entire file into a string
 *
 * Parameters:
 *   filename - path to the file to read
 *
 * Returns:
 *   char* - pointer to allocated memory containing the file contents
 *
 * This function opens the specified file, reads the entire content
 * into a dynamically allocated buffer, and returns it. It also
 * handles the case where the file is opened in binary mode.
 */
char* read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Cannot open file");
        exit(1);
    }

    /* Get the file size */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Allocate buffer */
    char *buf = (char*)malloc(size + 1);
    if (!buf) {
        perror("Memory allocation failed");
        fclose(f);
        exit(1);
    }

    /* Read the entire file */
    size_t bytes_read = fread(buf, 1, size, f);
    if (bytes_read != (size_t)size) {
        perror("Error reading file");
        free(buf);
        fclose(f);
        exit(1);
    }

    /* Null-terminate */
    buf[size] = '\0';

    fclose(f);
    return buf;
}

/*
 * is_whitespace - check if a character is a whitespace character
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is whitespace (space, tab, carriage return, newline), 0 otherwise
 */
int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/*
 * is_alpha_or_underscore - check if a character is a letter or underscore
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is a letter or '_', 0 otherwise
 *
 * This is used for parsing identifiers, labels, and keywords.
 */
int is_alpha_or_underscore(char c) {
    return isalpha(c) || c == '_';
}

/*
 * is_number_char - check if a character is a valid digit in a number
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is a digit, x, X, decimal point, or minus sign, 0 otherwise
 *
 * This allows numbers like:
 *   - 42 (decimal)
 *   - 3.14 (float)
 *   - 0xFF (hexadecimal)
 *   - -17 (negative)
 */
int is_number_char(char c) {
    return isxdigit(c) || c == 'x' || c == 'X' || c == '.' || c == '-';
}

/*
 * is_keyword - check if a string is a reserved keyword
 *
 * Parameters:
 *   value - string to check
 *
 * Returns:
 *   1 if the string is a reserved keyword, 0 otherwise
 */
int is_keyword(const char *value) {
    if (strcmp(value, "buns") == 0) return 1;
    if (strcmp(value, "bun") == 0) return 1;
    if (strcmp(value, "start") == 0) return 1;
    if (strcmp(value, "set") == 0) return 1;
    if (strcmp(value, "kprintk") == 0) return 1;
    if (strcmp(value, "wreadw") == 0) return 1;
    if (strcmp(value, "goto") == 0) return 1;
    if (strcmp(value, "stop") == 0) return 1;
    if (strcmp(value, "setexpception") == 0) return 1;
    if (strcmp(value, "repeat") == 0) return 1;
    if (strcmp(value, "in") == 0) return 1;
    if (strcmp(value, "number") == 0) return 1;
    if (strcmp(value, "letter") == 0) return 1;
    if (strcmp(value, "operator") == 0) return 1;
    return 0;
}

/*
 * get_keyword_type - get the token type for a keyword
 *
 * Parameters:
 *   value - keyword string
 *
 * Returns:
 *   TokenType corresponding to the keyword, or TOKEN_ID if not found
 *
 * This is used by the lexer to determine which specific token type
 * to assign to a keyword identifier.
 */
TokenType get_keyword_type(const char *value) {
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