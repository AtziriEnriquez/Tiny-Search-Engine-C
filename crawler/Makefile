# Makefile for crawler module in CS50 Tiny Search Engine
#
# Atziri Enriquez - 02/12/25

# Compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
LDFLAGS = -L../common -l:common.a -L../libcs50 -l:libcs50-given.a

# Source files and objects
SRCS = crawler.c
OBJS = $(SRCS:.c=.o)

# Executable
EXE = crawler

# Default target
all: $(EXE)

# Build the crawler executable
$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDFLAGS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests
.PHONY: test
test: $(EXE)
	bash -v testing.sh | tee testing.out

# Clean up compiled files
.PHONY: clean
clean:
	rm -f $(OBJS) $(EXE) testing.out
