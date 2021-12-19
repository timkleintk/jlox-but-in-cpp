#pragma once
#include <unordered_map>

#include "object.h"
#include "RuntimeError.h"
#include "token.h"

class Environment
{
public:
	Environment(Environment* enclosing = nullptr): enclosing(enclosing) {}
	Environment(const Environment&) = delete;

	Environment* enclosing = nullptr;


	Object get(const Token& name)
	{
		// current scope
		if (const auto& it = m_values.find(name.lexeme); it != m_values.end())
		{ return it->second; }

		// enclosing scope
		if (enclosing != nullptr)
		{ return enclosing->get(name); }

		throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
	}

	void define(const std::string& name, const Object& value)
	{
		m_values.insert_or_assign(name, value);
	}

	Environment& ancestor(const int distance)
	{
		Environment* environment = this;
		for (int i = 0; i < distance; i++)
		{
			environment = environment->enclosing;
		}
		return *environment;
	}

	Object getAt(const int distance, const std::string& name)
	{
		return ancestor(distance).m_values.at(name);
	}

	void assignAt(const int distance, const Token& name, const Object& value)
	{
		ancestor(distance).m_values.insert_or_assign(name.lexeme, value);
	}

	void assign(const Token& name, const Object& value)
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

private:
	std::unordered_map<std::string, Object> m_values;
};


