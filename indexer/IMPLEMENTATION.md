# CS50 TSE Indexer

## Implementation Spec

This document outlines the implementation-specific details for the Indexer module in the Tiny Search Engine (TSE). It references the Requirements Specification and Design Specification while focusing on the following aspects:

   - Data structures

   - Control flow: pseudo code for overall flow, and for each of the functions

   - Detailed function prototypes and their parameters

   - Error handling and recovery

   - Testing plan

## Data structures

The Indexer uses an **index_t structure** to store an inverted index mapping normalized words to document IDs and their respective counts. The index is implemented using a **hashtable**, where each word is a key, and the associated value is a **counters_t structure** that stores document IDs and their word counts.

## Control flow

The Indexer is implemented in one file, indexer.c, with four primary functions.

### main

The main function is responsible for:

   1. Parsing command-line arguments using parseArgs.

   2. Creating a new index_t structure.

   3. Building the index from the page directory using indexBuild.

   4. Saving the index to the specified index file.

   5. Freeing allocated memory before exiting.

### parseArgs

Parses command-line arguments and ensures correctness.

   - Requires exactly two arguments: pageDirectory and indexFilename.

   - Calls pagedir_validate() to ensure the page directory is valid.

   - Attempts to open the index file for writing to ensure it is accessible.

   - Exits with an error message if any checks fail.

### indexBuild

Reads page files from the provided pageDirectory and processes each webpage to build the index.

Pseudocode:
```
initialize docID to 1
while (file for docID exists in pageDirectory)
    load webpage from file
    call indexPage() with the webpage and docID
    delete the webpage object
    increment docID
```
### indexPage

Processes a webpage, extracts words, normalizes them, and adds them to the index.

Pseudocode:
```
initialize position to 0
while (next word is found in webpage content)
    if word length >= 3
        normalize the word
        insert the word into the index with the docID
free allocated words
```
## Other modules

### pagedir

The Indexer interacts with pagedir.c to:

   - Validate the pageDirectory before processing.

   - Load webpage content from page files.

### word

Handles word normalization, converting words to lowercase and ensuring they contain only alphabetic characters.

### index

Manages the index data structure and provides functions to:

   - Create a new index (index_new).

   - Insert words into the index (index_insert).

   - Save and load the index from files (index_save, index_load).

   - Free index memory (index_delete).

## Function prototypes

Detailed function descriptions are provided in indexer.c before each function definition.

```c
int main(int argc, char* argv[]);
static void parseArgs(int argc, char* argv[], char** pageDirectory, char** indexFilename);
static void indexBuild(const char* pageDirectory, index_t* index);
static void indexPage(webpage_t* page, const int docID, index_t* index);
```
## Error handling and recovery

   - If incorrect command-line arguments are provided, the program prints an error message and exits.

   - If memory allocation fails, mem_assert() is used to handle the error and exit gracefully.

   - If the pageDirectory is invalid or unreadable, the program prints an error and exits.

   - If the indexFilename is unwritable, the program prints an error and exits.

## Testing plan

### Unit testing

   - The index module is tested using a separate driver program to ensure proper insertion, retrieval, and deletion of words and document counts.

   - The word module is tested to verify correct word normalization.

   - The pagedir module is tested to confirm proper reading and validation of page directories.

### Regression testing

   - Index files generated from previous successful runs are stored for comparison using indxcmp.

   - Running indextest on an index file ensures the saved index correctly reloads into memory.

   - Output is compared with expected results to ensure correctness.

### Integration/system testing

A testing.sh script automates testing by:

1. Running indexer with various test cases:

   - Valid page directories and index filenames

   - Edge cases such as empty directories

2. Running indextest to verify index correctness:

   - Loads an index file and saves it to a new file

   - Compares the original and new index files using indexcmp

3. Running valgrind to check for memory leaks:

   - Ensures indexer does not leak memory

   - Logs memory errors for debugging

### Expected results

   - indexer should correctly parse and index words from valid page directories.

   - indextest should generate an identical index file.

   - valgrind should report no memory leaks.

   - Index files should match across multiple runs for the same input.