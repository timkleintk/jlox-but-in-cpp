#include "loxFunction.h"

#include <cassert>

#include "environment.h"
#include "interpreter.h"
#include "return.h"

LoxFunction::LoxFunction(std::shared_ptr<Stmt::Function> declaration, Environment* closure, const bool isInitializer):
	m_declaration(std::move(declaration)),
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

// returns a copy of the function with the "this" pointer bound
LoxFunction LoxFunction::bind(LoxInstance* instance) const
{
	auto* environment = new Environment(m_closure);
	environment->define("this", instance);
	return {m_declaration, environment, m_isInitializer};
	//return std::make_unique<LoxFunction>(m_declaration, m_closure, m_isInitializer);
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

size_t LoxFunction::arity() const
{
	return m_declaration->params.size();
}
