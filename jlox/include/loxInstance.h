#pragma once

#include "loxClass.h"

class Token;

class LoxInstance: public GarbageCollectable<LoxInstance>
{
public:
	LoxInstance(LoxClass klass);

	object_t get(const Token& name);
	void set(const Token& name, const object_t& value);

	const LoxClass& getClass() { return m_class; }
private:
	LoxClass m_class; // nts: make this a smart pointer?
	std::unordered_map<std::string, object_t> m_fields;
};
