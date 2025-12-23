/*
 * word.h - CS50 Tiny Search Engine (TSE) Word Module
 *
 * Provides a utility function to normalize words by converting them to lowercase.
 * This is used in the Indexer and Querier components to ensure consistency.
 *
 * Author: Atziri Enriquez
 * Date: 2/18/25
 */

 #ifndef __WORD_H
 #define __WORD_H
 
 #include <ctype.h>
 
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
 char* normalizeWord(const char* word);
 
 #endif // __WORD_H
 