#include "environment.h"

#include <iostream>

#include "lox.h"
#include "RuntimeError.h"

Environment::Environment(std::shared_ptr<Environment> enclosing): m_enclosing(std::move(enclosing))
{}


std::shared_ptr<Environment> Environment::getEnclosing() const
{
	return m_enclosing;
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

object_t Environment::getAt(const size_t distance, const std::string& name)
{
	return ancestor(distance).m_values.at(name);
}

void Environment::assign(const Token& name, object_t value)
{
	if (m_values.contains(name.lexeme))
	{
		m_values.at(name.lexeme) = std::move(value);
		return;
	}

	if (m_enclosing != nullptr)
	{
		m_enclosing->assign(name, std::move(value));
		return;
	}

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}
void Environment::assignAt(const size_t distance, const Token& name, object_t value)
{
	ancestor(distance).m_values.insert_or_assign(name.lexeme, std::move(value));
}

void Environment::define(const std::string& name, object_t value)
{
	m_values.insert_or_assign(name, std::move(value));
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


Environment& Environment::ancestor(const size_t distance)
{
	Environment* environment = this;
	for (size_t i = 0; i < distance; i++)
	{
		environment = environment->m_enclosing.get();
	}
	return *environment;
}

