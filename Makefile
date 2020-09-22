all: build rm

build: main.o lexer.o parser.o codeGen.o
	g++ main.o lexer.o parser.o codeGen.o -o exec

main.o: src/main.cpp
	g++ -c src/main.cpp

lexer.o: src/compiler_parts/lexer/lexer.cpp
	g++ -c src/compiler_parts/lexer/lexer.cpp src/compiler_parts/lexer/lexer.h

parser.o: src/compiler_parts/parser/parser.cpp
	g++ -c src/compiler_parts/parser/parser.cpp src/compiler_parts/parser/parser.h

codeGen.o: src/compiler_parts/codeGen/codeGen.cpp
	g++ -c src/compiler_parts/codeGen/codeGen.cpp src/compiler_parts/codeGen/codeGen.h
	
rm: 
	rm *.o
	rm g++ -c src/compiler_parts/lexer/*.gch
	rm g++ -c src/compiler_parts/parser/*.gch
	rm g++ -c src/compiler_parts/codeGen/*.gch
