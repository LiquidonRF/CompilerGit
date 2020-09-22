#include "codeGen.h"

bool CodeGen::haveInVarTable(SyntaxNode *node)
{
	auto search = m_varTable->find(node->getToken()->lexema);

	if (search == m_varTable->end())
		return false;

	return true;
}

std::string CodeGen::getAddrFromVarTable(SyntaxNode *node)
{
	auto search = m_varTable->find(node->getToken()->lexema);

	return search->second;
}

void CodeGen::init(SyntaxNode *root)
{
	m_syntaxNodeRoot = root;

	m_dotData->push_back(".data\n");
	m_dotText->push_back(".text\n.globl main\nmain:\n");
	m_dotText->push_back("\t\tpushq    %rbp\n");
	m_dotText->push_back("\t\tmovq    %rsp, %rbp\n");

	m_registers->push_back(new Register("%eax", false));
	m_registers->push_back(new Register("%ebx", false));
	m_registers->push_back(new Register("%ecx", false));
	m_registers->push_back(new Register("%edx", false));
	m_registers->push_back(new Register("%esp", false));
	m_registers->push_back(new Register("%ebp", false));
	m_registers->push_back(new Register("%esi", false));
	m_registers->push_back(new Register("%edi", false));
	m_registers->push_back(new Register("%rbp", false));
	m_registers->push_back(new Register("%rsp", false));
	m_registers->push_back(new Register("%jne", false));
	m_registers->push_back(new Register("%while", false));
	m_registers->push_back(new Register("%LC0", false));
	m_registers->push_back(new Register("%exit", false));
	m_registers->push_back(new Register("%print", false));
}

void CodeGen::start()
{
	generate(m_syntaxNodeRoot);

	m_dotText->push_back("\t\tnop\n");
	m_dotText->push_back("\t\tleave\n");
	m_dotText->push_back("\t\tret\n");

	m_dotText->push_back(".LC0:\n");
	m_dotText->push_back("\t\t.string \"%d\\n\"\n");

	dumpAsmToFile();
}

void CodeGen::generate(SyntaxNode *node)
{
	switch (node->getType())
	{
	case SyntaxNodeType::ASSIGN:
		genAssign(node);
		break;
	case SyntaxNodeType::IF:
		genIf(node);
		return;
	case SyntaxNodeType::WHILE:
		genWhile(node);
		return;
	case SyntaxNodeType::PRINT:
		genPrint(node);
		break;
	default:
		break;
	}

	auto iter = node->getChildren()->begin();
	for (; iter != node->getChildren()->end(); iter++)
		generate(*iter);
}

void CodeGen::genAssign(SyntaxNode *node)
{
	if (!haveInVarTable(node->getChildren()->at(0)))
	{
		if (node->getChildren()->at(0)->getType() == SyntaxNodeType::STRLITERAL)
		{
			m_dotData->push_back(node->getChildren()->at(0)->getToken()->lexema + ":\n");
			m_dotData->push_back("\t\t.string \"" + node->getChildren()->at(1)->getToken()->lexema + "\"\n");
			m_varTable->insert({ node->getChildren()->at(0)->getToken()->lexema, 
				std::to_string(node->getChildren()->at(1)->getToken()->lexema.size() - 2) });
		}
		else {
			m_countRBP += 4;
			m_varTable->insert({ node->getChildren()->at(0)->getToken()->lexema,
						"-" + std::to_string(m_countRBP) + "(%rbp)" });
		}
	}
	
	if (node->getChildren()->at(0)->getType() == SyntaxNodeType::NUMBER)
	{
		if (node->getChildren()->at(1)->getToken()->tokenClass == TokenClass::IntLiteral)
		{
			m_dotText->push_back("\t\tmovl    ");
			m_dotText->push_back("$" + node->getChildren()->at(1)->getToken()->lexema);
			m_dotText->push_back(", " + getAddrFromVarTable(node->getChildren()->at(0)) + "\n");
		} 
		else if (node->getChildren()->at(1)->getToken()->tokenClass == TokenClass::Identifier)
		{
			m_dotText->push_back("\t\tmovl    ");
			m_dotText->push_back(getAddrFromVarTable(node->getChildren()->at(1)));
			m_dotText->push_back(", " + getAddrFromVarTable(node->getChildren()->at(0)) + "\n");
		}
		else if (node->getChildren()->at(1)->getToken()->tokenClass == TokenClass::UnaryOperator)
		{
			genUnaryOperator(getAddrFromVarTable(node->getChildren()->at(0)), node->getChildren()->at(1));
		}
	}
}

void CodeGen::genUnaryOperator(std::string addres, SyntaxNode *oper)
{
	std::string operands[2];
	
	if (oper->getChildren()->at(0)->getToken()->tokenClass == TokenClass::UnaryOperator &&
		oper->getChildren()->at(1)->getToken()->tokenClass == TokenClass::UnaryOperator)
	{
		m_countRBP += 4;
		std::string buffAddres = "-" + std::to_string(m_countRBP) + "(%rbp)";
		genUnaryOperator(addres, oper->getChildren()->at(0));
		genUnaryOperator(buffAddres, oper->getChildren()->at(1));

		performUnaryOperator(addres, buffAddres, oper);
		m_countRBP -= 4;

		return;
	}

	for (size_t i = 0; i < 2; i++)
	{
		if (oper->getChildren()->at(i)->getToken()->tokenClass == TokenClass::Identifier)
		{
			operands[i] = getAddrFromVarTable(oper->getChildren()->at(i));
		}
		else {
			operands[i] = "$" + oper->getChildren()->at(i)->getToken()->lexema;
		}

		if (oper->getChildren()->at(i)->getToken()->tokenClass == TokenClass::UnaryOperator)
		{
			genUnaryOperator(addres, oper->getChildren()->at(i));

			performUnaryOperator(addres, operands[(i == 0) ? 1 : 0], oper);

			return;
		}
	}

	m_dotText->push_back("\t\tmovl    " + operands[0] + ", %eax\n");
	performUnaryOperator(addres, operands[1], oper);
}

void CodeGen::performUnaryOperator(std::string operand1, std::string operand2, SyntaxNode *oper)
{
	if (oper->getType() == SyntaxNodeType::MOD)
	{
		m_dotText->push_back("\t\tcltd\n");
		m_dotText->push_back("\t\tidivl    " + operand2 + "\n");
		m_dotText->push_back("\t\tmovl    %edx, " + operand1 + "\n");
	}
	else if (oper->getType() == SyntaxNodeType::ADD)
	{
		m_dotText->push_back("\t\taddl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tmovl    %eax, " + operand1 + "\n");
	}
	else if (oper->getType() == SyntaxNodeType::SUB)
	{
		m_dotText->push_back("\t\tsubl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tmovl    %eax, " + operand1 + "\n");
	}
	else if (oper->getType() == SyntaxNodeType::MUL)
	{
		m_dotText->push_back("\t\tmull    " + operand2 + "\n");
		m_dotText->push_back("\t\tmovl    %eax, " + operand1 + "\n");
	}
	else if (oper->getType() == SyntaxNodeType::DIV)
	{
		m_dotText->push_back("\t\tcltd\n");
		m_dotText->push_back("\t\tidivl    " + operand2 + "\n");
		m_dotText->push_back("\t\tmovl    %eax, " + operand1 + "\n");
	}
}

void CodeGen::genIf(SyntaxNode *node)
{
	std::string levelIf = "if" + std::to_string(node->getLevel());
	m_LC->insert({ levelIf + "_start", ".L" + std::to_string(m_countLC) });
	m_countLC++;

	if (node->getChildren()->size() == 3)
	{
		m_LC->insert({ levelIf + "_else", ".L" + std::to_string(m_countLC) });
		m_countLC++;
		m_LC->insert({ levelIf + "_end", ".L" + std::to_string(m_countLC) });
		m_countLC++;

		genLogicOperator(m_LC->find(levelIf + "_start")->second, 
			m_LC->find(levelIf + "_else")->second, 
			node->getChildren()->at(0)->getChildren()->at(0), false);

		m_dotText->push_back(m_LC->find(levelIf + "_start")->second + ":\n");
		generate(node->getChildren()->at(1));
		m_dotText->push_back("\t\tjmp    " + m_LC->find(levelIf + "_end")->second + "\n");

		m_dotText->push_back(m_LC->find(levelIf + "_else")->second + ":\n");
		generate(node->getChildren()->at(2));
		m_dotText->push_back("\t\tjmp    " + m_LC->find(levelIf + "_end")->second + "\n");

		m_dotText->push_back(m_LC->find(levelIf + "_end")->second + ":\n");

		m_LC->erase(levelIf + "_start");
		m_LC->erase(levelIf + "_else");
		m_LC->erase(levelIf + "_end");

		return;
	}

	m_LC->insert({ levelIf + "_end", ".L" + std::to_string(m_countLC) });
	m_countLC++;

	genLogicOperator(m_LC->find(levelIf + "_start")->second, 
		m_LC->find(levelIf + "_end")->second, 
		node->getChildren()->at(0)->getChildren()->at(0), false);

	m_dotText->push_back(m_LC->find(levelIf + "_start")->second + ":\n");
	generate(node->getChildren()->at(1));
	m_dotText->push_back("\t\tjmp    " + m_LC->find(levelIf + "_end")->second + "\n");

	m_dotText->push_back(m_LC->find(levelIf + "_end")->second + ":\n");

	m_LC->erase(levelIf + "_start");
	m_LC->erase(levelIf + "_end");
}

void CodeGen::genLogicOperator(std::string addresTrue, std::string addresFalse, SyntaxNode *oper, bool isWhile)
{
	std::string operands[2];

	if (oper->getChildren()->at(0)->getToken()->tokenClass == TokenClass::LogicOperator &&
		oper->getChildren()->at(1)->getToken()->tokenClass == TokenClass::LogicOperator)
	{
		if (oper->getType() == SyntaxNodeType::AND)
		{
			genLogicOperator(addresTrue, addresFalse, oper->getChildren()->at(0), isWhile);
			genLogicOperator(addresTrue, addresFalse, oper->getChildren()->at(1), isWhile);
			if (!isWhile)
				m_dotText->push_back("\t\tjmp    " + addresTrue + "\n");
		}
		else if (oper->getType() == SyntaxNodeType::OR)
		{
			invertLogicOperator(oper->getChildren()->at(0));
			invertLogicOperator(oper->getChildren()->at(1));
			genLogicOperator(addresFalse, addresTrue, oper->getChildren()->at(0), isWhile);
			genLogicOperator(addresFalse, addresTrue, oper->getChildren()->at(1), isWhile);
			if (!isWhile)
				m_dotText->push_back("\t\tjmp    " + addresFalse + "\n");
		}

		return;
	}

	bool isChar = false;
	for (size_t i = 0; i < 2; i++)
	{
		if (oper->getChildren()->at(i)->getToken()->tokenClass == TokenClass::Identifier)
		{
			if (oper->getChildren()->at(i)->getType() == SyntaxNodeType::STRLITERAL)
			{
				isChar = true;
				std::string addrIndex;
				if (oper->getChildren()->at(i)->getChildren()->at(0)->getToken()->tokenClass == TokenClass::Identifier)
				{
					addrIndex = getAddrFromVarTable(oper->getChildren()->at(i)->getChildren()->at(0));
				}
				else {
					addrIndex = "$" + oper->getChildren()->at(i)->getChildren()->at(0)->getToken()->lexema;
				}

				operands[i] = oper->getChildren()->at(i)->getToken()->lexema + "(," + addrIndex + ",1)";
			}
			else {
				operands[i] = getAddrFromVarTable(oper->getChildren()->at(i));
			}
		}
		else {
			operands[i] = "$" + oper->getChildren()->at(i)->getToken()->lexema;
		}

		if (oper->getChildren()->at(i)->getToken()->tokenClass == TokenClass::LogicOperator)
		{
			return;
		}
	}

	invertLogicOperator(oper);

	if (isChar)
	{
		performLogicOperatorChar(operands[0], operands[1], oper);
	}
	else {
		performLogicOperator(operands[0], operands[1], oper);
	}
	m_dotText->push_back(addresFalse + "\n");
	if (!isWhile)
		m_dotText->push_back("\t\tjmp    " + addresTrue + "\n");
}

void CodeGen::performLogicOperator(std::string operand1, std::string operand2, SyntaxNode *oper)
{
	if (oper->getType() == SyntaxNodeType::EQUAL)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tje    ");
	}
	else if (oper->getType() == SyntaxNodeType::NOTEQUAL)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tjne    ");
	}
	else if (oper->getType() == SyntaxNodeType::MORE)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tja    ");
	}
	else if (oper->getType() == SyntaxNodeType::MOREEQUAL)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tjae    ");
	}
	else if (oper->getType() == SyntaxNodeType::LESS)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tjb    ");
	}
	else if (oper->getType() == SyntaxNodeType::LESSEQUAL)
	{
		m_dotText->push_back("\t\tmovl    " + operand1 + ", %eax\n");
		m_dotText->push_back("\t\tcmpl    " + operand2 + ", %eax\n");
		m_dotText->push_back("\t\tjbe    ");
	}
}

void CodeGen::performLogicOperatorChar(std::string operand1, std::string operand2, SyntaxNode *oper)
{
	m_dotText->push_back("\t\tmovb    " + operand1 + ", %ah\n");
	m_dotText->push_back("\t\tmovb    " + operand2 + ", %dh\n");
	m_dotText->push_back("\t\tcmpb    %ah, %dh\n");

	if (oper->getType() == SyntaxNodeType::EQUAL)
	{
		m_dotText->push_back("\t\tje    ");
	}
	else if (oper->getType() == SyntaxNodeType::NOTEQUAL)
	{
		m_dotText->push_back("\t\tjne    ");
	}
	else if (oper->getType() == SyntaxNodeType::MORE)
	{
		m_dotText->push_back("\t\tja    ");
	}
	else if (oper->getType() == SyntaxNodeType::MOREEQUAL)
	{
		m_dotText->push_back("\t\tjae    ");
	}
	else if (oper->getType() == SyntaxNodeType::LESS)
	{
		m_dotText->push_back("\t\tjb    ");
	}
	else if (oper->getType() == SyntaxNodeType::LESSEQUAL)
	{
		m_dotText->push_back("\t\tjbe    ");
	}
}

void CodeGen::invertLogicOperator(SyntaxNode *node)
{
	switch (node->getType())
	{
	case SyntaxNodeType::EQUAL:
		node->setType(SyntaxNodeType::NOTEQUAL);
		break;
	case SyntaxNodeType::NOTEQUAL:
		node->setType(SyntaxNodeType::EQUAL);
		break;
	case SyntaxNodeType::MORE:
		node->setType(SyntaxNodeType::LESSEQUAL);
		break;
	case SyntaxNodeType::MOREEQUAL:
		node->setType(SyntaxNodeType::LESS);
		break;
	case SyntaxNodeType::LESS:
		node->setType(SyntaxNodeType::MOREEQUAL);
		break;
	case SyntaxNodeType::LESSEQUAL:
		node->setType(SyntaxNodeType::MORE);
		break;
	default:
		break;
	}
}

void CodeGen::genPrint(SyntaxNode *node)
{
	if (node->getChildren()->at(0)->getType() == SyntaxNodeType::NUMBER)
	{
		if (haveInVarTable(node->getChildren()->at(0)))
		{
			m_dotText->push_back("\t\tmovl    " + getAddrFromVarTable(node->getChildren()->at(0)) + ", %esi\n");
		}
		else {
			m_dotText->push_back("\t\tmovl    %edx, %esi\n");
		}
		m_dotText->push_back("\t\tmovl    $.LC0, %edi\n");
		m_dotText->push_back("\t\tmovl    $0, %eax\n");
		m_dotText->push_back("\t\tcall    printf\n");
	} 
	else if (node->getChildren()->at(0)->getToken()->tokenClass == TokenClass::UnaryOperator)
	{
		genUnaryOperator("%edx", node->getChildren()->at(0));

		m_dotText->push_back("\t\tmovl    %edx, %esi\n");
		m_dotText->push_back("\t\tmovl    $.LC0, %edi\n");
		m_dotText->push_back("\t\tmovl    $0, %eax\n");
		m_dotText->push_back("\t\tcall    printf\n");
	}
}

void CodeGen::genWhile(SyntaxNode *node)
{
	std::string levelWhile = "while" + std::to_string(node->getLevel());
	m_LC->insert({ levelWhile + "_start", ".L" + std::to_string(m_countLC) });
	m_countLC++;
	m_LC->insert({ levelWhile + "_end", ".L" + std::to_string(m_countLC) });
	m_countLC++;

	m_dotText->push_back(m_LC->find(levelWhile + "_start")->second + ":\n");

	genLogicOperator(m_LC->find(levelWhile + "_start")->second, 
		m_LC->find(levelWhile + "_end")->second, 
		node->getChildren()->at(0)->getChildren()->at(0), true);

	generate(node->getChildren()->at(1));
	m_dotText->push_back("\t\tjmp    " + m_LC->find(levelWhile + "_start")->second + "\n");

	m_dotText->push_back(m_LC->find(levelWhile + "_end")->second + ":\n");

	m_LC->erase(levelWhile + "_start");
	m_LC->erase(levelWhile + "_end");
}

void CodeGen::dumpAsm()
{
	auto iter = m_dotData->begin();
	for (; iter != m_dotData->end(); iter++)
	{
		std::cout << *iter;
	}

	auto iter2 = m_dotText->begin();
	for (; iter2 != m_dotText->end(); iter2++)
	{
		std::cout << *iter2;
	}
}

void CodeGen::dumpAsmToFile()
{
	std::ofstream outf("assembler.s");

	if (!outf)
	{
		std::cerr << "Uh oh, assembler.s could not be opened for writing!" << std::endl;
		exit(1);
	}

	auto iter = m_dotData->begin();
	for (; iter != m_dotData->end(); iter++)
	{
		outf << *iter;
	}

	auto iter2 = m_dotText->begin();
	for (; iter2 != m_dotText->end(); iter2++)
	{
		outf << *iter2;
	}
}