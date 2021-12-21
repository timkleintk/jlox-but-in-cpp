#pragma once

#include "loxClass.h"

class Token;

class LoxInstance
{
public:
	LoxInstance(const LoxClass& klass);

	Object get(const Token& name);
	void set(const Token& name, const Object& value);

	std::string toString()
	{
		return m_class.toString() + " instance";
	}
	
private:
	LoxClass m_class;
	std::unordered_map<std::string, Object> m_fields;
};
