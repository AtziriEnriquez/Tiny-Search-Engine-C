/*
 * pagedir.c - CS50 Tiny Search Engine (TSE) Page Directory Module
 *
 * This module provides functions for managing the page directory used in the Tiny Search Engine.
 * It ensures that crawled webpages are properly stored, validated, and retrieved.
 *
 * Functions included:
 *  - `pagedir_init`: Initializes a directory for storing crawler output by creating a `.crawler` marker file.
 *  - `pagedir_save`: Saves a webpage to a uniquely named file in the page directory.
 *  - `pagedir_validate`: Checks whether a given directory is a valid crawler-produced directory.
 *  - `pagedir_load`: Loads a webpage from a saved file in the page directory.
 *
 * Assumptions:
 *  - The provided directory exists before calling `pagedir_init`.
 *  - The webpage files are sequentially named as `1, 2, 3, ...` without gaps.
 *  - The content of stored webpage files follows the specified format.
 *
 * Error Handling:
 *  - Functions return `false` or `NULL` if an unrecoverable error occurs (e.g., memory allocation failure, file access issues).
 *  - Invalid arguments result in an error message to `stderr`.
 *  - Defensive programming techniques (e.g., `mem_assert()`) are used to ensure memory safety.
 *
 * CS50 Tiny Search Engine - Lab 4
 * Author: Atziri Enriquez
 * Date: 2/19/25
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 #include "../libcs50/webpage.h"
 #include "pagedir.h"
 #include "../libcs50/file.h"
 #include "../libcs50/mem.h" // Defensive programming helpers

  /**************** function prototypes ****************/
  bool pagedir_init(const char* pageDirectory);
  void pagedir_save(const webpage_t* page, const char* pageDirectory, int docID);
  bool pagedir_validate(const char* pageDirectory);
  webpage_t* pagedir_load(FILE* fp);

 /* ************** pagedir_init ************** */
/*
 * pagedir_init - Initializes the given directory for storing crawler output.
 *
 * This function verifies that the provided directory exists and is accessible.
 * If valid, it creates a ".crawler" file inside the directory as a marker
 * to indicate that the directory is being used for storing crawled pages.
 *
 * Parameters:
 * - pageDirectory: The directory where crawler output will be stored.
 *
 * Returns:
 * - true if the directory is valid and initialized successfully.
 * - false if the directory is invalid, inaccessible, or if the marker file cannot be created.
 */
bool pagedir_init(const char* pageDirectory) {
    if (pageDirectory == NULL) {
        fprintf(stderr, "Error: pageDirectory argument is NULL\n");
        return false;
    }

    // Calculate the length of the full path needed for the .crawler file, include the null terminator
    size_t pathLen = strlen(pageDirectory) + strlen("/.crawler") + 1; 

    // Allocate memory for the file path
    char* crawlerFilePath = mem_malloc(pathLen * sizeof(char));
    mem_assert(crawlerFilePath, "Out of memory: failed to allocate memory for filepath.");

    // Construct the full file path safely
    snprintf(crawlerFilePath, pathLen, "%s/.crawler", pageDirectory);

    // Attempt to open (create) the .crawler file in write mode ("w").
    FILE* crawlerFile = fopen(crawlerFilePath, "w");

    // Free memory after using the string
    mem_free(crawlerFilePath);

    // Check if the file was successfully created
    if (crawlerFile == NULL) {
        fprintf(stderr, "Error: cannot create .crawler file in directory %s\n", pageDirectory);
        return false;
    }

    fclose(crawlerFile);
    return true; 
}

/* ************** pagedir_save ************** */
/*
* pagedir_save - Saves a webpage's data to a file within the specified directory.
*
* The file is named based on a unique document ID, ensuring that multiple pages
* are stored uniquely in the directory. The file will contain:
* - The URL of the webpage on the first line.
* - The depth of the webpage on the second line.
* - The raw HTML content of the webpage following the depth.
*
* Parameters:
* - page: The webpage structure containing the data to be saved.
* - pageDirectory: The directory where the webpage should be stored.
* - docID: A unique document ID assigned to the webpage.
*/
void pagedir_save(const webpage_t* page, const char* pageDirectory, int docID) {
    if (page == NULL || pageDirectory == NULL || docID < 0) {
        fprintf(stderr, "Error: invalid arguments to pagedir_save\n");
        return;
    }

    // Calculate the necessary length for the file path ("/1234567890" fits any 32-bit integer docID)
    size_t pathLen = strlen(pageDirectory) + 12;

    // Allocate memory for the file path
    char* saveFilePath = mem_malloc(pathLen * sizeof(char));
    mem_assert(saveFilePath, "Out of memory: failed to allocate memory for filepath.");

    // Construct the full file path safely
    snprintf(saveFilePath, pathLen, "%s/%d", pageDirectory, docID);

    // Open the file for writing
    FILE* savedFile = fopen(saveFilePath, "w");

    // Free memory for file path immediately after using it
    mem_free(saveFilePath);

    // Check if file creation was successful
    if (savedFile == NULL) {
        fprintf(stderr, "Error: cannot create document file for ID %d\n", docID);
        return;
    }

    // Retrieve webpage components
    const char* url = webpage_getURL(page);
    const char* html = webpage_getHTML(page);
    int depth = webpage_getDepth(page);

    // Validate retrieved webpage data
    if (url == NULL || html == NULL) {
        fprintf(stderr, "Error: invalid webpage contents\n");
        fclose(savedFile);
        return;
    }

    // Write webpage data to file
    fprintf(savedFile, "%s\n%d\n%s\n", url, depth, html);

    // Close the file
    fclose(savedFile);
}

/**************** pagedir_validate ****************/
/*
 * Validates whether a directory is a crawler-produced directory.
 *
 * The function checks for the existence of a ".crawler" file in the directory.
 *
 * Parameters:
 *   - pageDirectory: The directory to validate.
 *
 * Returns:
 *   - true if the directory contains a ".crawler" file (valid crawler directory).
 *   - false if the file is missing or directory is invalid.
 */
bool pagedir_validate(const char* pageDirectory) {
    if (pageDirectory == NULL) {
        fprintf(stderr, "Error: pageDirectory argument is NULL\n");
        return false;
    }

    // Allocate memory for full path ("/.crawler")
    size_t pathLen = strlen(pageDirectory) + strlen("/.crawler") + 1;
    char* crawlerFilePath = mem_malloc(pathLen);
    mem_assert(crawlerFilePath, "Out of memory: failed to allocate validation filepath");

    // Construct the full path
    snprintf(crawlerFilePath, pathLen, "%s/.crawler", pageDirectory);

    // Try to open the ".crawler" file
    FILE* crawlerFile = fopen(crawlerFilePath, "r");
    mem_free(crawlerFilePath); // Free memory now that it's used

    if (crawlerFile == NULL) {
        return false; // Not a valid crawler directory
    }

    fclose(crawlerFile);
    return true;
}

/**************** pagedir_load ****************/
/*
 * Loads a webpage from a file in the pageDirectory.
 *
 * The function reads:
 *   - The URL from the first line.
 *   - The depth from the second line.
 *   - The HTML content from the remaining lines.
 *
 * Parameters:
 *   - fp: The file pointer to an open webpage file.
 *
 * Returns:
 *   - A `webpage_t*` structure with the loaded webpage data.
 *   - NULL if the file is invalid or an error occurs.
 */
webpage_t* pagedir_load(FILE* fp) {
    if (fp == NULL) {
        return NULL;
    }

    // Read the URL
    char* url = file_readLine(fp);
    if (url == NULL) {
        return NULL; // Unable to read the URL
    }

    // Read the depth
    char* depthStr = file_readLine(fp);
    if (depthStr == NULL) {
        free(url);
        return NULL; // Unable to read the depth
    }

    int depth = atoi(depthStr);
    free(depthStr);

    // Read the HTML content
    char* html = file_readFile(fp);
    if (html == NULL) {
        free(url);
        return NULL; // Unable to read the HTML
    }

    // Create a new webpage structure
    webpage_t* page = webpage_new(url, depth, html);
    return page;
}