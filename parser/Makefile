all:
	g++ -c token.cpp
	g++ -c parser.c
	g++ -c ast.cpp
	g++ -c test.cpp
	g++ -o test test.o token.o parser.o ast.o
clean:
	rm token.o
	rm test.o
	rm parser.o
	rm ast.o
	rm test

