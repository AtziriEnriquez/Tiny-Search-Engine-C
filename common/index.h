/*
 * index.h - CS50 Tiny Search Engine (TSE) Index Module
 *
 * This module provides an interface for managing an index structure that maps words
 * to document ID counts. The index is implemented as a wrapper around a `hashtable_t`,
 * where each word is mapped to a `counters_t` object storing document frequency counts.
 *
 * Functions:
 *  - `index_new`: Creates a new index structure.
 *  - `index_insert`: Inserts or updates a word-document count mapping.
 *  - `index_get`: Retrieves the counters associated with a word.
 *  - `index_save`: Writes the index structure to a file.
 *  - `index_load`: Loads an index structure from a file.
 *  - `index_delete`: Frees all allocated memory for the index.
 *
 * Assumptions:
 *  - The index file follows the specified format, with words mapped to document counts.
 *  - The index is intended for use by the TSE Indexer and Querier.
 *
 * Error Handling:
 *  - Functions return `NULL` or `false` on failure (e.g., memory allocation errors, invalid input).
 *  - Invalid arguments result in an error message to `stderr` where applicable.
 *
 * CS50 Tiny Search Engine - Lab 5
 * Author: Atziri Enriquez
 * Date: 2/18/25
 */

#ifndef __INDEX_H
#define __INDEX_H

#include <stdio.h>
#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"

/******************** STRUCTURES ********************/

/* The index data structure, which maps words to document ID counts */
typedef struct index index_t;

/******************** FUNCTION PROTOTYPES ********************/

/**
 * Creates a new index with a specified number of slots.
 * 
 * @param num_slots Number of slots in the hashtable.
 * @return Pointer to a new `index_t` or NULL on failure.
 */
index_t* index_new(const int num_slots);

/**
 * Inserts a word into the index or updates its document count.
 * If the word is not already in the index, it will be added.
 *
 * @param index Pointer to the index.
 * @param word The word to insert/update.
 * @param docID The document ID where the word appears.
 */
void index_insert(index_t* index, const char* word, const int docID);

/**
 * Sets the count for a specific word in a document.
 * If the word is not in the index, it will be added.
 *
 * @param index Pointer to the index.
 * @param word The word whose count should be set.
 * @param docID The document ID.
 * @param count The count to set.
 */
void index_set(index_t* index, const char* word, const int docID, const int count);

/**
 * Saves the index to a file.
 * The format follows the TSE specification:
 * Each line contains a word followed by (docID, count) pairs.
 *
 * @param index Pointer to the index.
 * @param fp File pointer to write to.
 */
void index_save(index_t* index, FILE* fp);

/**
 * Loads an index from a file.
 * The file should be formatted as described in `index_save()`.
 *
 * @param fp File pointer to read from.
 * @return Pointer to a newly allocated index, or NULL on failure.
 */
index_t* index_load(FILE* fp);

/**
 * Deletes an index and frees all associated memory.
 *
 * @param index Pointer to the index to delete.
 */
void index_delete(index_t* index);

/**
 * Finds the counters object for a given word in the index.
 *
 * @param index Pointer to the index.
 * @param word The word to search for.
 * @return Pointer to the `counters_t` structure containing document IDs and counts,
 *         or NULL if the word is not found.
 */
counters_t* index_find(index_t* index, const char* word);

#endif // __INDEX_H