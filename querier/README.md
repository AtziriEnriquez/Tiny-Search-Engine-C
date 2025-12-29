# TSE Querier

## README.md

### Atziri Enriquez (Github User: AtziriEnriquez)

### Assumptions

**1. Adherence to the Requirements Spec**

All assumptions outlined in the Requirements Spec were followed.

My implementation closely follows the spec, ensuring compliance with the expected behavior.

**2. Query Processing & Tokenization**

The maximum number of words in a query is estimated using a worst-case scenario, assuming every other character is a space.

This ensures that sufficient memory is allocated for tokenizing the query without excessive allocation.

**3. Data Structures & Memory Allocation**

Dynamic Hashtable Sizing in index_load():
The hashtable size is determined dynamically based on the number of lines in the index file:
```c
index_t* index = index_new(file_numLines(fp));
```
This ensures efficient memory allocation, scaling based on the actual number of words in the index.

**4. System Compatibility & Portability**

Using getline() with _POSIX_C_SOURCE 200809L:

_POSIX_C_SOURCE 200809L is defined to enable getline() and fileno() in a portable way.

This ensures compatibility across POSIX-compliant systems.

Global Array Definition:
Due to _POSIX_C_SOURCE constraints, I had to define a global array in some cases to ensure proper compilation.

#### How My Implementation Differs from the Specs

In testing.sh, I do not copy over fuzzquery into my repo, as it is a executable.

**1. Sorting Search Results with qsort() instead of Selection Sort**:
Instead of using selection sort, I implemented qsort() with compareScores() to sort document scores in descending order.

Why qsort()?
- It provides better time efficiency than a manual sorting approach.
- It reduces the complexity of the implementation while maintaining correctness.

**2. File Handling & Storage**:

 Fixed Filename Buffer Size (filename[256])

When printing results, I used a fixed buffer size of char filename[256].

Why 256?

After analyzing potential path lengths, 256 bytes is reasonable to accommodate valid file paths without excessive memory allocation.

**3. Added a Separator Line After Each Query for Clarity**

After processing each query, I added a separator line (-----------------------------------------------) to the output.

Why?

This was not required in the specs or instructions, but it enhances readability by clearly separating the results of different queries.

It helps users distinguish between consecutive query results when running multiple queries in a session.

### Implementation Status

My implementation does not fail to work in any way that I am aware of.

The program runs successfully with no known issues.

It meets all the functional requirements outlined in the spec, including query parsing, processing, ranking, and printing results correctly.

Valgrind reports no memory leaks or errors.

Below is a breakdown of the functionality and its completion status based on the grading rubric:

**1. Basic Query Processing**

The querier correctly retrieves and prints the set of documents that contain all words in the query.

Initially, operators (AND, OR) were treated as regular words, fulfilling the basic query functionality.

**2. Handling AND and OR Operators**

The querier properly processes AND and OR operators, treating them as left-associative with equal precedence.

Queries such as word1 AND word2 OR word3 are processed sequentially without additional operator precedence rules.

**3. Implementing AND Precedence Over OR**

The querier correctly prioritizes AND over OR when evaluating queries.

Queries such as word1 OR word2 AND word3 are evaluated as word1 OR (word2 AND word3), ensuring proper grouping of AND sequences.

**4. Ranking and Sorting Results**

The querier sorts matching documents in decreasing order by score before printing results.

Instead of the selection sort approach suggested in the specs, I used qsort() with compareScores() for improved time efficiency.

**Other Required Components**

querier.c (Implemented)
Successfully processes queries using common.a and libcs50.a libraries.

Makefile (Implemented)
Default target compiles querier.c into the querier executable.
Includes make clean to remove binaries and object files.
Includes make test to run automated tests.

testing.sh (Implemented)
A comprehensive Bash script tests various querier functionalities.
Includes:
- Basic queries
- Edge cases
- Performance tests
- Invalid input handling
Runs via:
```
make test &> testing.out
```
Output of tests is saved in testing.out.

.gitignore (Implemented)
Ensures that binaries, object files, and unnecessary artifacts are ignored.
