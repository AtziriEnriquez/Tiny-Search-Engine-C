# Makefile for querier module in CS50 Tiny Search Engine
#
# Atziri Enriquez - 03/1/2025

# Compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
LIBS = ../common/common.a ../libcs50/libcs50-given.a  # Link with libcs50-given.a

# Files
OBJ = querier.o
EXE = querier

# Default rule: build querier
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Compile querier.c
querier.o: querier.c
	$(CC) $(CFLAGS) -c querier.c

# Run tests
test: $(EXE)
	bash -v testing.sh

# Clean up compiled files
clean:
	rm -f $(EXE) $(OBJ) testing.out
