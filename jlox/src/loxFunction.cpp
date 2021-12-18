#include "loxFunction.h"

#include <utility>

#include "environment.h"
#include "interpreter.h"
#include "return.h"

LoxFunction::LoxFunction(const Stmt::Function& declaration, Environment* closure): m_declaration(std::make_unique<Stmt::Function>(declaration)), m_closure(closure)
{}

Object LoxFunction::call(Interpreter* interpreter, const std::vector<Object> arguments) const
{
	// when you call a function it is in global scope
	//const auto environment = std::make_unique<Environment>(&interpreter->globals);

	const auto environment = std::make_unique<Environment>(m_closure);

	for (size_t i = 0; i < m_declaration->params.size(); i++)
	{
		environment->define(m_declaration->params[i].lexeme, arguments[i]);
	}

	try
	{
		interpreter->executeBlock(m_declaration->body, std::make_unique<Environment>(environment.get()));
	}
	catch (Return& r)
	{
		return r.value;
	}

	// nts: this is not really expressive
	return Object::Nil();
}

int LoxFunction::arity() const
{
	//nts: this could be a variable in LoxCallable probably
	return static_cast<int>(m_declaration->params.size());
}

std::string LoxFunction::toString()
{
	return "<fn " + m_declaration->name.lexeme + ">";
}

std::unique_ptr<LoxCallable> LoxFunction::GetCopy()
{
	// nts: is this correct?
	return std::make_unique<LoxFunction>(*m_declaration, m_closure);
}
