#include "SyntaxTree.h"

Token *SyntaxNode::getToken()
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

std::list<SyntaxNode*>* SyntaxNode::getChildren()
{
	return m_children;
}

void SyntaxNode::setLevel(size_t newLevel)
{
	m_level = newLevel;
}

void SyntaxNode::addChild(SyntaxNode *child)
{
	if (child->m_parent != NULL)
		child->m_parent->m_children->remove(child);

	m_children->remove(child);
	m_children->push_back(child);
	child->m_parent = this;
}

void SyntaxNode::removeChild(SyntaxNode *child)
{
	m_children->remove(child);

	if (child->m_parent == this)
		child->m_parent = NULL;
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