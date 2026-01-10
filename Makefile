CC      = gcc
CFLAGS  = -O3 -march=native -flto -Wall -Wextra -std=c11 -fPIC -fopenmp `sdl2-config --cflags` -Iinclude
LDFLAGS = -fopenmp `sdl2-config --libs`

# Main library
LIB_SRC     = src/main.c
LIB_OBJ     = build/main.o
LIB_FILE    = build/librenderer.a

# Test mini-project
TEST_SRC    = test/src/test.c
TEST_OBJ    = test/build/test.o
TEST_BIN    = test/build/test

all: $(LIB_FILE) $(TEST_BIN)

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

# Build test object
$(TEST_OBJ): $(TEST_SRC) | test/build
	$(CC) $(CFLAGS) -c $(TEST_SRC) -o $(TEST_OBJ)

# Link test executable against the library
$(TEST_BIN): $(TEST_OBJ) $(LIB_FILE)
	$(CC) $(CFLAGS) $(TEST_OBJ) -Lbuild -lrenderer $(LDFLAGS) -o $(TEST_BIN)

clean:
	rm -rf build
	rm -rf test/build

.PHONY: all clean
