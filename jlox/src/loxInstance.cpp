#include "loxInstance.h"

#include "loxFunction.h"
#include "RuntimeError.h"

LoxInstance::LoxInstance(const LoxClass& klass): m_class(klass)
{}

Object LoxInstance::get(const Token& name)
{
	// field
	if (m_fields.contains(name.lexeme))
	{
		return m_fields.at(name.lexeme);
	}

	// method
	const Object method = m_class.findMethod(name.lexeme);
	if (method.type != Object::Type::NIL)
	{
		const auto* loxFunction = reinterpret_cast<LoxFunction*>(method.callable.get());
		return Object(std::make_unique<LoxFunction>(loxFunction->bind(this)));
	}

	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token& name, const Object& value)
{
	m_fields.insert_or_assign(name.lexeme, value);
}

