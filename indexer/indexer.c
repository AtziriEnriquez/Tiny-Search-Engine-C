/*
* indexer.c     Atziri Enriquez     February 18, 2025
*
* This file implements the Indexer module for the Tiny Search Engine (TSE).
* The indexer reads webpage files from a crawler-produced directory, processes 
* each document to extract words, and builds an inverted index. The index is 
* then saved to an output file in a format suitable for later use in a query engine.
*
* Functions in this file:
*
*     parseArgs(argc, argv, pageDirectory, indexFilename)
*         Parses and validates command-line arguments. Ensures the page directory 
*         exists and the index file can be written to.
*
*     indexBuild(pageDirectory, index)
*         Reads webpages from the page directory, extracts words, and inserts 
*         them into an index.
*
*     indexPage(page, docID, index)
*         Extracts words from a single webpage, normalizes them, and inserts 
*         them into the index.
*
* The indexer assumes that the input directory was created by the TSE Crawler 
* and contains valid webpage data. It also assumes the index file location is 
* writable before execution.
*
* Exit Codes:
*     0 - Success
*     1 - Invalid arguments, memory allocation failure, file I/O failure
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../common/index.h"
#include "../common/pagedir.h"
#include "../common/word.h"
#include "../libcs50/webpage.h"
#include "../libcs50/mem.h"  // Include CS50's memory functions

/**************** function prototypes ****************/
static void parseArgs(int argc, char* argv[], char** pageDirectory, char** indexFilename);
static void indexBuild(const char *pageDirectory, index_t *index);
static void indexPage(webpage_t *page, const int docID, index_t *index);

/**************** main ****************/
/**
 * The main function parses command-line arguments, builds an index, and saves it to a file.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * 
 * Returns: 0 on success, exits with an error message on failure.
 */
int main(int argc, char* argv[])
{
  char* pageDirectory;
  char* indexFilename;

  // Parse and validate command-line arguments
  parseArgs(argc, argv, &pageDirectory, &indexFilename);

  // Create an index with a reasonable size
  index_t* index = index_new(500);
  if (index == NULL) {
    fprintf(stderr, "Error: Could not allocate memory for index.\n");
    exit(1);
  }

  // Build the index from the page directory
  indexBuild(pageDirectory, index);

  // Open the index file for writing
  FILE* indexFile = fopen(indexFilename, "w");
  // Do not need to check again, already did in parseArgs

  // Save the index to the file
  index_save(index, indexFile);
  fclose(indexFile);

  // Free memory before exiting
  index_delete(index);

  // Exit successfully
  exit(0);
}

/**************** parseArgs ****************/
/**
 * Parses and validates command-line arguments.
 * Ensures the page directory exists and the index file is writable.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param pageDirectory Pointer to store the validated page directory.
 * @param indexFilename Pointer to store the validated index filename.
 * 
 * Assumptions: The caller provides `argc` and `argv` from `main()`.
 * Exits if arguments are invalid or if the index file cannot be written.
 */
static void parseArgs(int argc, char* argv[], char** pageDirectory, char** indexFilename)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: ./indexer pageDirectory indexFilename\n");
    exit(1);
  }

  *pageDirectory = argv[1];
  *indexFilename = argv[2];

  // Validate page directory
  if (!pagedir_validate(*pageDirectory)) {
    fprintf(stderr, "Error: Invalid page directory '%s'.\n", *pageDirectory);
    exit(1);
  }

  // Try opening the index file to ensure it's writable
  FILE* file = fopen(*indexFilename, "w");
  if (file == NULL) {
    fprintf(stderr, "Error: Index file '%s' could not be written to. Check if it exists, is readable, or if the directory is writable.\n", *indexFilename);
    exit(1);
  }
  fclose(file);  // Close file after checking
}

/**************** indexBuild ****************/
/**
 * Reads pages from the page directory, processes each document, and builds the index.
 *
 * @param pageDirectory The path to the directory containing crawler-produced pages.
 * @param index The index structure to store word-document frequency mappings.
 * 
 * Assumptions: `index` is allocated before calling this function.
 * The function will read all pages sequentially until an invalid document ID is encountered.
 */
void indexBuild(const char* pageDirectory, index_t* index)
{
  int docID = 1;
  char* filepath = mem_malloc(strlen(pageDirectory) + 12); // Allocate space for directory + docID
  if (filepath == NULL) {
    fprintf(stderr, "Error: Could not allocate memory for filepath.\n");
    exit(1);
  }

  // Loop through document files in pageDirectory
  while (1) {
    snprintf(filepath, strlen(pageDirectory) + 12, "%s/%d", pageDirectory, docID);
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
      break;  // Stop when we reach a missing document ID
    }

    // Load the webpage from the open file
    webpage_t* page = pagedir_load(fp);
    fclose(fp);  // Close the file immediately after reading

    if (page != NULL) {
      indexPage(page, docID, index);
      webpage_delete(page); // Free memory after use
    }

    docID++;
  }

  free(filepath);  // Free allocated memory
}

/**************** indexPage ****************/
/**
 * Processes a webpage by extracting words, normalizing them, and adding them to the index.
 *
 * @param page A pointer to the webpage structure.
 * @param docID The document ID associated with the webpage.
 * @param index The index structure where words will be stored.
 * 
 * Assumptions: The `index` structure is initialized and allocated.
 * The function ignores words shorter than three characters.
 */
static void indexPage(webpage_t *page, const int docID, index_t *index)
{
  int pos = 0;
  char *word;

  // Extract each word from the webpage content
  while ((word = webpage_getNextWord(page, &pos)) != NULL) {
    if (strlen(word) >= 3) {  // Ignore short words (less than 3 characters)
      char* normalizedWord = normalizeWord(word);
      if (normalizedWord != NULL) {  // Ensure memory allocation was successful
        index_insert(index, normalizedWord, docID);
        free(normalizedWord);  // Free after inserting into the index
      }
    }
    free(word);
  }
}
