#include "lexer.h"

void Lexer::readText(std::string fileName)
{
	std::ifstream inputStream(fileName);

	assert(inputStream.is_open() && "File not found");

	m_inputText = std::string(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());

	Token token = getTokenFromString();

	while (token.tokenClass != TokenClass::EndOfFile)
	{
		if (token.tokenClass != TokenClass::IgnoredSeparators)
			m_tokenList->push_back(token);
		token = getTokenFromString();
	}

	m_tokenList->push_back(token);
}

void Lexer::initRules()
{
	Rule rule;
	rule.type = TokenClass::Comment;
	rule.regular = "#.*\n";
	m_rules.push_back(rule);

	rule.type = TokenClass::String;
	rule.regular = "\".*?\"";
	m_rules.push_back(rule);

	rule.type = TokenClass::Separator;
	rule.regular = "\\n";
	m_rules.push_back(rule);

	rule.type = TokenClass::Comma;
	rule.regular = "\\,";
	m_rules.push_back(rule);

	rule.type = TokenClass::IgnoredSeparators;
	rule.regular = "(\\r\\n|\\r|\\s|\\t)";
	m_rules.push_back(rule);

	rule.type = TokenClass::LBracket;
	rule.regular = "\\(";
	m_rules.push_back(rule);

	rule.type = TokenClass::RBracket;
	rule.regular = "\\)";
	m_rules.push_back(rule);

	rule.type = TokenClass::LSquareBracket;
	rule.regular = "\\[";
	m_rules.push_back(rule);

	rule.type = TokenClass::RSquareBracket;
	rule.regular = "\\]";
	m_rules.push_back(rule);

	rule.type = TokenClass::FloatLiteral;
	rule.regular = "([0-9]*\\.[0-9]+)";
	m_rules.push_back(rule);

	rule.type = TokenClass::OctLiteral;
	rule.regular = "(0[0-9]+)";
	m_rules.push_back(rule);

	rule.type = TokenClass::HexLiteral;
	rule.regular = "(0x[\\d\\w]+)";
	m_rules.push_back(rule);

	rule.type = TokenClass::IntLiteral;
	rule.regular = "([1-9]+[0-9]*)|(0)";
	m_rules.push_back(rule);

	rule.type = TokenClass::LogicOperator;
	rule.regular = "==|!=|>=|<=|>|<|&&|\\|\\|";
	m_rules.push_back(rule);

	rule.type = TokenClass::UnaryOperator;
	rule.regular = "\\+|-|%|\\/|\\*";
	m_rules.push_back(rule);

	rule.type = TokenClass::Assignment;
	rule.regular = "=";
	m_rules.push_back(rule);

	rule.type = TokenClass::TwoPoints;
	rule.regular = "\\.\\.";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordFor;
	rule.regular = "(for)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordDo;
	rule.regular = "(do)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordWhile;
	rule.regular = "(while)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::Break;
	rule.regular = "(break)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordIf;
	rule.regular = "(if)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordIn;
	rule.regular = "(in)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordElse;
	rule.regular = "(else)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::Return;
	rule.regular = "(return)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordEnd;
	rule.regular = "(end)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::KeyWordThen;
	rule.regular = "(then)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::Output;
	rule.regular = "(puts)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::Input;
	rule.regular = "(gets)(?=\\W)";
	m_rules.push_back(rule);

	rule.type = TokenClass::Identifier;
	rule.regular = "[a-zA-Z]+[\\w\\d]*";
	m_rules.push_back(rule);

	rule.type = TokenClass::EndOfFile;
	rule.regular = "$";
	m_rules.push_back(rule);

}

std::vector<Token> *Lexer::getTokenList()
{
	return m_tokenList;
}

Token *Lexer::getNextToken()
{
	if (m_indexCurrToken >= 0 && m_indexCurrToken + 1 < m_tokenList->size())
		return &m_tokenList->at(++m_indexCurrToken);

	return &m_tokenList->back();
}
Token* Lexer::getPrevToken()
{
	if (m_indexCurrToken >= 0 && m_indexCurrToken - 1 < m_tokenList->size())
		return &m_tokenList->at(--m_indexCurrToken);

	return &m_tokenList->front();
}

Token* Lexer::getCurrToken()
{
	if (m_indexCurrToken >= 0 && m_indexCurrToken < m_tokenList->size())
		return &m_tokenList->at(m_indexCurrToken);

	return &m_tokenList->back();
}

size_t Lexer::getIndexCurrToken()
{
	return m_indexCurrToken;
}

void Lexer::setIndexCurrToken(size_t newIndexCurrToken)
{
	m_indexCurrToken = newIndexCurrToken;
}

void Lexer::printTokenList()
{
	auto iter = m_tokenList->begin();

	while (iter != m_tokenList->end())
	{
		if (iter->tokenClass != TokenClass::Separator)
			std::cout << getTokenTypeString(iter->tokenClass) << "\t" << iter->lexema << std::endl;
		++iter;
	}
}

Token Lexer::getTokenFromString()
{
	std::string::iterator currChar = m_inputText.begin() + m_charId;

	Token token;

	std::smatch resultedmatch;

	for (size_t i = 0; i < m_rules.size(); i++)
	{
		if (std::regex_search(std::string::const_iterator(currChar), 
			m_inputText.cend(), resultedmatch, m_rules[i].regular)
			&& resultedmatch.position(0) == 0)
		{
			std::string resultedStr = resultedmatch.str(0);

			m_charId += resultedStr.length();

			token.tokenClass = m_rules[i].type;

			std::cmatch brokenMatch;

			if (m_rules[i].type == TokenClass::OctLiteral)
			{
				if (std::regex_search(resultedStr.c_str(), brokenMatch,
					(std::regex)"(0[0-7]+)") && resultedmatch.position(0) == 0 &&
					resultedStr.length() == brokenMatch.length())
				{
					token.tokenClass = m_rules[i].type;
				}
				else {
					token.tokenClass = TokenClass::BrokenOct;
				}
			}

			if (m_rules[i].type == TokenClass::HexLiteral)
			{
				if (std::regex_search(resultedStr.c_str(), brokenMatch,
					(std::regex)"(0x[0-9A-F]+)") && resultedmatch.position(0) == 0 &&
					resultedStr.length() == brokenMatch.length())
				{
					token.tokenClass = m_rules[i].type;
				}
				else {
					token.tokenClass = TokenClass::BrokenHex;
				}
			}

			token.lexema = resultedStr;
			token.line = m_lineId;
			token.posInLine = m_posInLine;

			m_posInLine += resultedStr.length();

			for (auto& iter : resultedStr) 
				if (iter == L'\n')
				{
					m_lineId++;
					m_posInLine = 1;
				}

			return token;
		}
	}

	size_t separatorNum = 0;

	std::string unknownStr;

	token.lexema += *currChar;
	m_charId++;
		
	token.tokenClass = TokenClass::Unknown;
	token.line = m_lineId;
	token.posInLine = m_posInLine;

	m_posInLine++;

	return token;
}

std::string Lexer::getTokenTypeString(TokenClass token)
{
	switch (token)
	{
	case TokenClass::Comment:
		return "Comment";
	case TokenClass::String:
		return "String";
	case TokenClass::Separator:
		return "Separator";
	case TokenClass::Comma:
		return "Comma";
	case TokenClass::IgnoredSeparators:
		return "IgnoredSeparators";
	case TokenClass::LBracket:
		return "LBracket";
	case TokenClass::RBracket:
		return "RBracket";
	case TokenClass::LSquareBracket:
		return "LSquareBracket";
	case TokenClass::RSquareBracket:
		return "RSquareBracket";
	case TokenClass::FloatLiteral:
		return "FloatLiteral";
	case TokenClass::OctLiteral:
		return "OctLiteral";
	case TokenClass::HexLiteral:
		return "HexLiteral";
	case TokenClass::IntLiteral:
		return "IntLiteral";
	case TokenClass::LogicOperator:
		return "LogicOperator";
	case TokenClass::UnaryOperator:
		return "UnaryOperator";
	case TokenClass::Assignment:
		return "Assignment";
	case TokenClass::TwoPoints:
		return "TwoPoints";
	case TokenClass::KeyWordFor:
		return "KeyWordFor";
	case TokenClass::KeyWordIn:
		return "KeyWordIn";
	case TokenClass::KeyWordWhile:
		return "KeyWordWhile";
	case TokenClass::KeyWordIf:
		return "KeyWordIf";
	case TokenClass::KeyWordElse:
		return "KeyWordElse";
	case TokenClass::KeyWordDo:
		return "KeyWordDo";
	case TokenClass::Break:
		return "Break;";
	case TokenClass::Return:
		return "Return";
	case TokenClass::KeyWordEnd:
		return "KeyWordEnd";
	case TokenClass::KeyWordThen:
		return "KeyWordThen";
	case TokenClass::Output:
		return "Output";
	case TokenClass::Input:
		return "Input";
	case TokenClass::Identifier:
		return "Identifier";
	case TokenClass::EndOfFile:
		return "EndOfFile";
	case TokenClass::Unknown:
		return "Unknown";
	case TokenClass::BrokenHex:
		return "BrokenHex";
	case TokenClass::BrokenOct:
		return "BrokenOct";
	}

	return "Invalid Token";
}