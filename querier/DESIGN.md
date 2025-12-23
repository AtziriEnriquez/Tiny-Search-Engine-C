# CS50 TSE Querier

## Design Specification

## **1. User Interface**
The `querier` module is executed with:

- **pageDirectory**: Path to the directory containing the crawled pages.
- **indexFilename**: The file containing word-document frequency mappings.

It accepts **queries from standard input**, processes them, and outputs **ranked document results**.

## **2. Inputs and Outputs**
### **Inputs**
- The **page directory** contains pages fetched by the **TSE Crawler**.
- The **index file** contains word-document mappings produced by the **TSE Indexer**.
- The **query** is a space-separated list of words read from `stdin`.

### **Outputs**
- Prints a **ranked list of matching documents** to `stdout`, sorted by relevance.
- If no documents match the query, it prints:
- Error messages are printed to `stderr`.

## **3. Functional Decomposition into Modules**
The querier consists of the following ***key*** functions:

Note: these are the key functions, not all

- **`parseArgs()`**: Validates command-line arguments (`pageDirectory`, `indexFilename`).
- **`prompt()`**: Prints `"Query? "` to standard output if running interactively.
- **`processQuery()`**: Parses, validates, and executes the search query.
- **`queryEvaluate()`**: Evaluates the query using **boolean logic** (`AND`, `OR`).
- **`intersectCounters()`**: Computes the **AND** operation on document sets.
- **`unionCounters()`**: Computes the **OR** operation on document sets.
- **`printRankedResults()`**: Sorts and displays the query results in descending order.
- **Helper functions**:
- `queryTokenize()` - Tokenizes input query into words.
- `isValidCharacters()` - Ensures query contains only alphabetic characters.
- `isValidQuerySyntax()` - Validates AND/OR positioning.

## **4. Pseudo Code**
This section outlines the querier's logic in **plain-English pseudocode**.

### **Step 1: Parse Command-line Arguments**
```plaintext
parseArgs(argc, argv):
if (argc != 3):
  print "Usage: ./querier pageDirectory indexFilename"
  exit(1)

pageDirectory = argv[1]
indexFilename = argv[2]

if (!pagedir_validate(pageDirectory)):
  print "Error: Invalid page directory"
  exit(1)

if (index file cannot be opened):
  print "Error: Could not open index file"
  exit(1)

main():
  Load index from indexFilename into index_t *index
  while (prompt user for query):
    Read query from stdin
    processQuery(query, index, pageDirectory)
  Free memory and exit
```

### **Step 2: Read and Process Queries**
```plaintext
main():
  Load index from indexFilename into index_t *index
  while (prompt user for query):
    Read query from stdin
    processQuery(query, index, pageDirectory)
  Free memory and exit
```

### **Step 3: Process User Query**
```plaintext
processQuery(query, index, pageDirectory):
  if (!isValidCharacters(query)):
    print "Error: bad character in query"
    return

  words = tokenize(query)
  if (words are empty):
    return

  if (!isValidQuerySyntax(words)):
    print "Error: Invalid query syntax"
    return

  print "Query:" words  # Echo the parsed query
  result = queryEvaluate(words, index)
  printRankedResults(result, pageDirectory)
```

### **Step 4: Query Evaluation**
```plaintext
queryEvaluate(words, index):
  andSequence = NULL
  orSequence = NULL
  andSequenceInvalid = false

  for each word in words:
    if (word is "or"):
      matchMerge(andSequence, orSequence)
      continue

    if (word is "and"):
      continue

    wordMatch = index_find(index, word)
    
    if (wordMatch is NULL):
      andSequenceInvalid = true
      free(andSequence)
      andSequence = NULL
    else:
      if (andSequence is NULL):
        andSequence = copyCounters(wordMatch)
      else:
        intersectCounters(andSequence, wordMatch)

  matchMerge(andSequence, orSequence)
  return orSequence
```
### **Step 5: Merging AND/OR Sequences**
```plaintext
matchMerge(andSequence, orSequence):
  if (andSequence is not NULL):
    if (orSequence is NULL):
      orSequence = new counters structure
    unionCounters(orSequence, andSequence)
    free(andSequence)
```
### **Step 6: Print Ranked Results**
```plaintext
printRankedResults(result, pageDirectory):
  if (result is empty):
    print "No documents match."
    return

  while (true):
    find document with highest score
    if (maxScore ≤ 0):
      break
    
    read URL from pageDirectory
    print "score <score> doc <docID>: <URL>"
    mark document as processed
```
## **5. Major Data Structures**

Data Structure	Description

**index_t**:	A hashtable mapping words to counters_t structures.

**counters_t**:	Stores (docID, count) pairs for documents containing a word.

Query Storage (char **words): Holds tokenized query words.

Intermediate Results (counters_t *andSequence, counters_t *orSequence): Used for query evaluation logic.

## **6. Testing Plan**

**Functional Testing**

Run valid queries and ensure correct ranked output.

Test AND/OR precedence (e.g., "dog AND cat OR fish").

Verify empty and invalid queries return correct errors.

Test single-word, multi-word, and duplicate-word queries.

**Edge Case Testing**

Queries with leading/trailing/multiple spaces (e.g., " apple AND orange ").

Queries with consecutive AND/OR operators (e.g., "apple AND OR banana").

Queries containing non-alphabetic characters (e.g., "apple123").

**Fuzz Testing**

Use fuzzquery to generate random queries and ensure querier does not crash.

**Memory Safety Testing**

Run Valgrind on querier to detect memory leaks and invalid accesses.
```bash
valgrind --leak-check=full --show-leak-kinds=all ./querier pageDirectory indexFilename
```