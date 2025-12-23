/*
* word.c - CS50 Tiny Search Engine (TSE) Word Module
*
* Implements a function to normalize words by converting them to lowercase.
* This ensures consistency in indexing and searching operations.
*
* Author: Atziri Enriquez
* Date: 2/18/25
*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "word.h"
#include "../libcs50/mem.h"  // CS50 memory functions for safety

/**************** normalizeWord ****************/
/*
* Converts a given word to lowercase.
*
* The function takes a string (word) and returns a new dynamically allocated
* string containing the normalized version (all lowercase).
*
* Caller is responsible for freeing the returned string.
*
* Parameters:
*   - word: a null-terminated string to be normalized.
*
* Returns:
*   - A newly allocated lowercase string on success.
*   - NULL if the input is NULL or memory allocation fails.
*/
char* normalizeWord(const char* word)
{
  if (word == NULL) {
    return NULL;  // Defensive check for NULL input
  }

  int len = strlen(word);
  char* normalized = mem_malloc(len + 1);  // Use CS50 memory allocation
  if (normalized == NULL) {
    return NULL;  // Return NULL if allocation fails
  }

  for (int i = 0; i < len; i++) {
    normalized[i] = tolower((unsigned char) word[i]);  // Ensure safe casting
  }
  normalized[len] = '\0';  // Null-terminate the string

  return normalized;
}
