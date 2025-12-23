# CS50 TSE Querier

## Implementation Specification

In this document, we reference the Requirements Specification and Design Specification and focus on the implementation-specific decisions for the Querier component.
The Querier reads an index file produced by the Indexer and page files produced by the Crawler, processes search queries, and ranks matching results based on relevance.

The core topics covered in this spec are:

  - Data structures
  - Control flow: Pseudocode for overall flow and key functions
  - Detailed function prototypes and parameters
  - Error handling and recovery
  - Testing plan

## **Data Structures**

The Querier relies on the following data structures:

### Index (index_t)

  - A hashtable-based inverted index mapping words to counters_t structures.
  - Provides word lookup functionality (index_find()).

### Counters (counters_t)

  - Maps document IDs to word frequencies.
  - Supports set operations (AND, OR) for query processing.

### Document Scores (docScore_t)

  - Stores document ID and score for sorting ranked results.
  ```c
  typedef struct {
      int docID;
      int score;
  } docScore_t;
  ```

## **Control Flow**

The Querier is implemented in one file (querier.c) and follows this flow:

### prompt

This function checks whether standard input is coming from a terminal and, if so, prints a query prompt for user input.
Pseudocode:

  Check if standard input (stdin) is from a terminal:
    Use isatty(fileno(stdin)) to determine if the input source is interactive.

  If input is from a terminal:
    Print "Query? " to prompt the user.
    Flush the output buffer to ensure immediate display.

### parseArgs

Parses and validates command-line arguments, ensuring the correct number of arguments, verifying the page directory, and checking the validity of the index file.
Pseudocode:

  Check if exactly two arguments are provided (excluding the program name):
  If not, print usage instructions and exit.

  Store the command-line arguments:
  Assign pageDirectory to the first argument.
  Assign indexFilename to the second argument.

  Validate the page directory:
  Use pagedir_validate to ensure it is a valid crawler output directory.
  If invalid, print an error message and exit.

  Validate the index file:
  Attempt to open indexFilename for reading.
  If the file cannot be opened, print an error message and exit.
  Close the file after validation.

### main

The main function initializes the query engine, loads the index, and continuously processes user queries.
Pseudocode:

  Parse and validate command-line arguments:
  Extract pageDirectory and indexFilename from argv using parseArgs.

  Open the index file and load it into memory:
  Open indexFilename for reading.
  Load the index structure from the file into memory using index_load.
  Close the file after loading.

  Read user queries in a loop:
  Initialize query buffer.
  Continuously prompt the user and read input using getline.
  Process each query with processQuery.
  Print a separator line after each query.

  Free allocated memory before exiting:
  Free the query buffer.
  Free the loaded index.

  Return 0 to indicate successful execution.

### processQuery

Processes a user query by validating input, tokenizing words, checking syntax, evaluating the query, and displaying ranked results.
Pseudocode:

  Validate query characters:
  Check if query contains only letters and spaces using isValidCharacters.
  If invalid, return immediately.

  Estimate the maximum number of words:
  Assume a worst-case scenario where every other character is a space.
  Allocate memory for words array.
  Ensure memory allocation was successful.

  Tokenize the query:
  Use queryTokenize to split the query into words.
  If no tokens are found, return.

  Validate query syntax:
  Check for proper use of "AND" and "OR" with isValidQuerySyntax.
  If invalid, return.

  Print the formatted query:
  Output the parsed query for logging.

  Evaluate the query:
  Call queryEvaluate to retrieve matching documents based on the query.

  Print ranked results:
  Display ranked documents using printRankedResults.

  Free allocated memory:
  Delete result counters to prevent memory leaks.

### queryTokenize

Splits the query string into individual words, converts them to lowercase, and stores their addresses in the words array.
Pseudocode:

  Initialize counters and pointers:
  t = 0 -> Tracks the number of words stored.
  start = query -> Pointer to traverse the query string.

  Iterate through the query string:
  Skip leading whitespace.
  If the end of the string is reached, exit.

  Process each word:
  Set end = start to find the end of the word.
  Convert characters to lowercase while traversing.

  Store the word:
  If the word count is within maxWords, store the starting address.
  If the limit is exceeded, print an error and stop processing.

  Null-terminate the word:
  Replace the space at end with '\0'.
  Move start to the next word.

  Return the number of words found (t).

### isValidCharacters

Checks if a query string contains only valid characters (letters and spaces). If an invalid character is found, an error message is printed.
Pseudocode:

Iterate through each character in the query:
Loop until the null terminator ('\0') is reached.

Check character validity:
If the character is not a letter and not a space:
Print an error message identifying the invalid character.
Return false.

Return true if no invalid characters were found.

### isValidQuerySyntax

Ensures that a tokenized query follows correct syntax rules, preventing invalid placements of "AND" and "OR".
Pseudocode:

  Check if the query is empty:
  If t == 0, return false.

  Ensure the query does not start with "AND" or "OR":
  If the first word is "AND" or "OR", print an error and return false.

  Ensure the query does not end with "AND" or "OR":
  If the last word is "AND" or "OR", print an error and return false.

  Loop through words to check for consecutive "AND" or "OR":
  If "AND" or "OR" appears right after another "AND" or "OR", print an error and return false.

  Return true if all checks pass.

### matchMerge

Merges the current AND sequence (andSequence) into the OR sequence (orSequence), ensuring all valid matches are combined.
Pseudocode:

Check if andSequence is not NULL (i.e., there is something to merge):
  If orSequence is NULL, create a new counters structure.
    Ensure memory allocation for orSequence is successful.

  Merge andSequence into orSequence using unionCounters.

  Free andSequence after merging to prevent memory leaks.

  Set andSequence to NULL to mark it as merged.

### unionCounters

Combines two counters structures using OR logic, adding counts from andResult into result.
Pseudocode:

  Check for NULL inputs:
  If either result or andResult is NULL, return immediately.

  Iterate over andResult:
  Use counters_iterate to add counts from andResult to result using callbackAddCounts.

### intersectCounters

Combines two counters structures using AND logic, keeping the minimum counts for each document.
Pseudocode:

  Ensure valid memory before proceeding:
  Assert that acc (accumulator) is not NULL.
  Assert that wordCounters (counters for the current word) is not NULL.

  Prepare structure to pass both counters to the callback:
  Create a struct containing acc and wordCounters.

  Iterate over acc:
  Use counters_iterate with callbackSetMinimum to update acc with the minimum count for each document.

### queryEvaluate

Evaluates the query using AND/OR logic, finding matching documents based on the given search terms.
Pseudocode:

initialize andSequence to store results of consecutive "AND" operations.
initialize orSequence to store results merged by "OR".
initialize andSequenceInvalid to track if the current "AND" sequence is invalid.

Iterate through each word in the query:
  If the word is "or":
    Merge the current andSequence into orSequence.
    Reset andSequenceInvalid since a new OR sequence is starting.

  If andSequenceInvalid is true, skip processing.

  If the word is "and", continue to the next word.

  Find the counters structure for the word in index.
    If the word is not found:
      Mark andSequenceInvalid as true.
      Free and reset andSequence.

    If the word is found:
      If andSequence is NULL, create a new counters structure and copy counts.
      Otherwise, intersect the existing andSequence with the new word’s matches.

After processing all words:
Merge any remaining andSequence into orSequence.
Return orSequence, which contains the final matched documents.

## compareScores

Compares two docScore_t structures for sorting document scores in descending order. This function is used with qsort() to rank documents based on their scores.
Pseudocode:

Cast the input pointers to docScore_t *:
Convert a and b to docScore_t * to access their score fields.

Compute the difference between scores:
Subtract the score of a from the score of b.

Return the result:
If b -> score is greater, return a positive value → b comes before a (higher-ranked first).
If scores are equal, return zero -> order remains unchanged.
If a -> score is greater, return a negative value → a comes before b.

### printRankedResults

Prints ranked search results by retrieving document scores from result, sorting them, and displaying the corresponding URLs.
Pseudocode:

  Initialize printedCount = 0 to track the number of documents with nonzero scores.

  Initialize Count matching documents:
  Use counters_iterate with callbackCountNonZero to count documents with a nonzero score.

  If no documents match, print "No documents match." and return.

  Allocate memory for storing document scores:
  Allocate printedCount elements of docScore_t for storing document scores.
  If memory allocation fails, print an error message and return.

  Store document scores:
  Use a struct scoreList to hold docScores and track the current count.
  Use counters_iterate with callbackStoreDocScores to populate the array.

  Sort the document scores in descending order:
  Use qsort with compareScores.

  Print the ranked results:
  Iterate through scoreList.docScores.
  Construct the filename from pageDirectory and document ID.
  Open the corresponding file and read the first line (the URL).
  Print the score, document ID, and URL.
  Free the allocated memory for the URL.
  Free allocated memory:

  Free scoreList.docScores to prevent memory leaks.

### callbackSetMinimum

This function is a callback used to update an accumulator (acc) with the minimum counts between two counters structures. It ensures that the final count for each document is the lower value between acc and wordCounters, implementing AND logic for query evaluation.
Pseudocode:

  Retrieve both counters:
  Cast arg to a struct containing acc (accumulator) and wordCounters (new set of counters to compare with).

  Get the count for the current key in wordCounters.

  Determine the minimum count:
  If the count in acc is smaller, keep it.
  Otherwise, update it to the count from wordCounters.

  Store the minimum value in acc.

### callbackAddCounts

This function is a callback used to merge two counters structures by summing their counts. It is used in OR logic for query evaluation, ensuring that document scores accumulate across multiple terms.
Pseudocode:

  Retrieve the destination counters structure:
  Cast arg to a counters_t* (the structure where counts will be accumulated).

  Get the current count for the given key in result.

  Update the count:
  Add the count from the current counters structure to the existing count in result.

  Store the updated total count in result.

### callbackCountNonZero

This function is a callback used to count the number of keys in a counters structure that have a non-zero count. It is useful for determining the number of documents that match a query.
Pseudocode:

  Check if the count is greater than zero:
  If count is zero, ignore this key.
  If count is positive, proceed to the next step.

  Increment the counter:
  Cast arg to an integer pointer.
  Increment the value it points to.

### callbackStoreDocScores

This function is a callback used to store document scores when iterating over a counters structure. It ensures that only documents with nonzero scores are recorded.
Pseudocode:

  Check if the document has a nonzero score:
  If count is zero, ignore this document.
  If count is positive, proceed to store the score.

  Retrieve the document score list:
  Cast arg to a struct containing:
    docScores (array of document scores).
    docCount (current position in the array).

  Store the document's ID and score:
    Assign docID to key.
    Assign score to count.

  Increment docCount:
    Move to the next available position in the docScores array.

## **Other Modules**

### index

The index module provides functions for storing and retrieving word-document mappings. It is essential for query evaluation, as it allows efficient lookup of documents containing specific words.

`index_load`: Reads an index file and constructs an in-memory representation.
`index_find`: Retrieves a counters_t structure containing document frequencies for a given word.
By encapsulating index operations within index.c, we maintain modularity and enable reuse across different TSE components.

### pagedir

The pagedir module provides helper functions for working with the Crawler's page directory. It is used to validate that a directory contains Crawler-produced pages and to retrieve URLs from stored page files.

`pagedir_validate`: Ensures that a given directory contains valid Crawler-generated pages.

### libcs50
The libcs50 module provides essential data structures and utility functions used throughout the Querier.

`counters`: Used to store and manipulate document scores.
`mem`: Provides memory allocation functions with built-in error handling.
`file`: Provides helper functions for reading files.
By leveraging libcs50, we simplify memory management and ensure robust error handling across our program.

## **Function Prototypes**

### Querier (querier.c)

Detailed descriptions of each function's interface is provided as a paragraph comment prior to each function's implementation in `querier.c` and is not repeated here.

```c
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

void callbackSetMinimum(void *arg, const int key, const int count);
void callbackAddCounts(void *arg, const int key, const int count);
void callbackCountNonZero(void *arg, const int key, const int count);
void callbackStoreDocScores(void *arg, const int key, const int count);
```

## **Error Handling and Recovery**
The Querier follows a rigorous error-handling approach to ensure robustness and graceful failure when encountering issues.

**Command-line Argument Validation**
All command-line arguments are validated before any work begins.

If invalid arguments are provided, Querier prints an error message to stderr and exits with a non-zero status.
This prevents unnecessary resource allocation and guarantees that only valid inputs are processed.

**Handling File I/O Errors**
Querier assumes that pageDirectory contains files named sequentially (1, 2, 3, ...) without gaps.

If critical files (e.g., indexFilename, pageDirectory/.crawler, or pageDirectory/1) are missing, Querier exits immediately (exit(1);).

If an individual page file (e.g., pageDirectory/2, pageDirectory/3) is missing, this is an unrecoverable error, and Querier exits rather than continuing execution with incomplete data.

**Memory Allocation Failures**
Querier proactively checks for out-of-memory errors using mem_assert().

If memory allocation fails, Querier prints an error message to stderr and exits with a non-zero status.

Since memory failures are rare, the program is allowed to terminate cleanly in such cases rather than attempting recovery.

**Defensive Programming**
All functions validate their inputs to avoid segmentation faults and undefined behavior.

Functions that receive invalid parameters will exit immediately using mem_assert() variants rather than allowing further execution.

This ensures errors are caught early and explicitly, making debugging easier.

**Handling Internal (Recoverable) Errors**
Certain errors are handled internally rather than causing an immediate program exit:

- Invalid queries: If a query contains invalid syntax, Querier prints an error message but allows the user to re-enter a valid query.
- File reading issues: If a page file cannot be read, Querier exits immediately (exit(1);) rather than continuing with missing documents.
- Empty query results: If no documents match a query, Querier prints "No documents match." instead of treating it as an error.

## Testing Plan

The testing plan ensures the correctness, robustness, and memory safety of the `querier` module through **unit testing**, **integration/system testing**, and **memory safety testing with Valgrind**.

---

### **Unit Testing**
The `querier` module primarily interacts with **the index, the page directory, and user queries**. Since it is not easily separable into isolated units, unit testing is limited. Instead, **functional correctness is verified through system-level tests**.

- **Expected behavior**:
  - The querier should correctly process queries with **AND/OR precedence**.
  - It should **reject invalid inputs** (e.g., empty queries, standalone `AND`/`OR`, special characters).
  - It should return **correctly ranked document results**.

- **Edge cases to test**:
  - **Valid queries**: `home`, `algorithm AND search`, `dog OR cat`
  - **Invalid queries**: `AND algorithm`, `cat OR AND dog`, `&&search!!`
  - **No matches**: Queries that return zero results
  - **Duplicate words**: Ensuring repeated words don't alter ranking
  - **Long queries**: Handling queries with many terms

---

### **Integration & System Testing**
A **Bash script (`testing.sh`)** automates testing by running `querier` with various inputs:

#### **Standard Test Cases**
The script runs `querier` over **letters-1, toscrape-1, and toscrape-2**, testing:
```bash
# Letters site - simple valid queries
./querier ~/cs50-dev/shared/tse/output/letters-1 ~/cs50-dev/shared/tse/output/letters-1.index <<EOF
home
home algorithm
EOF
```
#### Toscrape site - queries with AND/OR logic
```bash
./querier ~/cs50-dev/shared/tse/output/toscrape-1 ~/cs50-dev/shared/tse/output/toscrape-1.index <<EOF
indonesia OR doctor
glass AND chronicles
stop AND running OR suddenly
EOF
```

#### Toscrape site - edge cases
```bash
./querier ~/cs50-dev/shared/tse/output/toscrape-2 ~/cs50-dev/shared/tse/output/toscrape-2.index <<EOF
&roaming
roaming-supper
EOF
```

**Error Handling Tests**

querier should properly reject:
```bash
./querier ~/cs50-dev/shared/tse/output/toscrape-2 ~/cs50-dev/shared/tse/output/toscrape-2.index <<EOF
AND
OR
AND dog or jump
jump and or dog
jump dog and
EOF
```

Tests that AND and OR are not standalone or consecutive.

**Fuzz Testing**

The fuzz testing tool (fuzzquery) generates random queries to check for unexpected crashes:

```bash
echo "Running fuzzquery and piping directly into querier..."
~/cs50-dev/shared/tse/fuzzquery ~/cs50-dev/shared/tse/output/toscrape-1.index 10 1 | \
./querier ~/cs50-dev/shared/tse/output/toscrape-1 ~/cs50-dev/shared/tse/output/toscrape-1.index
This simulates thousands of real-world queries.
```

**Memory Safety Testing with Valgrind**

To detect memory leaks, the script runs querier under Valgrind on all three files used in testing.sh