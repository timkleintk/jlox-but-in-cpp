#include "loxFunction.h"

#include <cassert>

#include "environment.h"
#include "interpreter.h"
#include "loxInstance.h"
#include "return.h"

LoxFunction::LoxFunction(std::shared_ptr<Stmt::Function> declaration, std::shared_ptr<Environment> closure, const bool isInitializer) :
	m_declaration(std::move(declaration)),
	m_closure(std::move(closure)),
	m_isInitializer(isInitializer)
{}

bool LoxFunction::operator==(const LoxFunction& other) const
{
	return other.getClosure() == m_closure && other.getDeclaration() == m_declaration;
}

// returns a copy of the function with the "this" pointer bound
std::shared_ptr<LoxFunction> LoxFunction::bind(const LoxInstance& instance) const
{
	auto environment = newShared<Environment>(m_closure);
	environment->define("this", instance.getShared());
	return newShared<LoxFunction>(m_declaration, std::move(environment), m_isInitializer);
}

object_t LoxFunction::call(Interpreter* interpreter, const std::vector<object_t>& arguments) const
{
	auto environment = newShared<Environment>(m_closure);

	for (size_t i = 0; i < m_declaration->params.size(); i++)
	{
		environment->define(m_declaration->params[i].lexeme, arguments[i]);
	}

	try
	{
		interpreter->executeBlock(m_declaration->body, std::move(environment));
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
