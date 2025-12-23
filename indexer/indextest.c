/*
* indextest.c     Atziri Enriquez     February 19, 2025
*
* This program tests the correctness of the Tiny Search Engine (TSE) indexer.
* It reads an existing index file into an index data structure, then writes
* it back to a new index file. The output should be identical to the input
* if the indexer functions correctly.
*
* Usage:
*     ./indextest oldIndexFilename newIndexFilename
*
* Parameters:
*     oldIndexFilename - The path to the existing index file to read.
*     newIndexFilename - The path to the new index file where data is written.
*
* Functions:
*     parseArgs(argc, argv, oldIndexFilename, newIndexFilename)
*         Parses and validates command-line arguments, ensuring correct usage.
*
* Exit Codes:
*     0 - Success
*     1 - Invalid arguments, file I/O errors, or index loading failure.
*
*/

 #include <stdio.h>
 #include <stdlib.h>
 #include "../common/index.h"
 #include "../libcs50/mem.h"
 
 /**************** function prototypes ****************/
 static void parseArgs(int argc, char* argv[], char** oldIndexFilename, char** newIndexFilename);
 
 /**************** main ****************/
 /**
 * Main function: Loads an index from an old index file and writes it to a new index file.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * 
 * Returns: 0 on success, exits with an error message on failure.
 */
 int main(int argc, char* argv[])
{
  char* oldIndexFilename;
  char* newIndexFilename;

  // Parse and validate command-line arguments
  parseArgs(argc, argv, &oldIndexFilename, &newIndexFilename);

  // Open the old index file for reading
  FILE* oldIndexFile = fopen(oldIndexFilename, "r");
  if (oldIndexFile == NULL) {
    fprintf(stderr, "Error: Unable to open index file '%s' for reading.\n", oldIndexFilename);
    exit(1);
  }

  // Load the index from the old index file
  index_t* index = index_load(oldIndexFile);
  fclose(oldIndexFile);  // Close after loading

  if (index == NULL) {
    fprintf(stderr, "Error: Unable to read index file '%s'.\n", oldIndexFilename);
    exit(1);
  }

  // Open the new index file for writing
  FILE* newIndexFile = fopen(newIndexFilename, "w");
  if (newIndexFile == NULL) {
    fprintf(stderr, "Error: Unable to write to index file '%s'.\n", newIndexFilename);
    index_delete(index);
    exit(1);
  }

  // Save the index to the new index file
  index_save(index, newIndexFile);
  fclose(newIndexFile);

  // Free the index memory
  index_delete(index);

  // Exit successfully
  exit(0);
}
 
  /**************** parseArgs ****************/
  /**
 * Parses and validates command-line arguments.
 * 
 * Ensures exactly two arguments are provided: an old index filename and a new index filename.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param oldIndexFilename Pointer to store the validated old index filename.
 * @param newIndexFilename Pointer to store the validated new index filename.
 * 
 * Assumptions: The caller provides `argc` and `argv` from `main()`.
 * Exits if arguments are invalid.
 */
 static void parseArgs(int argc, char* argv[], char** oldIndexFilename, char** newIndexFilename)
 {
  if (argc != 3) {
    fprintf(stderr, "Usage: ./indextest oldIndexFilename newIndexFilename\n");
    exit(1);
  }

  *oldIndexFilename = argv[1];
  *newIndexFilename = argv[2];
 }
 