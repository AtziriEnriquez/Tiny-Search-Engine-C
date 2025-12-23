# Makefile for common module in CS50 Tiny Search Engine
#
# Atziri Enriquez - 02/12/25

# Compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -g

# Source files
SRCS = index.c pagedir.c word.c
OBJS = $(SRCS:.c=.o)

# Static library
LIB = common.a

# Default target: Build the static library
all: $(LIB)

# Create the static library
$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(OBJS) $(LIB)
