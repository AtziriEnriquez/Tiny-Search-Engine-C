# Tiny Search Engine (C)

A modular C-based search engine consisting of a **web crawler**, **inverted indexer**, and **query processor**.

> Built as a systems-focused project emphasizing modular design, defensive programming, and memory correctness.

---

## Features

- **Depth-limited crawler** that fetches and stores pages locally
- **Inverted index** with word-frequency tracking
- **Query engine** that ranks results from indexed documents
- **Testing + memory validation** (unit tests, Valgrind-friendly)

---

## Project Structure

```text
├── common/     # shared helpers/utilities
├── crawler/    # crawl seed URL -> page directory
├── indexer/    # build inverted index from crawled pages
├── querier/    # query an index and rank results
├── libcs50/    # provided support library
├── Makefile
└── README.md
```
## Build

From the repository root:
```bash
make
```

To clean build artifacts:
```bash
make clean
```

## Usage (Quickstart)
1) Crawl pages
```bash
./crawler <seedURL> <pageDirectory> <maxDepth>
```

2) Build an index from crawled pages
```bash
./indexer <pageDirectory> <indexFilename>
```

3) Query the index
```bash
./querier <pageDirectory> <indexFilename>
```

## Testing & Memory Checks
Run tests:
```bash
make test
```

Run Valgrind (example pattern):
```bash
valgrind --leak-check=full --show-leak-kinds=all ./indexer...
```
