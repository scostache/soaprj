CFLAGS += -Wall
CC = g++

TEST_SRC = test.cpp

SRCS = token.cpp parser.c ast.cpp

OBJS = token.o parser.o ast.o

.o:
	$(CC) $(CFLAGS) -c $< -o $@

libparser.a : $(OBJS)
	ar rcs $@ $^

test: $(TEST_SRC) libparser.a
	$(CC) $(CFLAGS) $(TEST_SRC) -o $@ -lparser -L.

all: libparser.a test


clean:
	rm -f *.o
	rm -rf test

