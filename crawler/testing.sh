#!/bin/bash

# testing.sh - Automated testing script for TSE Crawler
#
# This script tests the functionality and memory safety of the Tiny Search Engine (TSE) Crawler.
# It performs the following:
#
# 1. **Cleanup** - Removes old test directories to ensure a fresh test run.
# 2. **Setup** - Creates necessary output directories for test cases.
# 3. **Functional Tests** - Runs the Crawler on different websites at various depths to verify correctness:
#    - letters site (depth 0 and 10)
#    - toscrape site (depth 0 and 1/2)
#    - Wikipedia site (depth 0 and 1)
# 4. **Memory Leak Detection (Valgrind)** - Runs the Crawler under Valgrind to check for memory leaks.
# 5. **Validation** - Confirms that the crawler output files were generated successfully.
# 6. **Cleanup** - Removes test directories after execution.
# 7. **Results Display** - Outputs Valgrind memory leak report for review.
#
# COSC 050, TSE Lab 4
# Atziri Enriquez, 2/11/2025

# Define test directory
TEST_DIR="data"

# Remove previous output files to ensure clean test runs
echo -e "\nCleaning previous test output..."
rm -rf $TEST_DIR

# Create necessary output directories for different test cases
mkdir -p $TEST_DIR/letters-0
mkdir -p $TEST_DIR/letters-10
mkdir -p $TEST_DIR/toscrape-0
mkdir -p $TEST_DIR/toscrape-1
mkdir -p $TEST_DIR/wikipedia-0
mkdir -p $TEST_DIR/wikipedia-1

# Run tests
echo -e "\nRunning tests...\n"

echo -e "\n===== Crawling letters site at depth 0 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html $TEST_DIR/letters-0 0

echo -e "\n===== Crawling letters site at max depth 10 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/letters/index.html $TEST_DIR/letters-10 10

echo -e "\n===== Crawling toscrape site at depth 0 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/toscrape/index.html $TEST_DIR/toscrape-0 0

echo -e "\n===== Crawling toscrape site at depth 1 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/toscrape/index.html $TEST_DIR/toscrape-1 1

echo -e "\n===== Crawling Wikipedia site at depth 0 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/index.html $TEST_DIR/wikipedia-0 0

echo -e "\n===== Crawling Wikipedia site at depth 1 =====\n"
valgrind ./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/index.html $TEST_DIR/wikipedia-1 1

# Cleanup test directory
echo -e "\nCleaning up test directories...\n"
rm -rf $TEST_DIR

echo -e "\n===== All tests completed! =====\n"