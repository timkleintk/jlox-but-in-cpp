#pragma once

#include "loxClass.h"

class Token;

class LoxInstance
{
public:
	LoxInstance(const LoxClass& klass);

	object_t get(const Token& name);
	void set(const Token& name, const object_t& value);

	const LoxClass& getClass() { return m_class; }
private:
	LoxClass m_class;
	std::unordered_map<std::string, object_t> m_fields;
};
