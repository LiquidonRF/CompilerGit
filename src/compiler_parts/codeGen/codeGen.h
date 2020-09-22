#pragma once
#include "../parser/parser.h"
#include <map>

class Register
{
private:
	std::string m_name;
	bool m_value;

public:
	Register(std::string newString, bool newValue)
	{
		m_name = newString;
		m_value = newValue;
	}

	std::string getName() { return m_name; }
	bool isValue() { return m_value; }
	void setName(std::string newName) { m_name = newName; }
	void setValue(bool newValue) { m_value = newValue; }
};

class CodeGen
{
private:
	SyntaxNode *m_syntaxNodeRoot;
	std::list<Register*> *m_registers = new std::list<Register*>();
	std::vector<std::string> *m_dotData = new std::vector<std::string>();
	std::vector<std::string> *m_dotText = new std::vector<std::string>();

	std::unordered_map<std::string, std::string> *m_varTable = new std::unordered_map<std::string, std::string>();
	std::unordered_map<std::string, std::string> *m_LC = new std::unordered_map<std::string, std::string>();

	size_t m_varsInStack = 0;
	size_t m_countLC = 0;
	size_t m_countRBP = 0;

	bool haveInVarTable(SyntaxNode *node);
	std::string getAddrFromVarTable(SyntaxNode *node);
	void genAssign(SyntaxNode *node);
	void genUnaryOperator(std::string addres, SyntaxNode *oper);
	void performUnaryOperator(std::string operand1, std::string operand2, SyntaxNode *oper);
	void genIf(SyntaxNode *node);
	void genLogicOperator(std::string addresTrue, std::string addresFalse, SyntaxNode *oper, bool isWhile);
	void performLogicOperator(std::string operand1, std::string operand2, SyntaxNode *oper);
	void invertLogicOperator(SyntaxNode *node);
	void genPrint(SyntaxNode *node);
	void genWhile(SyntaxNode *node);

public:
	std::vector<std::string> *getDotData() { return m_dotData; }
	std::vector<std::string> *getDotText() { return m_dotText; }

	void init(SyntaxNode* root);
	void start();
	void generate(SyntaxNode *node);
	void dumpAsm();
};