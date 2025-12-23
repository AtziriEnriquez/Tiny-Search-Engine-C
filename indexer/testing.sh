#!/bin/bash

# testing.sh - Testing script for the Indexer and Indextest programs
#
# Author: Atziri Enriquez
# Date: 02/19/25
#
# Description:
# This script performs automated testing for the Tiny Search Engine's Indexer and Indextest
# programs. It verifies that the indexer correctly generates index files from a given
# page directory and that indextest accurately loads and writes back index files while
# maintaining data integrity.
#
# The script runs:
# - **Valid Tests:** Executes indexer on multiple datasets, checks index file consistency
#   using indextest, and compares outputs with indexcmp, no output means success in copying one index file to another.
# - **Edge Cases:** Tests small and large datasets.
# - **Invalid Argument Tests:** Ensures indexer and indextest handle incorrect inputs
#   (missing arguments, extra arguments, invalid paths, etc.).
# - **Special Cases:**
#   - Running indexer on a valid non-crawler directory (should fail).
#   - Running indexer on an empty crawler directory (should handle gracefully).
# - **Memory Leak Testing:** Uses Valgrind to check for memory leaks in both indexer
#   and indextest.

TESTDIR="../data"
SHAREDDIR="../../shared/tse/output"
INDEXCMP="$HOME/cs50-dev/shared/tse/indexcmp"
VALGRIND_LOG="$TESTDIR/valgrind.log"

mkdir -p $TESTDIR
# ------------------------------------
# 1. Valid Indexer and Indextest Tests
# ------------------------------------

echo "Running indexer on $SHAREDDIR/letters-2..."
./indexer $SHAREDDIR/letters-2 $TESTDIR/letters-2.index

echo "Running indextest on $TESTDIR/letters-2.index..."
./indextest $TESTDIR/letters-2.index $TESTDIR/new-letters-2.index

echo "Comparing index files with indexcmp..."
$INDEXCMP $TESTDIR/letters-2.index $TESTDIR/new-letters-2.index

echo "Running indexer on $SHAREDDIR/letters-3..."
./indexer $SHAREDDIR/letters-3 $TESTDIR/letters-3.index

echo "Running indextest on $TESTDIR/letters-3.index..."
./indextest $TESTDIR/letters-3.index $TESTDIR/new-letters-3.index

echo "Comparing index files with indexcmp..."
$INDEXCMP $TESTDIR/letters-3.index $TESTDIR/new-letters-3.index

# ------------------------------------
# 2. Invalid Argument Tests
# ------------------------------------

echo "TEST 1: No arguments (should fail)"
./indexer

echo "TEST 2: One argument (should fail)"
./indexer onlyonearg

echo "TEST 3: Three or more arguments (should fail)"
./indexer arg1 arg2 arg3

# ------------------------------------
# 3. Invalid pageDirectory (Non-existent path)
# ------------------------------------

echo "TEST 4: Invalid pageDirectory (non-existent path, should fail)"
./indexer /invalid/path $TESTDIR/bad.index

# ------------------------------------
# 4. Invalid pageDirectory (Existing but NOT a crawler directory)
# ------------------------------------

mkdir -p $TESTDIR/notcrawler
echo "TEST 5: Valid directory but NOT a crawler directory (should fail)"
./indexer $TESTDIR/notcrawler $TESTDIR/bad.index

# ------------------------------------
# 5. Invalid indexFile (Non-existent path)
# ------------------------------------

echo "TEST 6: Invalid indexFile (non-existent path, should fail)"
./indexer $SHAREDDIR/letters-2 /invalid/path/bad.index

# ------------------------------------
# 6. Invalid indexFile (Read-only directory)
# ------------------------------------

mkdir -p $TESTDIR/readonly-dir
chmod 555 $TESTDIR/readonly-dir  # Set directory as read-only

echo "TEST 7: Index file in a read-only directory (should fail)"
./indexer $SHAREDDIR/letters-2 $TESTDIR/readonly-dir/bad.index

# ------------------------------------
# 7. Invalid indexFile (Existing, read-only file)
# ------------------------------------

touch $TESTDIR/readonly.index
chmod 444 $TESTDIR/readonly.index  # Make the file read-only

echo "TEST 8: Existing, read-only index file (should fail)"
./indexer $SHAREDDIR/letters-2 $TESTDIR/readonly.index

chmod 644 $TESTDIR/readonly.index  # Restore file permissions after test

# ------------------------------------
# Extra Edge Cases
# ------------------------------------

mkdir -p $TESTDIR/empty-crawler
touch $TESTDIR/empty-crawler/.crawler
echo "TEST 9: Indexer on an empty crawler directory (should handle gracefully)"
./indexer $TESTDIR/empty-crawler $TESTDIR/empty.index

# ------------------------------------
# Run Valgrind Memory Leak Tests
# ------------------------------------

echo "Running Valgrind on indexer..."
valgrind --leak-check=full --show-leak-kinds=all ./indexer $SHAREDDIR/toscrape-1 $TESTDIR/toscrape-1.index

echo "Running Valgrind on indextest..."
valgrind --leak-check=full --show-leak-kinds=all ./indextest $TESTDIR/toscrape-1.index $TESTDIR/new-toscrape-1.index

echo "All tests completed!"
