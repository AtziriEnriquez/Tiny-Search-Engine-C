#!/bin/bash

# Testing script for the querier module in Tiny Search Engine
#
# This script automates functional, integration, and memory tests 
# for the querier module. It runs various query test cases, 
# including:
#   - Standard queries with AND/OR precedence
#   - Edge cases (invalid characters, empty queries, incorrect syntax)
#   - Large query stress testing
#   - Fuzz testing using fuzzquery
#   - Memory leak detection with Valgrind
#
# Usage:
#   chmod +x testing.sh
#   ./testing.sh
#
# The script assumes the querier executable is compiled and located
# in the same directory. It uses test data from the CS50 TSE sites 
# (letters, toscrape, wikipedia).
#
# Written for Lab 6 - Tiny Search Engine

# Set paths to shared TSE output files
SHARED_DIR=~/cs50-dev/shared/tse
FUZZQUERY=$SHARED_DIR/fuzzquery  # Path to fuzzquery

QUERIER=./querier  # Path to querier executable

# === RUNNING STANDARD TEST CASES ===

# 1. Basic valid query test (letters-1)
$QUERIER $SHARED_DIR/output/letters-1 $SHARED_DIR/output/letters-1.index <<EOF
home
home algorithm
EOF

# 2. Complex query with AND/OR precedence (toscrape-1)
$QUERIER $SHARED_DIR/output/toscrape-1 $SHARED_DIR/output/toscrape-1.index <<EOF
indonesia OR doctor
glass AND chronicles
stop AND running OR suddenly
EOF

# 3. Edge cases: invalid characters (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
&roaming
roaming-supper
EOF

# 4. Edge case: Empty query (toscrape-2)
echo "" | $QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index

# 5. Handling unexpected whitespace (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
   bachelor    true   
flavor       AND    supper   
   happily OR  shopping  
EOF

# 6. Invalid standalone AND/OR (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
AND
OR
AND dog or jump
jump and or dog
jump dog and
EOF

# 7. No common documents (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
careers AND asher
EOF

# 8. Long query stress test (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
real AND recently AND you AND tales AND decision AND type
EOF

# 9. Duplicate words in query (toscrape-2)
$QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index <<EOF
bread bread bread
rivers AND rivers
EOF

# === FUZZ TESTING QUERIER ===

echo "Running fuzzquery and piping directly into querier..."
$FUZZQUERY $SHARED_DIR/output/toscrape-1.index 10 1 | $QUERIER $SHARED_DIR/output/toscrape-1 $SHARED_DIR/output/toscrape-1.index

# === FINAL MEMORY TESTING WITH VALGRIND ===
echo "Running Valgrind for memory testing..."

# Run Valgrind on querier for letters-1
echo "home algorithm" | valgrind --leak-check=full --show-leak-kinds=all $QUERIER $SHARED_DIR/output/letters-1 $SHARED_DIR/output/letters-1.index

# Run Valgrind on querier for toscrape-1
echo "indonesia OR chronicles" | valgrind --leak-check=full --show-leak-kinds=all $QUERIER $SHARED_DIR/output/toscrape-1 $SHARED_DIR/output/toscrape-1.index

# Run Valgrind on querier for toscrape-2
echo "careers AND asher" | valgrind --leak-check=full --show-leak-kinds=all $QUERIER $SHARED_DIR/output/toscrape-2 $SHARED_DIR/output/toscrape-2.index

