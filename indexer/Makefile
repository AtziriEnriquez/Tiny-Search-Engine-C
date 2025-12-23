# Makefile for indexer module in CS50 Tiny Search Engine
#
# Atziri Enriquez - 02/19/25

# Compiler
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -g
LDFLAGS = -L../common -l:common.a -L../libcs50 -l:libcs50-given.a

# Directories
COMMON_DIR = ../common
LIBCS50_DIR = ../libcs50

# Object files
COMMON_OBJS = $(COMMON_DIR)/index.o $(COMMON_DIR)/pagedir.o $(COMMON_DIR)/word.o
INDEXER_OBJS = indexer.o $(COMMON_OBJS)
INDEXTEST_OBJS = indextest.o $(COMMON_DIR)/index.o $(COMMON_DIR)/word.o

# Executables
EXECS = indexer indextest

# Default target
.PHONY: all clean test

all: $(EXECS)

# Compile indexer
indexer: $(INDEXER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile indextest
indextest: $(INDEXTEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(EXECS) *.o $(COMMON_DIR)/*.o

# Run tests
test:
	bash -v testing.sh