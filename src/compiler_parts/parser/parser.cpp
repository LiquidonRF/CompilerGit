#include "Parser.h"

SyntaxNodeType SyntaxNode::getType()
{
	return m_type;
}

void SyntaxNode::setType(SyntaxNodeType newType)
{
	m_type = newType;
}

Token* SyntaxNode::getToken()
{
	return m_token;
}

void SyntaxNode::setToken(Token* newToken)
{
	m_token = newToken;
}

size_t SyntaxNode::getLevel()
{
	return m_level;
}

SyntaxNode* SyntaxNode::getParent()
{
	return m_parent;
}

std::vector<SyntaxNode*>* SyntaxNode::getChildren()
{
	return m_children;
}

SyntaxNode *SyntaxNode::getChild(SyntaxNode* child)
{
	size_t index = lookupChildIndex(child);

	return m_children->at(index);
}

void SyntaxNode::removeChild(SyntaxNode *child)
{
	size_t index = lookupChildIndex(child);

	m_children->erase(m_children->begin() + index);
}

void SyntaxNode::setLevel(size_t newLevel)
{
	m_level = newLevel;
}

void SyntaxNode::addChild(SyntaxNode* child)
{
	m_children->push_back(child);
}

size_t SyntaxNode::lookupChildIndex(SyntaxNode* child)
{
	size_t counter = 0;
	for (auto iter = m_children->begin(); iter != m_children->end(); iter++)
	{
		if (child == *iter)
			return counter;
		counter++;
	}

	return -1;
}

void Parser::nextToken()
{
	m_currToken = m_lexer->getNextToken();
	m_currTokenIndex++;

	if (m_currToken->tokenClass == TokenClass::KeyWordDo)
		m_currLevel++;
	if (m_currToken->tokenClass == TokenClass::KeyWordEnd)
		m_currLevel--;
}

void Parser::prevToken()
{
	m_currToken = m_lexer->getPrevToken();
	m_currTokenIndex--;
}

void Parser::checkTokenType(TokenClass type)
{
	bool haveError;
	if (m_currToken->tokenClass != type)
	{
		haveError = true;
		std::cout << "\tError in " << m_currToken->line << ',' << m_currToken->posInLine 
			<< ". Expecting " << m_lexer->getTokenTypeString(type) << ", but found " 
			<< m_lexer->getTokenTypeString(m_currToken->tokenClass) << std::endl;

		assert(!haveError);
	}
}

bool Parser::haveIdentInIdentTable(std::string lexem)
{
	if (m_IdentifierTable->find(lexem) == m_IdentifierTable->end())
		return false;

	return true;
}

SyntaxNodeType Parser::getIdentTypeFromIdentTable(std::string lexem)
{
	auto search = m_IdentifierTable->find(lexem);
	if (search == m_IdentifierTable->end())
		return SyntaxNodeType::UNKNOWN;

	return search->second->getType();
}

void Parser::start()
{
	m_syntaxTreeRoot = new SyntaxNode(SyntaxNodeType::PROGRAM, new Token, 0);

	m_currLevel++;

	while (m_currToken->tokenClass != TokenClass::EndOfFile)
	{
		SyntaxNode* tmp = parseMain();
		if(tmp != NULL)
			m_syntaxTreeRoot->addChild(tmp);
	}
}

SyntaxNode *Parser::parseMain()
{
	SyntaxNode* node = NULL;
	SyntaxNode* tmp = NULL;

	switch (m_currToken->tokenClass)
	{
	case TokenClass::KeyWordFor:
		node = parseFor();
		break;
	case TokenClass::KeyWordWhile:
		node = parseWhile();
		break;
	case TokenClass::KeyWordIf:
		node = parseIf();
		break;
	case TokenClass::Identifier:
		node = parseIdentifier();
		break;
	case TokenClass::Input:
		break;
	case TokenClass::Output:
		node = new SyntaxNode(SyntaxNodeType::PRINT, m_currToken, m_currLevel);
		nextToken();
		node->addChild(parseStatement(node));
		node->setType(SyntaxNodeType::PRINT);
		break;
	case TokenClass::Separator:
		nextToken();
		node = parseMain();
		break;
	default:
		break;
	}

	return node;
}

SyntaxNode *Parser::parseFor()
{
	SyntaxNode *node = new SyntaxNode(SyntaxNodeType::FOR, m_currToken, m_currLevel);

	nextToken();

	node->addChild(new SyntaxNode(SyntaxNodeType::ID, m_currToken, m_currLevel));
	checkTokenType(TokenClass::Identifier);
	nextToken();

	node->addChild(new SyntaxNode(SyntaxNodeType::IN, m_currToken, m_currLevel));
	checkTokenType(TokenClass::KeyWordIn);
	nextToken();

	if (m_currToken->tokenClass == TokenClass::Identifier)//for ID in ID do
	{
		node->addChild(new SyntaxNode(SyntaxNodeType::ID, m_currToken, m_currLevel));
		nextToken();
		checkTokenType(TokenClass::KeyWordDo);
		nextToken();
		node->addChild(new SyntaxNode(SyntaxNodeType::DO, m_currToken, m_currLevel - 1));
		checkTokenType(TokenClass::Separator);
		nextToken();
	}
	else if (m_currToken->tokenClass == TokenClass::IntLiteral)//for ID in num..num do
	{
		node->addChild(new SyntaxNode(SyntaxNodeType::NUMBER, m_currToken, m_currLevel));
		nextToken();
		checkTokenType(TokenClass::TwoPoints);
		nextToken();
		checkTokenType(TokenClass::IntLiteral);
		node->addChild(new SyntaxNode(SyntaxNodeType::NUMBER, m_currToken, m_currLevel));
		nextToken();
		checkTokenType(TokenClass::KeyWordDo);
		node->addChild(new SyntaxNode(SyntaxNodeType::DO, m_currToken, m_currLevel - 1));
		nextToken();
		checkTokenType(TokenClass::Separator);
		nextToken();
	}
	else {
		bool detectIdOrNum = false;
		assert(detectIdOrNum);
	}

	SyntaxNode *tmp = parseMain();
	node->getChildren()->back()->addChild(parseMain());

	if (m_currToken->tokenClass == TokenClass::KeyWordElse)
	{
		node->addChild(new SyntaxNode(SyntaxNodeType::ELSE, m_currToken, m_currLevel));
		tmp = parseMain();
	}
	
	checkTokenType(TokenClass::KeyWordEnd);
	nextToken();

	return node;
}

SyntaxNode *Parser::parseWhile()
{
	SyntaxNode *node = new SyntaxNode(SyntaxNodeType::WHILE, m_currToken, m_currLevel);

	node->addChild(new SyntaxNode(SyntaxNodeType::EXPRESSION));
	nextToken();
	node->getChildren()->back()->addChild(parseExpr());
	
	checkTokenType(TokenClass::KeyWordDo);
	node->addChild(new SyntaxNode(SyntaxNodeType::DO, m_currToken, m_currLevel - 1));
	nextToken();

	SyntaxNode *tmp = parseMain();
	node->getChildren()->back()->addChild(tmp);

	checkTokenType(TokenClass::KeyWordEnd);
	nextToken();

	return node;
}

SyntaxNode *Parser::parseIf()
{
	SyntaxNode* node = new SyntaxNode(SyntaxNodeType::IF, m_currToken, m_currLevel);

	node->addChild(new SyntaxNode(SyntaxNodeType::EXPRESSION));
	nextToken();
	node->getChildren()->back()->addChild(parseExpr());

	checkTokenType(TokenClass::KeyWordDo);
	node->addChild(new SyntaxNode(SyntaxNodeType::DO, m_currToken, m_currLevel - 1));
	nextToken();

	SyntaxNode* tmp = parseMain();
	node->getChildren()->back()->addChild(tmp);

	if (m_currToken->tokenClass == TokenClass::KeyWordElse)
	{
		node->addChild(new SyntaxNode(SyntaxNodeType::ELSE, m_currToken, m_currLevel - 1));
		nextToken();

		tmp = parseMain();
		node->getChildren()->back()->addChild(tmp);
	}

	checkTokenType(TokenClass::KeyWordEnd);
	nextToken();
	checkTokenType(TokenClass::Separator);
	nextToken();

	return node;
}

SyntaxNode *Parser::parseExpr()
{
	SyntaxNode* node = NULL;
	SyntaxNode* statePart = NULL;
	SyntaxNode* localRoot = new SyntaxNode();
	TokenClass statementClass = TokenClass::Unknown;
	SyntaxNodeType statementType = SyntaxNodeType::UNKNOWN;
	SyntaxNodeType identType = SyntaxNodeType::UNKNOWN;

	switch (m_currToken->tokenClass)
	{
	case TokenClass::IntLiteral:
		statementClass = TokenClass::IntLiteral;
		statementType = SyntaxNodeType::NUMBER;
		statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
		node = parseLogicOperator(localRoot, statePart, statementClass, statementType);
		break;
	case TokenClass::FloatLiteral:
		statementClass = TokenClass::FloatLiteral;
		statementType = SyntaxNodeType::FLOAT;
		statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
		node = parseLogicOperator(localRoot, statePart, statementClass, statementType);
		break;
	case TokenClass::Identifier:
		if (!haveIdentInIdentTable(m_currToken->lexema))
			assert(false && "Not initialized identifier in array");

		identType = getIdentTypeFromIdentTable(m_currToken->lexema);

		if (identType == SyntaxNodeType::NUMBER)
		{
			statementClass = TokenClass::IntLiteral;
			statementType = SyntaxNodeType::NUMBER;
			statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
			node = parseLogicOperator(localRoot, statePart, statementClass, statementType);
		}
		else if (identType == SyntaxNodeType::FLOAT)
		{
			statementClass = TokenClass::FloatLiteral;
			statementType = SyntaxNodeType::FLOAT;
			statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
			node = parseLogicOperator(localRoot, statePart, statementClass, statementType);
		}
		break;
	default:
		assert(false && "Expected int, float, ident in statement");
		break;
	}

	return node;
}

SyntaxNode *Parser::parseLogicOperator(SyntaxNode* localRoot, SyntaxNode* identifier, TokenClass statementClass, SyntaxNodeType statementType)
{
	SyntaxNodeType operatorType = SyntaxNodeType::UNKNOWN;

	nextToken();

	if (m_currToken->tokenClass == TokenClass::LogicOperator)
	{
		if (m_currToken->lexema == "==")
			operatorType = SyntaxNodeType::EQUAL;
		else if (m_currToken->lexema == "!=")
			operatorType = SyntaxNodeType::NOTEQUAL;
		else if (m_currToken->lexema == ">=")
			operatorType = SyntaxNodeType::MOREEQUAL;
		else if (m_currToken->lexema == "<=")
			operatorType = SyntaxNodeType::LESSEQUAL;
		else if (m_currToken->lexema == ">")
			operatorType = SyntaxNodeType::MORE;
		else if (m_currToken->lexema == "<")
			operatorType = SyntaxNodeType::LESS;
		else if (m_currToken->lexema == "&&")
			operatorType = SyntaxNodeType::AND;
		else
			operatorType = SyntaxNodeType::OR;

		if (localRoot->getType() == SyntaxNodeType::UNKNOWN)
		{
			localRoot->setType(operatorType);
			localRoot->setToken(m_currToken);
			localRoot->setLevel(m_currLevel);
			localRoot->addChild(identifier);

			nextToken();
			checkForParseOperator(statementClass, statementType);

			localRoot->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
			SyntaxNode *tmp = parseLogicOperator(localRoot, localRoot->getChildren()->back(), statementClass, statementType);
			if (tmp != NULL)
				localRoot = tmp;

			return localRoot;
		}
		else {
			if (operatorType == SyntaxNodeType::AND ||
				operatorType == SyntaxNodeType::OR)
			{
				SyntaxNode* node = new SyntaxNode(operatorType, m_currToken, m_currLevel);
				node->addChild(localRoot);
				localRoot = node;

				nextToken();
				checkForParseOperator(statementClass, statementType);

				localRoot->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
				SyntaxNode* tmp = parseLogicOperator(localRoot, localRoot->getChildren()->back(), statementClass, statementType);
				if (tmp != NULL)
					localRoot = tmp;

				return localRoot;
			}
			else {
				SyntaxNode* tmp = localRoot->getChildren()->back();
				localRoot->removeChild(localRoot->getChildren()->back());
				localRoot->addChild(new SyntaxNode(operatorType, m_currToken, m_currLevel));
				localRoot->getChildren()->back()->addChild(tmp);

				nextToken();
				checkForParseOperator(statementClass, statementType);

				localRoot->getChildren()->back()->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
				tmp = parseLogicOperator(localRoot, localRoot->getChildren()->back()->getChildren()->back(), statementClass, statementType);
				if (tmp != NULL)
					localRoot = tmp;

				return localRoot;
			}
		}
	}

	checkTokenType(TokenClass::KeyWordDo);

	return NULL;
}

SyntaxNode *Parser::parseStatement(SyntaxNode *Identifier)
{
	SyntaxNode *node = NULL;
	SyntaxNode *statePart = NULL;
	SyntaxNode *localRoot = new SyntaxNode();
	TokenClass statementClass = TokenClass::Unknown;
	SyntaxNodeType statementType = SyntaxNodeType::UNKNOWN;
	SyntaxNodeType identType = SyntaxNodeType::UNKNOWN;

	switch (m_currToken->tokenClass)
	{
	case TokenClass::IntLiteral:
		statementClass = TokenClass::IntLiteral;
		statementType = SyntaxNodeType::NUMBER;
		statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
		node = parseOperator(localRoot, statePart, statementClass, statementType);
		break;
	case TokenClass::FloatLiteral:
		statementClass = TokenClass::FloatLiteral;
		statementType = SyntaxNodeType::FLOAT;
		statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
		node = parseOperator(localRoot, statePart, statementClass, statementType);
		break;
	case TokenClass::Identifier:
		if (!haveIdentInIdentTable(m_currToken->lexema))
			assert(false && "Not initialized identifier in array");

		identType = getIdentTypeFromIdentTable(m_currToken->lexema);

		if (identType == SyntaxNodeType::NUMBER)
		{
			statementClass = TokenClass::IntLiteral;
			statementType = SyntaxNodeType::NUMBER;
		}
		else if (identType == SyntaxNodeType::FLOAT) 
		{
			statementClass = TokenClass::FloatLiteral;
			statementType = SyntaxNodeType::FLOAT;
		}
		else if (identType == SyntaxNodeType::ARRAY) 
		{
			statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);

			nextToken();
			checkTokenType(TokenClass::LSquareBracket);

			nextToken();
			checkForParseOperator(TokenClass::IntLiteral, SyntaxNodeType::NUMBER);
			statePart->addChild(new SyntaxNode(SyntaxNodeType::INDEX, m_currToken, m_currLevel));

			if (!haveIdentInIdentTable(statePart->getToken()->lexema + "[" + m_currToken->lexema + "]"))
				assert(false && "Not initialized identifier in array");

			identType = getIdentTypeFromIdentTable(statePart->getToken()->lexema + "[" + m_currToken->lexema + "]");
			if (identType == SyntaxNodeType::NUMBER)
			{
				statementClass = TokenClass::IntLiteral;
				statementType = SyntaxNodeType::NUMBER;
			}
			else if (identType == SyntaxNodeType::FLOAT)
			{
				statementClass = TokenClass::FloatLiteral;
				statementType = SyntaxNodeType::FLOAT;
			}
			statePart->setType(statementType);

			nextToken();
			checkTokenType(TokenClass::RSquareBracket);
			
			node = parseOperator(localRoot, statePart, statementClass, statementType);
			break;
		}
		statePart = new SyntaxNode(statementType, m_currToken, m_currLevel);
		node = parseOperator(localRoot, statePart, statementClass, statementType);

		break;
	default:
		assert(false && "Expected int, float, ident in statement");
		break;
	}

	Identifier->setType(statementType);

	if (node == NULL)
		node = statePart;

	return node;
}

void Parser::checkForParseOperator(TokenClass statementClass, SyntaxNodeType statementType)
{
	if (m_currToken->tokenClass == TokenClass::Identifier)
	{
		if (!haveIdentInIdentTable(m_currToken->lexema))
			assert(false && "Not initialized identifier");

		if (getIdentTypeFromIdentTable(m_currToken->lexema) != statementType)
			assert(false && "Invalid identifier type");
	}
	else {
		checkTokenType(statementClass);
	}
}

SyntaxNode *Parser::parseOperator(SyntaxNode *localRoot, SyntaxNode *identifier, TokenClass statementClass, SyntaxNodeType statementType)
{
	SyntaxNodeType operatorType = SyntaxNodeType::UNKNOWN;
	
	nextToken();

	if (m_currToken->tokenClass == TokenClass::UnaryOperator)
	{
		if (m_currToken->lexema == "+")
			operatorType = SyntaxNodeType::ADD;
		else if (m_currToken->lexema == "-")
			operatorType = SyntaxNodeType::SUB;
		else if (m_currToken->lexema == "*")
			operatorType = SyntaxNodeType::MUL;
		else if (m_currToken->lexema == "/")
			operatorType = SyntaxNodeType::DIV;
		else if (m_currToken->lexema == "%")
			operatorType = SyntaxNodeType::MOD;

		if (localRoot->getType() == SyntaxNodeType::UNKNOWN)
		{
			localRoot->setType(operatorType);
			localRoot->setToken(m_currToken);
			localRoot->setLevel(m_currLevel);
			localRoot->addChild(identifier);

			nextToken();
			checkForParseOperator(statementClass, statementType);

			localRoot->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
			SyntaxNode *tmp = parseOperator(localRoot, localRoot->getChildren()->back(), statementClass, statementType);
			if (tmp != NULL)
				localRoot = tmp;

			return localRoot;
		}
		else {
			if (operatorType == SyntaxNodeType::ADD || 
				operatorType == SyntaxNodeType::SUB)
			{
				SyntaxNode *node = new SyntaxNode(operatorType, m_currToken, m_currLevel);
				node->addChild(localRoot);
				localRoot = node;

				nextToken();
				checkForParseOperator(statementClass, statementType);

				localRoot->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
				SyntaxNode *tmp = parseOperator(localRoot, localRoot->getChildren()->back(), statementClass, statementType);
				if (tmp != NULL)
					localRoot = tmp;

				return localRoot;
			}
			else if (operatorType == SyntaxNodeType::MUL || 
				operatorType == SyntaxNodeType::DIV || 
				operatorType == SyntaxNodeType::MOD) 
			{
				SyntaxNode* tmp = localRoot->getChildren()->back();
				localRoot->removeChild(localRoot->getChildren()->back());
				localRoot->addChild(new SyntaxNode(operatorType, m_currToken, m_currLevel));
				localRoot->getChildren()->back()->addChild(tmp);

				nextToken();
				checkForParseOperator(statementClass, statementType);

				localRoot->getChildren()->back()->addChild(new SyntaxNode(statementType, m_currToken, m_currLevel));
				tmp = parseOperator(localRoot, localRoot->getChildren()->back()->getChildren()->back(), statementClass, statementType);
				if (tmp != NULL)
					localRoot = tmp;

				return localRoot;
			}
		}
	}
	else if (m_currToken->tokenClass == TokenClass::LSquareBracket) 
	{
		nextToken();
		checkForParseOperator(TokenClass::IntLiteral, SyntaxNodeType::NUMBER);
		SyntaxNodeType search = getIdentTypeFromIdentTable(m_currToken->lexema);
		
		if (search == SyntaxNodeType::ARRAY || search == SyntaxNodeType::STRLITERAL)
		{
			identifier->addChild(new SyntaxNode(SyntaxNodeType::INDEX, m_currToken, m_currLevel));
			nextToken();
			checkTokenType(TokenClass::RSquareBracket);

			SyntaxNode* tmp = parseOperator(localRoot, identifier, statementClass, statementType);
			if (tmp != NULL)
				localRoot = tmp;

			return localRoot;
		} else {
			assert(false && "Get by index in not array or string");
		}
	}

	if (m_currToken->tokenClass == TokenClass::EndOfFile)
		return NULL;

	checkTokenType(TokenClass::Separator);

	return NULL;
}

SyntaxNode *Parser::parseIdentifier()
{
	SyntaxNode *node = new SyntaxNode(SyntaxNodeType::ID, m_currToken, m_currLevel);

	nextToken();

	SyntaxNode *assign = NULL;
	SyntaxNodeType search = SyntaxNodeType::UNKNOWN;

	switch (m_currToken->tokenClass)
	{
	case TokenClass::Assignment:
		assign = parseAssignment(node);
		break;
	case TokenClass::LSquareBracket:
		nextToken();
		checkForParseOperator(TokenClass::IntLiteral, SyntaxNodeType::NUMBER);
		search = getIdentTypeFromIdentTable(m_currToken->lexema);
		
		if (search == SyntaxNodeType::ARRAY || search == SyntaxNodeType::STRLITERAL)
		{
			node->addChild(new SyntaxNode(SyntaxNodeType::INDEX, m_currToken, m_currLevel));
			nextToken();
			checkTokenType(TokenClass::RSquareBracket);

			assign = parseAssignment(node);
		} else {
			assert(false && "Get by index in not array or string");
		}
		break;
	default:
		assert(false && "Invalid init identifier");
		break;
	}

	return assign;
}
SyntaxNode *Parser::parseAssignment(SyntaxNode *Identifier)
{
	SyntaxNode* node = new SyntaxNode(SyntaxNodeType::ASSIGN, m_currToken, m_currLevel);

	nextToken();

	switch (m_currToken->tokenClass)
	{
	case TokenClass::LSquareBracket:
		delete(node);
		return parseArray(Identifier);
	case TokenClass::String:
		Identifier->setType(SyntaxNodeType::STRLITERAL);
		node->addChild(Identifier);
		node->addChild(new SyntaxNode(SyntaxNodeType::STRLITERAL, m_currToken, m_currLevel, node));
		nextToken();
		checkTokenType(TokenClass::Separator);

		m_IdentifierTable->insert({ Identifier->getToken()->lexema, Identifier });

		nextToken();
		break;
	default:
		node->addChild(Identifier);
		node->addChild(parseStatement(Identifier));

		if (!haveIdentInIdentTable(Identifier->getToken()->lexema))
			m_IdentifierTable->insert({ Identifier->getToken()->lexema, Identifier });

		nextToken();
		break;
	}

	return node;
}

SyntaxNode *Parser::parseArray(SyntaxNode *Identifier)
{
	Identifier->setType(SyntaxNodeType::ARRAY);

	m_IdentifierTable->insert({ Identifier->getToken()->lexema, Identifier });

	nextToken();

	TokenClass arrayType = TokenClass::Unknown;
	SyntaxNodeType nodeType = SyntaxNodeType::UNKNOWN;
	size_t iter = 0;

	if (m_currToken->tokenClass == TokenClass::IntLiteral)
	{
		arrayType = TokenClass::IntLiteral;
		nodeType = SyntaxNodeType::NUMBER;
	}
	else if (m_currToken->tokenClass == TokenClass::FloatLiteral)
	{
		arrayType = TokenClass::FloatLiteral;
		nodeType = SyntaxNodeType::FLOAT;
	} 
	else if (m_currToken->tokenClass == TokenClass::Identifier) 
	{
		if (!haveIdentInIdentTable(m_currToken->lexema))
			assert(false && "not initialized identifier in array");

		auto search = m_IdentifierTable->find(m_currToken->lexema);

		if (search->second->getType() == SyntaxNodeType::NUMBER)
		{
			arrayType = TokenClass::IntLiteral;
			nodeType = SyntaxNodeType::NUMBER;
		}
		else if (search->second->getType() == SyntaxNodeType::FLOAT) 
		{
			arrayType = TokenClass::FloatLiteral;
			nodeType = SyntaxNodeType::FLOAT;
		}
		else {
			assert(false && "Invalid type identifier in array");
		}
	}
	else {
		assert(false && "Expected int or float");
	}
	Identifier->addChild(new SyntaxNode(nodeType, m_currToken, m_currLevel));
	nextToken();
	m_IdentifierTable->insert({ Identifier->getToken()->lexema + '[' + std::to_string(iter) + ']',
	Identifier->getChildren()->back() });

	while (m_currToken->tokenClass != TokenClass::RSquareBracket)
	{
		iter++;
		checkTokenType(TokenClass::Comma);
		nextToken();
		if (m_currToken->tokenClass == TokenClass::Identifier)
		{
			if (!haveIdentInIdentTable(m_currToken->lexema))
				assert(false && "not initialized identifier in array");

			auto search = m_IdentifierTable->find(m_currToken->lexema);

			if (search->second->getType() != nodeType)
				assert(false && "Invalid type identifier in array");

			Identifier->addChild(new SyntaxNode(nodeType, m_currToken, m_currLevel));
			nextToken();
		}
		else {
			checkTokenType(arrayType);
			Identifier->addChild(new SyntaxNode(nodeType, m_currToken, m_currLevel));
			nextToken();
		}
		m_IdentifierTable->insert({ Identifier->getToken()->lexema + '[' + std::to_string(iter) + ']',
			Identifier->getChildren()->back() });
	}
	nextToken();
	checkTokenType(TokenClass::Separator);

	return Identifier;
}

SyntaxNode *Parser::getSyntaxTreeRoot()
{
	return m_syntaxTreeRoot;
}

void Parser::showTree()
{
	auto iter = m_syntaxTreeRoot->getChildren()->begin();
	for (; iter != m_syntaxTreeRoot->getChildren()->end(); iter++)
	{
		showTreeNode(*iter, 0);
		std::cout << std::endl;
	}

}

void Parser::showTreeNode(SyntaxNode *node, size_t depth)
{
	if (node == NULL)
		return;

	std::cout << "|";

	for (int i = 0; i < depth; i++)
		std::cout << "-";
	std::cout << ">";

	if (node->getType() == SyntaxNodeType::UNKNOWN || node->getType() == SyntaxNodeType::EXPRESSION)
		std::cout << node->getTypeString() << std::endl;
	else 
		std::cout << node->getTypeString() + ", \"" + 
		node->getToken()->lexema + "\", level=" + 
		std::to_string(node->getLevel()) << std::endl;

	//bool check = false;

	auto iter = node->getChildren()->begin();
	for (; iter != node->getChildren()->end(); iter++)
	{
		showTreeNode(*iter, depth + 1);
		//check = true;
	}

	//if (check)
	//	std::cout << std::endl;
}

std::string SyntaxNode::getTypeString()
{
	switch (m_type)
	{
	case SyntaxNodeType::ADD:
		return "ADD";
	case SyntaxNodeType::AND:
		return "AND";
	case SyntaxNodeType::ARRAY:
		return "ARRAY";
	case SyntaxNodeType::ASSIGN:
		return "ASSIGN";
	case SyntaxNodeType::DIV:
		return "DIV";
	case SyntaxNodeType::DO:
		return "DO";
	case SyntaxNodeType::ELSE:
		return "ELSE";
	case SyntaxNodeType::EQUAL:
		return "EQUAL";
	case SyntaxNodeType::EXPRESSION:
		return "EXPRESSION";
	case SyntaxNodeType::FLOAT:
		return "FLOAT";
	case SyntaxNodeType::FOR:
		return "FOR";
	case SyntaxNodeType::ID:
		return "ID";
	case SyntaxNodeType::IF:
		return "IF";
	case SyntaxNodeType::IN:
		return "IN";
	case SyntaxNodeType::INDEX:
		return "INDEX";
	case SyntaxNodeType::INPUT:
		return "INPUT";
	case SyntaxNodeType::LESS:
		return "LESS";
	case SyntaxNodeType::LESSEQUAL:
		return "LESSEQUAL";
	case SyntaxNodeType::LOGIC:
		return "LOGIC";
	case SyntaxNodeType::MOD:
		return "MOD";
	case SyntaxNodeType::MORE:
		return "MORE";
	case SyntaxNodeType::MOREEQUAL:
		return "MOREEQUAL";
	case SyntaxNodeType::MUL:
		return "MUL";
	case SyntaxNodeType::NOTEQUAL:
		return "NOTEQUAL";
	case SyntaxNodeType::NUMBER:
		return "NUMBER";
	case SyntaxNodeType::OPERATOR:
		return "OPERATOR";
	case SyntaxNodeType::OR:
		return "OR";
	case SyntaxNodeType::PRINT:
		return "PRINT";
	case SyntaxNodeType::STATEMENT:
		return "STATEMENT";
	case SyntaxNodeType::STRLITERAL:
		return "STRLITERAL";
	case SyntaxNodeType::SUB:
		return "SUB";
	case SyntaxNodeType::WHILE:
		return "WHILE";
	case SyntaxNodeType::UNKNOWN:
		return "UNKNOWN";
	default:
		break;
	}

	return "error type for output";
}