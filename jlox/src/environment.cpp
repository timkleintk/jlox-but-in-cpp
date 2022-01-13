#include "environment.h"

#include <iostream>

#include "lox.h"
#include "RuntimeError.h"

Environment::Environment(Environment* enclosing): m_enclosing(enclosing)
{
	
}

Environment::~Environment()
{

}

object_t Environment::get(const Token& name)
{
	// current scope
	if (const auto& it = m_values.find(name.lexeme); it != m_values.end())
	{ return it->second; }

	// enclosing scope
	if (m_enclosing != nullptr)
	{ return m_enclosing->get(name); }

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::define(const std::string& name, const object_t& value)
{
	m_values.insert_or_assign(name, value);
}

void Environment::debugPrint() const
{
	if (m_enclosing)
	{
		m_enclosing->debugPrint();
		std::cout << "^\n|\n";
	}
	std::cout << "Environment\n";

	for (auto& [key, value] : m_values)
	{
		std::cout << "  [\"" << key << "\"]: " << toString(value) << "\n";
	}
}

Environment& Environment::ancestor(const int distance)
{
	Environment* environment = this;
	for (int i = 0; i < distance; i++)
	{
		environment = environment->m_enclosing;
	}
	return *environment;
}

object_t Environment::getAt(const int distance, const std::string& name)
{
	return ancestor(distance).m_values.at(name);
}

void Environment::assignAt(const int distance, const Token& name, const object_t& value)
{
	ancestor(distance).m_values.insert_or_assign(name.lexeme, value);
}

void Environment::assign(const Token& name, const object_t& value)
{
	if (m_values.contains(name.lexeme))
	{
		m_values.at(name.lexeme) = value;
		return;
	}

	if (m_enclosing != nullptr)
	{
		m_enclosing->assign(name, value);
		return;
	}

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}
