#pragma once
#include <iostream>
#include <unordered_map>
#include "../lexer/lexer.h"
#include <cstddef>
#include <algorithm>
#include <vector>
#include <vector>

enum class SyntaxNodeType {
	PROGRAM, NUMBER, FLOAT, ID, ASSIGN, IF, ELSE,
	DO, FOR, WHILE, INPUT, DEF, STRLITERAL,
	UNKNOWN, EXPRESSION, OPERATOR, STATEMENT,
	PRINT, ARG, IN, RETURN, NOTHING, LOGIC,
	ADD, SUB, MUL, DIV, MOD, EQUAL, NOTEQUAL,
	MOREEQUAL, LESSEQUAL, MORE, LESS, AND, OR,
	ARRAY, INDEX, INTtoFLOAT, EndOfFile
};

class SyntaxNode
{
public:
	SyntaxNodeType getType();
	std::string getTypeString();
	void setType(SyntaxNodeType newType);
	Token *getToken();
	void setToken(Token* newToken);
	size_t getLevel();
	void setLevel(size_t newLevel);
	SyntaxNode *getParent();
	std::vector<SyntaxNode*>* getChildren();
	void addChild(SyntaxNode* child);
	SyntaxNode *getChild(SyntaxNode *child);
	void removeChild(SyntaxNode *child);
	size_t lookupChildIndex(SyntaxNode* child);

	SyntaxNode()
	{
		m_type = SyntaxNodeType::UNKNOWN;
		m_token = NULL;
		m_level = 0;
		m_parent = NULL;
		m_children = new std::vector<SyntaxNode*>();
	}

	SyntaxNode(SyntaxNodeType newType)
	{
		m_type = newType;
		m_token = NULL;
		m_level = 0;
		m_parent = NULL;
		m_children = new std::vector<SyntaxNode*>();
	}

	SyntaxNode(SyntaxNodeType newType, Token *newToken)
	{
		m_type = newType;
		m_token = newToken;
		m_level = 0;
		m_parent = NULL;
		m_children = new std::vector<SyntaxNode*>();
	}

	SyntaxNode(SyntaxNodeType newType, Token *newToken, size_t newLevel)
	{
		m_type = newType;
		m_token = newToken;
		m_level = newLevel;
		m_parent = NULL;
		m_children = new std::vector<SyntaxNode*>();
	}

	SyntaxNode(SyntaxNodeType newType, Token* newToken, size_t newLevel, SyntaxNode *newParent)
	{
		m_type = newType;
		m_token = newToken;
		m_level = newLevel;
		m_parent = newParent;
		m_children = new std::vector<SyntaxNode*>();
	}

private:
	SyntaxNodeType m_type;
	Token *m_token;
	size_t m_level;
	SyntaxNode *m_parent;
	std::vector<SyntaxNode*> *m_children;
};

class Parser
{
public:
	Parser(Lexer *lexer)
	{
		m_IdentifierTable = new std::unordered_map<std::string, SyntaxNode*>();
		m_currToken = lexer->getCurrToken();
		m_lexer = lexer;
	}
	void start();
	void showTree();
	SyntaxNode *getSyntaxTreeRoot();
	
private:
	void nextToken();
	void prevToken();

	std::unordered_map<std::string, SyntaxNode*> *m_IdentifierTable = NULL;
	Lexer *m_lexer = NULL;
	SyntaxNode* m_syntaxTreeRoot = NULL;
	Token *m_currToken = NULL;
	size_t m_currTokenIndex = 0;
	size_t m_currLevel = 0;

	void showTreeNode(SyntaxNode* node, size_t depth);
	void checkTokenType(TokenClass);
	bool haveIdentInIdentTable(std::string);
	SyntaxNodeType getIdentTypeFromIdentTable(std::string);
	void checkForParseOperator(TokenClass statementClass, SyntaxNodeType statementType);

	SyntaxNode *parseMain();
	SyntaxNode *parseFor();
	SyntaxNode *parseWhile();
	SyntaxNode *parseIf();
	SyntaxNode *parseExpr();
	SyntaxNode *parseLogicOperator(SyntaxNode*, SyntaxNode*, TokenClass, SyntaxNodeType);
	SyntaxNode *parseStatement(SyntaxNode *);
	SyntaxNode *parseOperator(SyntaxNode *,SyntaxNode *, TokenClass, SyntaxNodeType);
	SyntaxNode *parseIdentifier();
	SyntaxNode *parseAssignment(SyntaxNode *);
	SyntaxNode *parseArray(SyntaxNode *);
};