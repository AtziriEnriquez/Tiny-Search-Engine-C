/*
* querier.c
* Author: Atziri Enriquez
* Date: 23 February 2025
*
* This program reads an index file produced by the TSE Indexer and page files 
* produced by the TSE Crawler. It processes search queries from stdin, evaluates 
* them using logical AND/OR operations, and prints ranked search results to stdout.
*
* The querier:
* - Parses and validates command-line arguments.
* - Loads the index file into memory.
* - Reads queries from stdin, tokenizes and validates them.
* - Evaluates the query using intersection (AND) and union (OR) logic.
* - Sorts the matching documents by score in descending order.
* - Prints the search results along with URLs.
*
* Usage:
*   ./querier pageDirectory indexFilename
*
* Example:
*   ./querier data/toscrape-2 data/toscrape-2.index
*
* Exit codes:
* - 0: Success
* - 1: Invalid arguments (wrong number of arguments, invalid page directory, etc.)
* - 2: Index file could not be opened or loaded.
*
* Dependencies:
* - Requires counters, index, and memory management modules.
* - Uses `qsort()` to rank search results.
* - Assumes the provided index file is well-formed.
*
*/

#define _POSIX_C_SOURCE 200809L  // Enables getline() and fileno() in a portable way
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../libcs50/counters.h"
#include "../libcs50/mem.h"
#include "../libcs50/file.h"
#include "../common/index.h"
#include "../common/pagedir.h"

// Function prototypes
static void prompt(void);
static void parseArgs(int argc, char *argv[], const char **pageDirectory, const char **indexFilename);
static void matchMerge(counters_t **andSequence, counters_t **orSequence);

int queryTokenize(char *query, char *words[], int maxWords);
bool isValidQuerySyntax(char **words, int n);
bool isValidCharacters(const char *query);
void processQuery(char *query, index_t *index, const char *pageDirectory);
void intersectCounters(counters_t *acc, counters_t *wordCounters);
void unionCounters(counters_t *result, counters_t *andResult);
counters_t* queryEvaluate(char **words, int n, index_t *index);
int compareScores(const void *a, const void *b);
void printRankedResults(counters_t *result, const char *pageDirectory);

// Callback functions
void callbackSetMinimum(void *arg, const int key, const int count);
void callbackAddCounts(void *arg, const int key, const int count);
void callbackCountNonZero(void *arg, const int key, const int count);
void callbackStoreDocScores(void *arg, const int key, const int count);

// Struct to hold document ID and its score for sorting
typedef struct {
  int docID;
  int score;
} docScore_t;

/* 
 * prompt - Prints "Query? " only if stdin is a terminal
 * 
 * This function checks whether the program's standard input (stdin) 
 * is connected to a terminal. If so, it prints "Query? " as a prompt.
 * 
 * This ensures that when the program is running interactively, 
 * the user receives a visual cue to enter input.
 * 
 * However, if stdin is redirected from a file or another program, 
 * the prompt is suppressed to prevent unnecessary output.
 * 
 * Parameters:
 * - None
 * 
 * Returns:
 * - None (prints to stdout if applicable)
 */
static void prompt(void)
{
  // Step 1: Check if standard input (stdin) is from a terminal
  if (isatty(fileno(stdin))) {
    printf("Query? ");  // Print prompt for user input
    fflush(stdout);     // Ensure output is immediately displayed
  }
}

/* 
 * parseArgs - Validates the command-line arguments
 * 
 * This function checks that the correct number of arguments is provided, 
 * validates the page directory, and ensures the index file can be opened.
 * 
 * If any validation fails, the function prints an error message and exits 
 * with a nonzero status.
 * 
 * Parameters:
 * - argc: The number of command-line arguments.
 * - argv: The array of argument strings.
 * - pageDirectory: A pointer to store the validated page directory path.
 * - indexFilename: A pointer to store the validated index file path.
 * 
 * Returns:
 * - None (exits on failure).
 */
static void parseArgs(int argc, char *argv[], const char **pageDirectory, const char **indexFilename)
{
  // Step 1: Check that exactly 2 arguments are provided (excluding program name)
  if (argc != 3) {
    fprintf(stderr, "Usage: ./querier pageDirectory indexFilename\n");
    exit(1);
  }

  // Step 2: Store arguments in the provided pointers
  *pageDirectory = argv[1];
  *indexFilename = argv[2];

  // Step 3: Validate that the provided page directory is valid
  if (!pagedir_validate(*pageDirectory)) {
    fprintf(stderr, "Error: Invalid page directory: %s\n", *pageDirectory);
    exit(1);
  }

  // Step 4: Attempt to open the index file for reading to check its validity
  FILE *indexFp = fopen(*indexFilename, "r");
  if (indexFp == NULL) {
    fprintf(stderr, "Error: Could not open index file: %s\n", *indexFilename);
    exit(1);
  }
  fclose(indexFp); // Close file after successful validation
}

/* 
 * main - Entry point of the querier program
 * 
 * This function:
 * 1. Parses and validates command-line arguments.
 * 2. Loads the index file into memory.
 * 3. Reads user queries in a loop, processing each one.
 * 4. Cleans up memory before exiting.
 * 
 * Parameters:
 * - argc: The number of command-line arguments.
 * - argv: The array of argument strings.
 * 
 * Returns:
 * - 0 on successful execution.
 */
int main(int argc, char *argv[])
{
  // Step 1: Parse and validate command-line arguments
  const char *pageDirectory;
  const char *indexFilename;
  parseArgs(argc, argv, &pageDirectory, &indexFilename);

  // Step 2: Open the index file and load it into memory
  FILE *indexFile = fopen(indexFilename, "r");
  index_t *index = mem_assert(index_load(indexFile), "index_load failed"); 
  // index_load reads the index file and allocates memory to store it
  // technically, we already check for file failure in parseArgs, consider this a sanity check

  fclose(indexFile); // Close the file after loading the index

  // Step 3: Read user queries in a loop
  char *query = NULL;
  size_t len = 0;
  
  // Continuously prompt the user for queries and process them
  while (prompt(), getline(&query, &len, stdin) != -1) {
    processQuery(query, index, pageDirectory);
    printf("-----------------------------------------------\n");
  }

  // Step 4: Free allocated memory before exiting
  free(query);         // Free dynamically allocated query buffer
  index_delete(index); // Free allocated index structure

  return 0; // Indicate successful execution
}

/* 
 * processQuery - Parses, validates, and evaluates a query
 * 
 * This function processes a raw query string by:
 * 1. Validating its characters.
 * 2. Tokenizing it into words.
 * 3. Validating its syntax.
 * 4. Evaluating the query using AND/OR logic.
 * 5. Printing the ranked results.
 * 
 * Parameters:
 * - query: The input query string.
 * - index: A pointer to the index structure for word-document mappings.
 * - pageDirectory: The directory containing the crawled pages.
 * 
 * Returns:
 * - None (outputs query results or errors).
 */
void processQuery(char *query, index_t *index, const char *pageDirectory)
{
  // Step 1: Validate that the query contains only valid characters (letters and spaces)
  if (!isValidCharacters(query)) {
    return;  // If invalid, exit immediately
  }

  // Step 2: Estimate the maximum possible number of words in the query
  // A worst-case assumption is that every other character is a space, so max words = strlen(query) / 2 + 1
  int maxWords = strlen(query) / 2 + 1;
  char **words = mem_malloc(maxWords * sizeof(char*));       // Allocate memory for words array
  mem_assert(words, "Memory allocation for words failed.");  // Ensure memory allocation was successful

  // Step 3: Tokenize the query into words and count the number of tokens
  int t = queryTokenize(query, words, maxWords);
  
  // If no tokens were found, return without processing further
  if (t == 0) {
    return;
  }

  // Step 4: Validate query syntax (ensures proper use of "AND"/"OR")
  if (!isValidQuerySyntax(words, t)) {
    return;
  }

  // Step 5: Print the formatted query for logging
  printf("Query:");
  for (int i = 0; i < t; i++) {
    printf(" %s", words[i]);
  }
  printf("\n");

  // Step 6: Evaluate the query and retrieve matching documents
  counters_t *result = queryEvaluate(words, t, index);

  // Step 7: Print the ranked results of the query
  printRankedResults(result, pageDirectory);

  // Step 8: Free memory used by the result counters
  counters_delete(result);
}

/* 
 * matchMerge - Merges an AND sequence into an OR sequence
 * 
 * This function takes an `andSequence` (representing a set of documents 
 * that match all words in an AND query) and merges it into `orSequence`, 
 * which accumulates results for an OR query.
 * 
 * Behavior:
 * - If `andSequence` is NULL, nothing happens.
 * - If `orSequence` is NULL, it is initialized as a new counters set.
 * - The counts from `andSequence` are merged into `orSequence` using `unionCounters()`.
 * - `andSequence` is then deleted to free memory.
 * 
 * Parameters:
 * - andSequence: A pointer to the counters representing an AND sequence.
 * - orSequence: A pointer to the counters representing an OR sequence.
 * 
 * Returns:
 * - None (modifies `orSequence` in place and frees `andSequence`).
 */
static void matchMerge(counters_t **andSequence, counters_t **orSequence)
{
  // Check if andSequence exists before merging
  if (*andSequence != NULL) {
    
    // If orSequence is NULL, create a new counters set
    if (*orSequence == NULL) {
      *orSequence = mem_assert(counters_new(), "matchMerge: Failed to allocate memory for orSequence");
    }

    // Merge the counts from andSequence into orSequence using OR logic
    unionCounters(*orSequence, *andSequence);

    // Free andSequence memory after merging, since it is no longer needed
    counters_delete(*andSequence);
    *andSequence = NULL; // Prevent dangling pointer
  }
}

/* 
 * unionCounters - Combines two counters using OR logic
 * 
 * This function merges the counts from `andResult` into `result`. 
 * It performs an "OR" operation, meaning:
 * - If a key exists in both counters, their counts are added.
 * - If a key exists only in `andResult`, it is added to `result` with its count.
 * - If a key exists only in `result`, it remains unchanged.
 * 
 * Parameters:
 * - result: The accumulator counters where counts will be combined.
 * - andResult: The counters to merge into `result`.
 * 
 * Returns:
 * - None (modifies `result` in place).
 */
void unionCounters(counters_t *result, counters_t *andResult)
{
  // Ensure both counters are valid before proceeding
  if (result == NULL || andResult == NULL) return;

  // Iterate through `andResult`, adding counts to `result`
  counters_iterate(andResult, result, callbackAddCounts);
}

/* 
 * intersectCounters - Combines two counters using AND logic
 * 
 * This function modifies `acc` (accumulator) to retain only the minimum 
 * count for each key that exists in both `acc` and `wordCounters`. 
 * This implements an "AND" operation in the sense that a key must be 
 * present in both counters to be kept, and its final count is the minimum 
 * of the two.
 * 
 * Parameters:
 * - acc: The accumulator counters (updated in-place).
 * - wordCounters: The counters to compare with `acc`.
 * 
 * Returns:
 * - None (modifies `acc` in place).
 */
void intersectCounters(counters_t *acc, counters_t *wordCounters)
{
  // Ensure valid memory before proceeding
  mem_assert(acc, "intersectCounters: acc is NULL");
  mem_assert(wordCounters, "intersectCounters: wordCounters is NULL");
  
  // Create a struct to pass both counters as arguments to the callback function
  struct {
    counters_t *acc;          // The accumulator to be updated
    counters_t *wordCounters; // The second set of counters for comparison
  } args = {acc, wordCounters};

  // Iterate over `acc` and update values based on `wordCounters`
  counters_iterate(acc, &args, callbackSetMinimum);
}

/* 
 * queryEvaluate - Evaluates the query using AND/OR logic
 * 
 * This function processes a query represented as an array of words and evaluates 
 * it using logical AND/OR operations. It retrieves word-document mappings from 
 * the index and determines the final set of matching documents.
 * 
 * Logic:
 * - Words separated by "AND" are intersected (documents must match all).
 * - Groups of "AND" sequences are combined with "OR" (documents can match any).
 * - If a word is not found in the index, the AND sequence is invalidated.
 * - At the end, the function returns a counters_t structure with the final results.
 * 
 * Parameters:
 * - words: Array of words representing the query.
 * - t: The number of words in the query.
 * - index: The index structure storing word-document mappings.
 * 
 * Returns:
 * - A counters_t* containing the merged document matches.
 */
counters_t* queryEvaluate(char **words, int t, index_t *index)
{
  counters_t *andSequence = NULL;  // Holds the result of consecutive "AND" operations
  counters_t *orSequence = NULL;   // Holds the result of merging multiple "AND" sequences with "OR"
  bool andSequenceInvalid = false; // Tracks if a word results in an empty intersection, meaning no match

  for (int i = 0; i < t; i++) {
    // If the word is "or", merge the current AND sequence into OR sequence
    if (strcmp(words[i], "or") == 0) {
      matchMerge(&andSequence, &orSequence);
      andSequenceInvalid = false; // Reset since a new OR sequence is starting
      continue;
    }

    // If the previous words resulted in a complete failure, skip processing further
    if (andSequenceInvalid){
      continue;
    } 

    // If the word is "and", do nothing and continue
    if (strcmp(words[i], "and") == 0){
      continue;
    }

    // Find the counters structure for the current word in the index
    counters_t *wordMatch = index_find(index, words[i]);
    // If the word is NOT found in the index, the entire AND sequence is invalid
    if (wordMatch == NULL) {
      andSequenceInvalid = true;
      if (andSequence != NULL) { // Free any existing AND sequence since it's now irrelevant
        counters_delete(andSequence);
        andSequence = NULL;
      }
    } else {
      // If this is the first word of a new AND sequence, initialize a new counters structure
      if (andSequence == NULL) {
        andSequence = counters_new();
        mem_assert(andSequence, "Error: Out of memory for andSequence counter.");
        counters_iterate(wordMatch, andSequence, callbackAddCounts); // Copy counts from wordMatch
      } else {
        // Otherwise, perform an intersection between the existing AND sequence and the current word’s matches
        intersectCounters(andSequence, wordMatch);
      }
    }
  }

  // Merge any remaining AND sequence into the OR sequence before returning the result
  matchMerge(&andSequence, &orSequence);

  // Return the final OR sequence containing all matched documents
  return orSequence; 
}

/* 
 * queryTokenize - Tokenizes input query into words
 * 
 * This function takes a query string and splits it into individual words, 
 * storing pointers to each word in the `words[]` array. It also converts 
 * all characters to lowercase for case-insensitive processing.
 * 
 * Parameters:
 * - query: The input query string to be tokenized.
 * - words: An array of character pointers to store tokenized words.
 * - maxWords: The maximum number of words allowed in the query.
 * 
 * Returns:
 * - The number of words successfully tokenized.
 */
int queryTokenize(char *query, char *words[], int maxWords)
{
  int t = 0;           // Counter for the number of words stored
  char *start = query; // Pointer to traverse the query string

  while (*start) { // Iterate through the string until the null terminator
    // Skip leading whitespace
    while (isspace(*start)) start++;

    // If we reach the end of the string, break out of the loop
    if (*start == '\0') {
      break;
    } 

    char *end = start; // Pointer to find the end of the current word

    // Convert the word to lowercase and find the end of the word
    while (*end && !isspace(*end)) {
      *end = tolower(*end); // Convert character to lowercase
      end++;
    }

    // Store the word's starting address in the words array
    if (t < maxWords) {
      words[t++] = start; // Store the start of the word and increment counter
    } else {
      break; // Stop processing if the word limit is exceeded
    }

    // Null-terminate the word and move to the next word
    if (*end) *end++ = '\0';

    start = end; // Continue parsing from the next character
  }

  return t; // Return the number of words found
}

/* 
 * isValidCharacters - Checks if query contains only valid characters
 * 
 * This function verifies that a query string consists only of:
 * - Alphabetic characters (A-Z, a-z)
 * - Whitespace characters (spaces, tabs, newlines)
 * 
 * If an invalid character is found, an error message is printed, 
 * and the function returns `false`. Otherwise, it returns `true`.
 * 
 * Parameters:
 * - query: The input query string to be validated.
 * 
 * Returns:
 * - `true` if the query contains only valid characters.
 * - `false` if an invalid character is found.
 */
bool isValidCharacters(const char *query)
{
  // Iterate through each character in the query string
  for (int i = 0; query[i] != '\0'; i++) {
    // If the character is not a letter and not whitespace, it's invalid
    if (!isalpha(query[i]) && !isspace(query[i])) {
      fprintf(stderr, "Error: bad character '%c' in query.\n", query[i]);
      return false;  // Invalid character found
    }
  }

  return true;  // All characters are valid
}

/* 
 * isValidQuerySyntax - Validates the query syntax
 * 
 * This function ensures that a tokenized query follows proper logical structure.
 * Specifically, it checks for:
 * - The presence of at least one valid word.
 * - No leading or trailing logical operators ("and" or "or").
 * - No consecutive occurrences of "and" or "or".
 * 
 * If the query is syntactically incorrect, an error message is printed, 
 * and the function returns `false`. Otherwise, it returns `true`.
 * 
 * Parameters:
 * - words: An array of tokenized words from the query.
 * - t: The number of words in the query.
 * 
 * Returns:
 * - `true` if the query syntax is valid.
 * - `false` if a syntax error is detected.
 */
bool isValidQuerySyntax(char **words, int t)
{
  // Step 1: If there are no tokens (empty query), return false
  if (t == 0) return false; 

  // Step 2: Ensure the query does not start with "and" or "or"
  if (strcmp(words[0], "and") == 0 || strcmp(words[0], "or") == 0) {
    fprintf(stderr, "Error: '%s' cannot be first\n", words[0]);
    return false;
  }

  // Step 3: Ensure the query does not end with "and" or "or"
  if (strcmp(words[t - 1], "and") == 0 || strcmp(words[t - 1], "or") == 0) {
    fprintf(stderr, "Error: '%s' cannot be last\n", words[t - 1]);
    return false;
  }

  // Step 4: Loop through words to check for consecutive "and" or "or"
  for (int i = 1; i < t; i++) {
    if ((strcmp(words[i], "and") == 0 || strcmp(words[i], "or") == 0) &&
        (strcmp(words[i - 1], "and") == 0 || strcmp(words[i - 1], "or") == 0)) {
      fprintf(stderr, "Error: Consecutive '%s' and '%s' are not allowed.\n", words[i - 1], words[i]);
      return false;
    }
  }

  // If all checks pass, return true (valid syntax)
  return true;
}

/* 
 * compareScores - Comparison function for qsort (sorts in descending order)
 * 
 * This function is used by `qsort()` to sort an array of `docScore_t` structures 
 * in **descending order** based on their `score` values.
 * 
 * Sorting in descending order ensures that documents with the **highest relevance scores**
 * appear first in the ranked results.
 * 
 * Parameters:
 * - a: Pointer to the first `docScore_t` element.
 * - b: Pointer to the second `docScore_t` element.
 * 
 * Returns:
 * - A negative value if `b`'s score is greater (placing `b` before `a`).
 * - Zero if the scores are equal.
 * - A positive value if `a`'s score is greater (placing `a` before `b`).
 */
int compareScores(const void *a, const void *b) {
  return ((docScore_t *)b)->score - ((docScore_t *)a)->score;
}

/* 
 * printRankedResults - Sorts and prints query results
 * 
 * This function:
 * - Counts the number of documents with nonzero scores.
 * - Stores the document scores in an array.
 * - Sorts the array in descending order based on score.
 * - Prints the ranked results, displaying document IDs and URLs.
 * 
 * Parameters:
 * - result: A `counters_t` structure containing document scores.
 * - pageDirectory: The directory where crawled pages are stored.
 * 
 * Returns:
 * - None (outputs results to stdout).
 */
void printRankedResults(counters_t *result, const char *pageDirectory)
{
  int printedCount = 0;

  // Step 1: Count the number of documents with non-zero scores
  counters_iterate(result, &printedCount, callbackCountNonZero);

  // Step 2: If no documents match, print message and return
  if (printedCount == 0) {
    printf("No documents match.\n");
    return;
  }

  printf("Matches %d documents (ranked):\n", printedCount);

  // Step 3: Allocate memory for storing document scores
  docScore_t *docScores = mem_assert(mem_malloc(printedCount * sizeof(docScore_t)), "Memory allocation for docScores failed.");

  // Step 4: Create a structure to pass both docScores and count to callback
  struct {
    docScore_t *docScores;
    int docCount;  // Tracks index of stored scores
  } scoreList = {docScores, 0};

  // Step 5: Store document scores in the array using counters_iterate
  counters_iterate(result, &scoreList, callbackStoreDocScores);

  // Step 6: Sort documents by score in descending order
  qsort(scoreList.docScores, printedCount, sizeof(docScore_t), compareScores);

  // Step 7: Print sorted results, retrieving URLs for each document
  for (int i = 0; i < printedCount; i++) {
    char filename[256];

    // Construct the filename based on pageDirectory and document ID
    snprintf(filename, sizeof(filename), "%s/%d", pageDirectory, scoreList.docScores[i].docID);

    // Open the file to read the URL
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
      fprintf(stderr, "Error: could not open file %s\n", filename);
      exit(1);
    }

    // Read the first line, which contains the URL
    char *url = file_readLine(file);
    fclose(file);

    // Print document score, ID, and URL
    printf("score %d doc %d: %s\n", scoreList.docScores[i].score, scoreList.docScores[i].docID, url);
    free(url);
  }

  // Step 8: Free allocated memory
  free(scoreList.docScores);
}

/* Callback Functions */
/* 
 * callbackSetMinimum - Sets the minimum count for matching keys in two counters
 * 
 * This function is used as a callback in `counters_iterate()`. It compares the 
 * counts of a specific key in two `counters_t` structures: 
 * - `acc` (the accumulator, storing the results)
 * - `wordCounters` (the new set of counts to be merged)
 * 
 * The function ensures that for each key, the minimum count between `acc` and 
 * `wordCounters` is retained in `acc`. This effectively performs an "AND" 
 * operation, ensuring that a document's score is limited by the lowest 
 * matching count across all words in the query.
 * 
 * Parameters:
 * - arg: A pointer to a struct containing `acc` and `wordCounters`.
 * - key: The document ID being processed.
 * - count: The count associated with this key in `acc`.
 * 
 * Returns:
 * - None (modifies `acc` in place).
 */
void callbackSetMinimum(void *arg, const int key, const int count)
{
  // Step 1: Cast arg to a pointer to the struct containing both counters
  struct { 
    counters_t *acc;          // The accumulator storing final results
    counters_t *wordCounters; // The new set of counters to compare with
  } *args = arg;              // cast arg back into the struct type so you can access acc and wordCounters correctly

  // Step 2: Get the count for this key from wordCounters
  int c = counters_get(args->wordCounters, key);

  // Step 3: Update c to the minimum count between acc and wordCounters
  if (count < c) {
    c = count;
  }

  // Step 4: Set the minimum value in acc
  counters_set(args->acc, key, c);
}

/* 
 * callbackAddCounts - Adds counts from one counters_t to another 
 * 
 * This function is used as a callback in `counters_iterate()`. It adds the count 
 * of a given key from the source `counters_t` to the corresponding key in the 
 * destination `counters_t` (`result`). This is useful for performing an "OR" 
 * operation in query processing, where documents get the combined score from 
 * multiple matching words.
 * 
 * Parameters:
 * - arg: A pointer to the destination `counters_t` where counts should be updated.
 * - key: The document ID being processed.
 * - count: The count associated with this key in the source `counters_t`.
 * 
 * Returns:
 * - None (modifies `result` in place).
 */
void callbackAddCounts(void *arg, const int key, const int count)
{
  // Step 1: Cast the argument to a counters_t pointer (the destination counters)
  counters_t *result = arg;

  // Step 2: Retrieve the existing count for this key in result
  int existingCount = counters_get(result, key);

  // Step 3: Update the count by adding the new count
  counters_set(result, key, existingCount + count);
}

/* 
 * callbackCountNonZero - Counts the number of non-zero entries in a counters_t
 * 
 * This function is used as a callback in `counters_iterate()`. It counts how 
 * many document IDs have a non-zero count, which helps determine how many 
 * documents match a query.
 * 
 * Parameters:
 * - arg: A pointer to an integer counter, which will be incremented.
 * - key: The document ID being checked.
 * - count: The count associated with this key in the counters_t.
 * 
 * Returns:
 * - None (modifies the counter in `arg`).
 */
void callbackCountNonZero(void *arg, const int key, const int count)
{
  // Step 1: Only count keys with a non-zero count
  if (count > 0) {
    // Step 2: Cast arg to an integer pointer and increment it
    int *counter = arg;
    (*counter)++;
  }
}

/* 
 * callbackStoreDocScores - Stores nonzero document scores in an array
 * 
 * This function is used as a callback in `counters_iterate()`. It copies 
 * document scores from a `counters_t` structure into a `docScore_t` array 
 * for sorting and ranking purposes.
 * 
 * Only documents with a nonzero score are stored.
 * 
 * Parameters:
 * - arg: A pointer to a struct containing:
 *    - `docScores`: An array of `docScore_t` structures where scores are stored.
 *    - `docCount`: An integer tracking the number of stored entries.
 * - key: The document ID being processed.
 * - count: The count (score) associated with this document.
 * 
 * Returns:
 * - None (modifies `docScores` and updates `docCount`).
 */
void callbackStoreDocScores(void *arg, const int key, const int count) {
  // Step 1: Only process documents with nonzero scores
  if (count > 0) {
    // Step 2: Cast arg to a pointer to the struct containing docScores and docCount
    struct {
      docScore_t *docScores; // Array of document scores
      int docCount;          // Number of stored scores
    } *scoreList = arg;

    // Step 3: Store document ID and its score in the array
    scoreList->docScores[scoreList->docCount].docID = key;
    scoreList->docScores[scoreList->docCount].score = count;

    // Step 4: Move to the next available position in the array
    scoreList->docCount++;
  }
}