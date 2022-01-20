#include "loxInstance.h"

#include "loxFunction.h"
#include "RuntimeError.h"

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass) : m_class(std::move(klass))
{}

object_t LoxInstance::get(const Token& name)
{
	// field
	if (m_fields.contains(name.lexeme))
	{
		return m_fields.at(name.lexeme);
	}

	// method
	if (const auto method = m_class->findMethod(name.lexeme); method != nullptr)
	{
		// return a copy of the classes method, but with "this" defined in it's closure
		return method->bind(*this);
	}

	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token& name, const object_t& value)
{
	m_fields.insert_or_assign(name.lexeme, value);
}

