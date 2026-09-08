#ifndef UTILS_H
#define UTILS_H

#include "token.h"

/*
 * utils.h - Utility functions for the lexer
 *
 * This file declares utility functions used by the lexer for
 * file I/O, character classification, and token processing.
 */

/*
 * read_file - read an entire file into a string
 *
 * Parameters:
 *   filename - path to the file to read
 *
 * Returns:
 *   char* - pointer to allocated memory containing the file contents
 */
char* read_file(const char *filename);

/*
 * is_whitespace - check if a character is whitespace
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is whitespace, 0 otherwise
 */
int is_whitespace(char c);

/*
 * is_alpha_or_underscore - check if a character is a letter or underscore
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is a letter or '_', 0 otherwise
 */
int is_alpha_or_underscore(char c);

/*
 * is_number_char - check if a character is valid in a number
 *
 * Parameters:
 *   c - character to check
 *
 * Returns:
 *   1 if c is a digit, x, X, ., or -, 0 otherwise
 */
int is_number_char(char c);

/*
 * is_keyword - check if a string is a reserved keyword
 *
 * Parameters:
 *   value - string to check
 *
 * Returns:
 *   1 if the string is a keyword, 0 otherwise
 */
int is_keyword(const char *value);

/*
 * get_keyword_type - get the token type for a keyword
 *
 * Parameters:
 *   value - keyword string
 *
 * Returns:
 *   TokenType corresponding to the keyword
 */
TokenType get_keyword_type(const char *value);

#endif