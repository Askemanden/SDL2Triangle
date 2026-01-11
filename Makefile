CC      = gcc
CFLAGS  = -O3 -march=native -flto -Wall -Wextra -std=c11 -fPIC -fopenmp `sdl2-config --cflags` -Iinclude
LDFLAGS = -fopenmp `sdl2-config --libs` -mconsole

# Main library
LIB_SRC     = src/main.c
LIB_OBJ     = build/main.o
LIB_FILE    = build/librenderer.a

# Test sources (automatically picks up all .c files)
TEST_SRCS   = $(wildcard test/src/*.c)
TEST_OBJS   = $(patsubst test/src/%.c, test/build/%.o, $(TEST_SRCS))
TEST_BINS   = $(patsubst test/src/%.c, test/build/%,   $(TEST_SRCS))

# Default: build only the library
all: $(LIB_FILE)

# Build everything (library + all tests)
everything: $(LIB_FILE) $(TEST_BINS)

# Build only tests
test: $(TEST_BINS)

# Ensure build dirs exist
build:
	mkdir -p build

test/build:
	mkdir -p test/build

# Build library object
$(LIB_OBJ): $(LIB_SRC) | build
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

# Archive static library
$(LIB_FILE): $(LIB_OBJ)
	ar rcs $(LIB_FILE) $(LIB_OBJ)

# Pattern rule: build any test .o file
test/build/%.o: test/src/%.c | test/build
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule: link any test executable
test/build/%: test/build/%.o $(LIB_FILE)
	$(CC) $(CFLAGS) $< -Lbuild -lrenderer $(LDFLAGS) -o $@

clean:
	rm -rf build
	rm -rf test/build

.PHONY: all clean test everything
