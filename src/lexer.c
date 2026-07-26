#include "lexer.h"

void normalize_redirects(const char* src, char* dst, size_t dst_size)
{
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 4 < dst_size; i++) {
        char c = src[i];
        if (c == '<' || c == '>') {
            dst[j++] = ' ';
            dst[j++] = c;
            if (c == '>' && src[i + 1] == '>') {
                dst[j++] = '>';
                i++;
            }
            dst[j++] = ' ';
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

#define QUOTE_PLACEHOLDER '\x01'

void mask_quoted_whitespace(char* line)
{
    char quote = 0;
    for (; *line != '\0'; line++) {
        if (!quote && (*line == '"' || *line == '\'')) {
            quote = *line;
        } else if (quote && *line == quote) {
            quote = 0;
        } else if (quote && (*line == ' ' || *line == '\t')) {
            *line = QUOTE_PLACEHOLDER;
        }
    }
}

void unquote_token(char* token)
{
    char* read = token;
    char* write = token;
    while (*read != '\0') {
        if (*read == '"' || *read == '\'') {
            read++;
        } else if (*read == QUOTE_PLACEHOLDER) {
            *write++ = ' ';
            read++;
        } else {
            *write++ = *read++;
        }
    }
    *write = '\0';
}
