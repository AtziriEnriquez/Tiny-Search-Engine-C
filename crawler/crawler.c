/*
 * crawler.c - CS50 TSE Crawler
 * 
 * A simple web crawler that starts at a given seed URL, follows links to a specified depth, 
 * and saves fetched pages into a directory. It maintains a bag of URLs to be processed 
 * and a hashtable of seen URLs to avoid duplicates.
 *
 * Author: Atziri Enriquez
 * Date: 2/7/25
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 #include "../common/pagedir.h" // for pagedir_init and pagedir_save
 #include "../libcs50/bag.h"
 #include "../libcs50/hashtable.h"
 #include "../libcs50/webpage.h"
 #include "../libcs50/mem.h" // Defensive programming helpers
 
 /**************** function prototypes ****************/
 static void parseArgs(const int argc, char* argv[], char** seedURL, char** pageDirectory, int* maxDepth);
 static void crawl(char* seedURL, char* pageDirectory, const int maxDepth);
 static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen);
 
 /**************** main() ****************/
 /*
  * main - Entry point for the crawler program.
  * 
  * Parses command-line arguments, initializes data structures, and starts crawling.
  *
  * Parameters:
  *   argc - the number of command-line arguments
  *   argv - array of command-line arguments
  *
  * Returns:
  *   0 if execution is successful.
  *   Exits with error codes if command-line arguments are invalid.
  */
int main(const int argc, char* argv[]) {
  // Variables to hold user inputs
  char* seedURL;
  char* pageDirectory;
  int maxDepth;

  // Parse and validate arguments
  parseArgs(argc, argv, &seedURL, &pageDirectory, &maxDepth);

  // Start crawling
  crawl(seedURL, pageDirectory, maxDepth);

  return 0;
}

/**************** parseArgs() ****************/
/*
  * parseArgs - Parses command-line arguments and validates them.
  * 
  * Parameters:
  *   argc - the number of command-line arguments
  *   argv - array of command-line arguments
  *   seedURL - pointer to store the validated seed URL
  *   pageDirectory - pointer to store the validated directory
  *   maxDepth - pointer to store the parsed max depth
  *
  * Returns:
  *   None. Exits with an error message if arguments are invalid.
  *
  * Assumptions:
  *   - The user provides exactly three arguments: seed URL, directory, and max depth.
  *   - The seed URL is normalized and must be an internal URL.
  *   - The directory is writable and prepared for storing crawled pages.
  *   - The depth must be between 0 and 10.
  */
static void parseArgs(const int argc, char* argv[], char** seedURL, char** pageDirectory, int* maxDepth) {
    // Check argument count
    if (argc != 4) {
        fprintf(stderr, "Usage: ./crawler seedURL pageDirectory maxDepth\n");
        exit(1);
    }

    // Assign arguments
    *seedURL = argv[1];
    *pageDirectory = argv[2];
    *maxDepth = atoi(argv[3]); // Convert depth to integer

    // Validate seedURL
    *seedURL = normalizeURL(*seedURL);
    mem_assert(*seedURL, "Error: Invalid seedURL.");

    if (!isInternalURL(*seedURL)) {
        fprintf(stderr, "Error: seedURL must be an internal URL.\n");
        exit(1);
    }

    // Make sure you can initialize pageDirectory
    if (!pagedir_init(*pageDirectory)) {
        fprintf(stderr, "Error: Cannot initialize pageDirectory.\n");
        exit(1);
    }

    // Validate maxDepth
    if (*maxDepth < 0 || *maxDepth > 10) {
        fprintf(stderr, "Error: maxDepth must be between 0 and 10.\n");
        exit(1);
    }
}

/**************** crawl() ****************/
/*
  * crawl - Main crawling function that follows links up to a given depth.
  *
  * Parameters:
  *   seedURL - the starting webpage URL
  *   pageDirectory - directory to store fetched pages
  *   maxDepth - the maximum depth to crawl
  *
  * Returns:
  *   None.
  *
  * Assumptions:
  *   - The function starts with the seed URL and processes each discovered URL.
  *   - Pages are saved sequentially in the directory.
  *   - The crawler stops when the max depth is reached.
  *   - Memory is properly allocated and freed.
  */
static void crawl(char* seedURL, char* pageDirectory, const int maxDepth) {
    hashtable_t* pagesSeen = hashtable_new(200);
    mem_assert(pagesSeen, "Out of memory: Failed to create hashtable.");
    hashtable_insert(pagesSeen, seedURL, ""); // Add seed URL to hashtable

    bag_t* pagesToCrawl = bag_new();
    mem_assert(pagesToCrawl, "Out of memory: Failed to create bag.");

    webpage_t* seedPage = webpage_new(seedURL, 0, NULL);
    mem_assert(seedPage, "Out of memory: Failed to allocate webpage.");
    bag_insert(pagesToCrawl, seedPage); // initialize the bag and add a webpage representing the seedURL at depth 0

    int docID = 1; //docID starts at one
    webpage_t* page;
    // Process webpages until the bag is empty
    while ((page = bag_extract(pagesToCrawl)) != NULL) {
        int depth = webpage_getDepth(page);
        // Fetch the webpage content
        if (webpage_fetch(page)) {
            printf("%d   Fetched: %s\n", depth, webpage_getURL(page));
            // Save the fetched webpage to the directory
            pagedir_save(page, pageDirectory, docID++);
            // If not at max depth, scan the page for more links
            if (depth < maxDepth) {
                printf("%d  Scanning: %s\n", depth, webpage_getURL(page));
                pageScan(page, pagesToCrawl, pagesSeen);
            }
        }
        // Free the webpage memory
        webpage_delete(page);
    }
    // Clean up data structures
    hashtable_delete(pagesSeen, NULL);
    bag_delete(pagesToCrawl, webpage_delete);
}
/**************** pageScan() ****************/
/*
  * pageScan - Extracts links from a given webpage and adds them to the crawl list.
  *
  * Parameters:
  *   page - the current webpage being processed
  *   pagesToCrawl - bag to store new URLs to be visited
  *   pagesSeen - hashtable to track already visited URLs
  *
  * Returns:
  *   None.
  *
  * Assumptions:
  *   - The given webpage has already been fetched successfully.
  *   - Only internal URLs are considered.
  *   - URLs are normalized before being stored.
  *   - URLs are added to the bag only if they haven't been seen before.
  */
static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen) {
    int pos = 0;
    char* nURL;
    char* nextURL;
    int depth = webpage_getDepth(page);
    // Extract and process each URL in the page
    while ((nURL = webpage_getNextURL(page, &pos)) != NULL) {
        printf("%d     Found: %s\n", depth, nURL);
        // Normalize the URL
        nextURL = normalizeURL(nURL);
        mem_assert(nextURL, "Error: Invalid URL.");
        mem_free(nURL);
        
        // Only proceed if the URL is internal
        if (isInternalURL(nextURL)) {
            // If URL not seen before, add it
            if (hashtable_insert(pagesSeen, nextURL, "")) {
                webpage_t* newPage = webpage_new(nextURL, depth + 1, NULL);
                mem_assert(newPage, "Out of memory: Failed to allocate webpage.");
                bag_insert(pagesToCrawl, newPage);
                printf("%d     Added: %s\n", depth, nextURL);
            } else {
                printf("%d    IgnDupl: %s\n", depth, nextURL);
                mem_free(nextURL); // Free memory for ignored duplicate
            }
        } else {
            printf("%d   IgnExtrn: %s\n", depth, nextURL);
            mem_free(nextURL); // Free memory for ignored external URL
        }
    }
}