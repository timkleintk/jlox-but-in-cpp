#pragma once

#include "loxClass.h"

class Token;

class LoxInstance: public GarbageCollectable<LoxInstance>
{
public:
	LoxInstance(std::shared_ptr<LoxClass> klass);

	object_t get(const Token& name);
	void set(const Token& name, const object_t& value);

	const LoxClass& getClass() const { return *m_class; }
private:
	std::shared_ptr<LoxClass> m_class;
	std::unordered_map<std::string, object_t> m_fields;
};
