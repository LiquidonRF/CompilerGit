#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <iterator>
#include <cassert>

enum class TokenClass
{
	Comment,
	String,
	Separator,
	Comma,
	IgnoredSeparators,
	LBracket,
	RBracket,
	LSquareBracket,
	RSquareBracket,
	FloatLiteral,
	OctLiteral,
	HexLiteral,
	IntLiteral,
	LogicOperator,
	UnaryOperator,
	Assignment,
	KeyWordFor,
	TwoPoints,
	KeyWordIf,
	KeyWordElse,
	KeyWordDo,
	KeyWordWhile,
	KeyWordThen,
	KeyWordEnd,
	KeyWordIn,
	Break,
	Input,
	Output,
	Return,
	Identifier,
	EndOfFile,
	Unknown,
	BrokenHex,
	BrokenOct
};

struct Token {
	TokenClass tokenClass;
	std::string lexema;
	size_t line;
	size_t posInLine;
};

struct Rule {
	std::regex regular;
	TokenClass type;
};

class Lexer
{
public:
	void readText(std::string fileName);
	std::vector<Token> *getTokenList();
	Token *getNextToken();
	Token *getPrevToken();
	Token *getCurrToken();
	size_t getIndexCurrToken();
	void setIndexCurrToken(size_t newIndexCurrToken);
	void printTokenList();
	std::string getTokenTypeString(TokenClass token);

	Lexer() 
	{ 
		initRules();
		m_tokenList = new std::vector<Token>;
	}
	~Lexer() { m_rules.clear(); }
private:
	std::vector<Token> *m_tokenList;
	std::vector<Rule> m_rules;
	std::string m_inputText;

	void initRules();
	Token getTokenFromString();

	size_t m_indexCurrToken = 0;
	size_t m_charId = 0;
	size_t m_lineId = 1;
	size_t m_posInLine = 1;
};