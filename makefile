CC = gcc
CFLAGS = -Wall -g -O2 -I./src -I/usr/local/include
LDFLAGS_TEST = -L/usr/local/lib -lcunit

TARGET = tetris
TEST_TARGET = test_tetris

SRC_FILES = src/tetris.c src/print_utils.c
TEST_FILES = tests/test_tetris.c

OBJ_FILES = $(SRC_FILES:.c=.o)
TEST_OBJ_FILES = $(TEST_FILES:.c=.o)
MAIN_OBJ = src/main.o

all: $(TARGET)

$(TARGET): $(OBJ_FILES) $(MAIN_OBJ)
	$(CC) $(OBJ_FILES) $(MAIN_OBJ) -o $(TARGET)

test: $(TEST_OBJ_FILES) $(OBJ_FILES)
	$(CC) $(TEST_OBJ_FILES) $(OBJ_FILES) -o $(TEST_TARGET) $(LDFLAGS_TEST)
	./$(TEST_TARGET)

src/tetris.o: src/tetris.c src/tetris.h
src/main.o: src/main.c src/tetris.h
tests/test_tetris.o: tests/test_tetris.c src/tetris.h


clean:
	rm -f $(OBJ_FILES) $(TEST_OBJ_FILES) $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
