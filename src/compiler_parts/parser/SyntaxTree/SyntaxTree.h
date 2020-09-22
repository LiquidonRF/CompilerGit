#pragma once
#include <list>

class SyntaxNode
{
public:
	Token *getToken();
	void setToken(Token *newToken);
	size_t getLevel();
	void setLevel(size_t newLevel);
	SyntaxNode* getParent();
	std::list<SyntaxNode*> *getChildren();
	void addChild(SyntaxNode *child);
	void removeChild(SyntaxNode *child);
	size_t lookupChildIndex(SyntaxNode* child);

	SyntaxNode()
	{
		m_token = NULL;
		m_level = 0;
		m_parent = NULL;
		m_children = NULL;
	}

	SyntaxNode(Token *newToken)
	{
		m_token = newToken;
		m_level = 0;
		m_parent = NULL;
		m_children = NULL;
	}

	SyntaxNode(Token* newToken, size_t newLevel)
	{
		m_token = newToken;
		m_level = newLevel;
		m_parent = NULL;
		m_children = NULL;
	}

private:
	Token *m_token;
	size_t m_level;
	SyntaxNode *m_parent;
	std::list<SyntaxNode*> *m_children = new std::list<SyntaxNode*>();
};