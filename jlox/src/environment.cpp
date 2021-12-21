#include "environment.h"

#include "RuntimeError.h"

Environment::Environment(Environment* enclosing): enclosing(enclosing)
{}

Object Environment::get(const Token& name)
{
	// current scope
	if (const auto& it = m_values.find(name.lexeme); it != m_values.end())
	{ return it->second; }

	// enclosing scope
	if (enclosing != nullptr)
	{ return enclosing->get(name); }

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::define(const std::string& name, const Object& value)
{
	m_values.insert_or_assign(name, value);
}

Environment& Environment::ancestor(const int distance)
{
	Environment* environment = this;
	for (int i = 0; i < distance; i++)
	{
		environment = environment->enclosing;
	}
	return *environment;
}

Object Environment::getAt(const int distance, const std::string& name)
{
	return ancestor(distance).m_values.at(name);
}

void Environment::assignAt(const int distance, const Token& name, const Object& value)
{
	ancestor(distance).m_values.insert_or_assign(name.lexeme, value);
}

void Environment::assign(const Token& name, const Object& value)
{
	if (m_values.contains(name.lexeme))
	{
		m_values.at(name.lexeme) = value;
		return;
	}

	if (enclosing != nullptr)
	{
		enclosing->assign(name, value);
		return;
	}

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}
