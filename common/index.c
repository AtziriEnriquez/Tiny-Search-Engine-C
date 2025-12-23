/*
* index.c     Atziri Enriquez     February 18, 2025
*
* This file implements the Index module for the Tiny Search Engine (TSE).
* The index maps words to document frequency counts, allowing efficient lookups
* for querying. It is implemented as a wrapper around a `hashtable_t`, where
* each word maps to a `counters_t` object that tracks occurrences of the word
* in different documents.
*
* Functions:
*     index_new(num_slots)
*         Creates and initializes a new index.
*
*     index_insert(index, word, docID)
*         Adds a word to the index, incrementing its count for the given docID.
*
*     index_set(index, word, docID, count)
*         Explicitly sets the count of a word for a given docID.
*
*     index_save(index, fp)
*         Saves the index structure to a file.
*
*     index_load(fp)
*         Loads an index structure from a file.
*
*     index_find(index, word)
*         Retrieves the counters object associated with a word.
*
*     index_delete(index)
*         Frees all memory allocated for the index.
*
* Assumptions:
*     - Words are stored in lowercase and are dynamically allocated.
*     - The index file follows the required format for the Indexer and Querier.
*     - Memory allocation failures are handled with program termination.
*
* Exit Codes:
*     - The program will exit with an error if memory allocation fails.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"
#include "../libcs50/mem.h"
#include "../libcs50/file.h" 

typedef struct index {
  hashtable_t* ht;  // Wrapper around hashtable, mapping words → (docID, count)
} index_t;

/******************** FUNCTION PROTOTYPES ********************/
/******************** INDEX FUNCTIONS ********************/
index_t* index_new(const int num_slots);
void index_insert(index_t* index, const char* word, const int docID);
void index_set(index_t* index, const char* word, const int docID, const int count);
void index_save(index_t* index, FILE* fp);
index_t* index_load(FILE* fp);
void index_delete(index_t* index);
counters_t* index_find(index_t* index, const char* word);
/******************** HELPER FUNCTIONS ********************/
static void save_index_word(void* arg, const char* key, void* item);
static void save_index_counts(void* arg, const int docID, const int count);
static void delete_counters(void* item);

/**
 * Creates a new index structure.
 * 
 * @param num_slots Number of slots in the hashtable.
 * @return Pointer to the new index structure, or NULL if allocation fails.
 */
index_t* index_new(const int num_slots)
{
  // Allocate memory for the index structure
  index_t* index = mem_malloc(sizeof(index_t));
  
  // Ensure memory was allocated successfully
  mem_assert(index, "Error: Could not allocate memory for index.\n");

  // Create a new hashtable with the specified number of slots
  index->ht = hashtable_new(num_slots);
  
  // If hashtable creation fails, clean up and return NULL
  if (index->ht == NULL) {
    mem_free(index);  // Free the allocated index structure
    return NULL;  // Indicate failure
  }

  // Return the newly created index structure
  return index;
}

/**
 * Inserts a word and document ID into the index, incrementing its count.
 * 
 * @param index Pointer to the index structure.
 * @param word Word to insert (must be a valid string).
 * @param docID Document ID associated with the word.
 */
void index_insert(index_t* index, const char* word, const int docID)
{
  // Defensive check: ensure valid input
  if (index == NULL || word == NULL || docID < 0) {
    return;  // Invalid input, do nothing
  }

  // Look up the word in the index's hashtable
  counters_t* ctrs = hashtable_find(index->ht, word);

  // If the word is not already in the index, create a new counters structure
  if (ctrs == NULL) {
    ctrs = counters_new();  // Allocate a new counter set
    mem_assert(ctrs, "Failed to allocate memory for counters in index_insert");  // Ensure allocation succeeded
    hashtable_insert(index->ht, word, ctrs);  // Insert the new counter set into the hashtable
  }

  // Increment the count for the given document ID
  counters_add(ctrs, docID);
}

/**
 * Sets the count of a word for a given document in the index.
 * 
 * @param index Pointer to the index structure.
 * @param word Word to update.
 * @param docID Document ID associated with the word.
 * @param count New count to set.
 */
void index_set(index_t* index, const char* word, const int docID, const int count)
{
  // Defensive check: ensure valid input
  if (index == NULL || word == NULL || docID < 0 || count < 0) {
    return;  // Invalid input, do nothing
  }

  // Look up the word in the index's hashtable
  counters_t* ctrs = hashtable_find(index->ht, word);

  // If the word is not already in the index, create a new counters structure
  if (ctrs == NULL) {
    ctrs = counters_new();  // Allocate a new counter set
    mem_assert(ctrs, "Failed to allocate memory for counters in index_set");  // Ensure allocation succeeded
    hashtable_insert(index->ht, word, ctrs);  // Insert the new counter set into the hashtable
  }

  // Set/update the count for the given document ID
  counters_set(ctrs, docID, count);
}

/**
 * Saves the index to a file.
 * 
 * @param index Pointer to the index structure.
 * @param fp File pointer to write to.
 */
void index_save(index_t* index, FILE* fp)
{
  if (index == NULL || fp == NULL) {
    return;
  }
  hashtable_iterate(index->ht, fp, save_index_word);
}

/* Writes a word and its associated document counts to the file */
static void save_index_word(void* arg, const char* key, void* item)
{
  FILE* fp = arg;
  counters_t* ctrs = item;
  fprintf(fp, "%s", key);
  counters_iterate(ctrs, fp, save_index_counts);  // Nested _iterate method
  fprintf(fp, "\n");
}

/* Writes each (docID, count) pair for a given word */
static void save_index_counts(void* arg, const int docID, const int count)
{
  FILE* fp = arg;
  fprintf(fp, " %d %d", docID, count);
}

/**
 * Loads an index from a file.
 * 
 * @param fp File pointer to read from.
 * @return Pointer to the loaded index structure, or NULL on failure.
 */
index_t* index_load(FILE* fp)
{
  // Defensive check: ensure the file pointer is valid
  if (fp == NULL) {
    return NULL;  // Return NULL if file is not open or invalid
  }

  // Create a new index structure with allocated slots in hashtable
  index_t* index = index_new(file_numLines(fp));
  
  // Ensure index was successfully created
  mem_assert(index, "Error: Failed to create index.\n");

  char word[256];  // Buffer to store the word read from file
  int docID, count;  // Variables to store document ID and count

  // Read words from the file one at a time
  while (fscanf(fp, "%s", word) == 1) {  
    // For each word, read its associated docID and count pairs
    while (fscanf(fp, "%d %d", &docID, &count) == 2) {  
      // Insert the word and its corresponding docID-count pair into the index
      index_set(index, word, docID, count);
    }
  }

  // Return the populated index structure
  return index;
}

/**
 * Deletes the index and frees all allocated memory.
 * 
 * @param index Pointer to the index structure.
 */
void index_delete(index_t* index)
{
  if (index != NULL) {
    hashtable_delete(index->ht, delete_counters);
    mem_free(index);
  }
}

/* Frees the memory allocated for a counters object */
static void delete_counters(void* item)
{
  counters_t* ctrs = item;
  counters_delete(ctrs);
}

/**
 * Finds the counters object associated with a word.
 * 
 * @param index Pointer to the index structure.
 * @param word Word to find.
 * @return Pointer to the counters object, or NULL if the word is not found.
 */
counters_t* index_find(index_t* index, const char* word) 
{
  if (index == NULL || word == NULL) {
    return NULL;
  }
  return hashtable_find(index->ht, word);
}