/*
 * pagedir.h - CS50 Tiny Search Engine (TSE) Page Directory Interface
 *
 * This module provides an interface for managing the page directory in the Tiny Search Engine.
 * It allows for initializing, validating, saving, and loading webpage files.
 *
 * Functions:
 *  - `pagedir_init`: Initializes a directory for storing crawler output by creating a `.crawler` marker file.
 *  - `pagedir_save`: Saves a webpage to a uniquely named file in the page directory.
 *  - `pagedir_validate`: Checks whether a given directory is a valid crawler-produced directory.
 *  - `pagedir_load`: Loads a webpage from a saved file in the page directory.
 *
 * Assumptions:
 *  - The provided directory exists before calling `pagedir_init`.
 *  - The page directory contains sequentially named files (`1, 2, 3, ...`).
 *  - Webpage files follow the specified format required by the Indexer and Querier.
 *
 * Error Handling:
 *  - Functions return `false` or `NULL` on failure (e.g., memory allocation issues, file access errors).
 *  - Invalid arguments result in an error message to `stderr`.
 *  - Defensive programming techniques (`mem_assert()`) are used to prevent memory leaks.
 *
 * CS50 Tiny Search Engine - Lab 4
 * Author: Atziri Enriquez
 * Date: 2/19/25
 */

 #ifndef __PAGEDIR_H
 #define __PAGEDIR_H
 
 #include <stdio.h>
 #include <stdbool.h>
 #include "../libcs50/webpage.h"  // Needed for webpage_t type
 
 /**
  * Initializes a page directory by creating a `.crawler` file.
  * 
  * @param pageDirectory The directory where pages will be stored.
  * @return true if successful, false otherwise.
  */
 bool pagedir_init(const char* pageDirectory);
 
 /**
  * Saves a webpage to the specified directory with a unique document ID.
  * 
  * @param page The webpage to save.
  * @param pageDirectory The directory to save the webpage in.
  * @param docID The document ID (unique number for the file).
  */
 void pagedir_save(const webpage_t* page, const char* pageDirectory, int docID);

/**
 * Validates whether a given directory is a crawler-produced directory.
 * 
 * This function checks for the existence of a `.crawler` file inside
 * the specified directory to determine if it was created by the crawler.
 * 
 * @param pageDirectory The directory to validate.
 * @return true if the directory contains a `.crawler` file, false otherwise.
 */
bool pagedir_validate(const char* pageDirectory);

/**
 * Loads a webpage from an open file in the page directory.
 * 
 * This function reads:
 *  - The URL from the first line.
 *  - The depth from the second line.
 *  - The HTML content from the remaining lines.
 * 
 * @param fp The file pointer to an open webpage file.
 * @return A `webpage_t*` structure containing the loaded webpage,
 *         or NULL if an error occurs.
 */
webpage_t* pagedir_load(FILE* fp);
 
 #endif // __PAGEDIR_H