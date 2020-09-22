#include <iostream>
#include "compiler_parts/codeGen/codeGen.h"

int main(int argc, char* argv[])
{
	std::string fileName = "data/NOD.rb";

	Lexer lexer;
	lexer.readText(fileName);
	lexer.printTokenList();

	Parser parser(&lexer);
	parser.start();
	parser.showTree();

	CodeGen codeGen;
	codeGen.init(parser.getSyntaxTreeRoot());
	codeGen.start();
	codeGen.dumpAsm();
}
