#include "loxInstance.h"

#include "RuntimeError.h"

LoxInstance::LoxInstance(const LoxClass& klass): m_class(klass)
{}

Object LoxInstance::get(const Token& name)
{
	if (m_fields.contains(name.lexeme))
	{
		return m_fields.at(name.lexeme);
	}

	if (Object method = m_class.findMethod(name.lexeme); method.type != Object::Type::NIL) return method;

	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token& name, const Object& value)
{
	m_fields.insert_or_assign(name.lexeme, value);
}

