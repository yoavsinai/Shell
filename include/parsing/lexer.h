#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// Rewrites src into dst, inserting spaces around '<', '>' and '>>' so a
// plain whitespace split (e.g. strtok_r) treats them as standalone tokens
// even when the user wrote them with no surrounding spaces (`cat<file`).
void normalize_redirects(const char* src, char* dst, size_t dst_size);

// Replaces whitespace inside quoted spans ('...' or "...") with a
// placeholder byte, in place, so a later whitespace split treats the whole
// quoted phrase as a single token instead of splitting on it.
void mask_quoted_whitespace(char* line);

// Strips quote characters from a token and restores placeholder bytes
// (from mask_quoted_whitespace) back to real spaces, in place.
void unquote_token(char* token);

#endif
