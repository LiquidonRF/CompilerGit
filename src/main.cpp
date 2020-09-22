#include <iostream>
#include "compiler_parts/codeGen/codeGen.h"

int main(int argc, char* argv[])
{
	if (argc == 0 || argc == 1)
	{
		std::cout << "Usage:\n\t./exec <input_program.rb> [Options]\n";
		std::cout << "Options:\n\t";
		std::cout << "--dump-tokens - display the result of the lexical analyzer\n\t";
		std::cout << "--dump-ast - display AST\n\t";
		std::cout << "--dump-asm - display assembler\n";
			
		return 0;
	}

	Lexer lexer;
	lexer.readText(argv[1]);

	Parser parser(&lexer);
	parser.start();

	CodeGen codeGen;
	codeGen.init(parser.getSyntaxTreeRoot());
	codeGen.start();
	codeGen.dumpAsm();

	for (size_t iter = 1; iter < argc; iter++)
	{
		if (argv[iter] == "--dump-tokens")
		{
			lexer.printTokenList();
		}
		else if (argv[iter] == "--dump-ast") 
		{
			parser.showTree();
		}
		else if (argv[iter] == "--dump-asm") 
		{
			codeGen.dumpAsm();
		}
	}

	return 0;
}