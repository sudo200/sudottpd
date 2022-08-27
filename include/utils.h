#ifndef ___UTILS_H__
#define ___UTILS_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Gets the next character from the stream without removing it from the stream.
 * @param stream  The file stream to read from.
 * @return  The next character in the stream.
 */
int fpeek(FILE * stream);

/**
 * Gets n characters (excluding NULL-byte) from the stream without removing it from the stream.
 * @param stream  The file stream to read from.
 * @param n The amount of bytes to read.
 * @return  The read string
 */
char * fpeeks(FILE * stream, size_t n);

/**
 * Compares the next characters from stream with str without removing them from stream.
 * @param stream  The file stream to read from.
 * @param str The string to compare with.
 * @return  1 if true, 0 otherwise, negative value if error occured (see errno)
 */
int fpeekstrcmp(FILE * stream, const char * str);

/**
 * Returns the amount of bytes in a file.
 * @param stream  The file stream referring to the file.
 * @returns Amount of bytes in the file.
 */
size_t fsize(FILE * stream);

int vasprintf(char **str, const char *format, va_list args);

int asprintf(char **str, const char *format, ...);

char * strcata(char **dest, const char *src);

#endif//___UTILS_H__

