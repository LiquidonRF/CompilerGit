all: make

make: main.o lexer.o parser.o godgen.o
	g++ main.o lexer.o parser.o godgen.o -o exec

main.o: main.cpp
	g++ -c src/main.cpp

lexer.o: lexer.cpp
	g++ -c src\compiler_parts\lexer\lexer.cpp

parser.o: parser.cpp
	g++ -c src\compiler_parts\parser\parser.cpp

lexer.o: codeGen.cpp
	g++ -c src\compiler_parts\codeGen\codeGen.cpp