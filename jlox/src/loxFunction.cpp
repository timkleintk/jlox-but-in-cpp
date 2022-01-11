#include "loxFunction.h"

#include <cassert>

#include "environment.h"
#include "interpreter.h"
#include "return.h"

LoxFunction::LoxFunction(const Stmt::Function* declaration, Environment* closure, const bool isInitializer):
	m_declaration(declaration),
	m_closure(closure),
	m_isInitializer(isInitializer)
{}

LoxFunction::~LoxFunction()
{
}

bool LoxFunction::operator==(const LoxFunction& as) const
{
	return as.getClosure() == m_closure && as.getDeclaration() == m_declaration;
}

// binds the "this" to the correct instance
LoxFunction LoxFunction::bind(LoxInstance* instance) const
{
	auto* environment = new Environment(m_closure);
	environment->define("this", instance);
	return {m_declaration, environment, m_isInitializer};
}

object_t LoxFunction::call(Interpreter* interpreter, const std::vector<object_t> arguments) const
{
	assert(m_closure != nullptr);
	const auto environment = new Environment(m_closure);
	
	for (size_t i = 0; i < m_declaration->params.size(); i++)
	{
		environment->define(m_declaration->params[i].lexeme, arguments[i]);
	}

	try
	{
		interpreter->executeBlock(m_declaration->body, environment);
	}
	catch (Return& r)
	{
		if (m_isInitializer) { return m_closure->getAt(0, "this"); }
		return r.value;
	}

	if (m_isInitializer)  return m_closure->getAt(0, "this");
	return {};
}

int LoxFunction::arity() const
{
	return static_cast<int>(m_declaration->params.size());
}
