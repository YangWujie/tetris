CC = gcc
CFLAGS = -Wall -g -I./src -I/usr/local/include
LDFLAGS_TEST = -L/usr/local/lib -lcunit

TARGET = tetris
TEST_TARGET = test_tetris

SRC_FILES = src/tetris.c
TEST_FILES = tests/test_tetris.c

OBJ_FILES = $(SRC_FILES:.c=.o)
TEST_OBJ_FILES = $(TEST_FILES:.c=.o)

#tests/%.o: tests/%.c
#	$(CC) $(CFLAGS) -c $< -o $@


all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET)

test: $(TEST_OBJ_FILES) $(OBJ_FILES)
	$(CC) $(TEST_OBJ_FILES) $(OBJ_FILES) -o $(TEST_TARGET) $(LDFLAGS_TEST)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJ_FILES) $(TEST_OBJ_FILES) $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
