#include "loxClass.h"

#include <utility>

#include "loxFunction.h"
#include "loxInstance.h"
#include "object.h"

LoxClass::LoxClass(std::string name, const std::unordered_map<std::string, LoxFunction>& methods): name(std::move(name)) 
{
	for (const auto& [methodName, methodFunction] : methods)
	{
		m_methods.insert_or_assign(methodName, new LoxFunction(methodFunction));
	}
}

LoxClass::~LoxClass()
{
	for (const auto& [string, method] : m_methods)
	{
		delete method;
	}
}

// nts: baseclass calls?
LoxClass::LoxClass(const LoxClass& klass): name(klass.name)
{
	for (const auto& [methodName, func] : klass.m_methods)
	{
		m_methods.insert_or_assign(methodName, new LoxFunction(*func));
	}
}

//LoxClass& LoxClass::operator=(const LoxClass& klass)
//{
//	
//}


Object LoxClass::findMethod(const std::string& methodName) const
{
	if (m_methods.contains(methodName))
	{ return Object(m_methods.at(methodName)->GetCopy()); } // nts: this returns a copy, not a reference

	return Object::Nil();
}

std::unique_ptr<LoxCallable> LoxClass::GetCopy()
{
	//return LoxCallable::GetCopy();
	return std::make_unique<LoxClass>(*this);
}

std::string LoxClass::toString()
{
	return name;
}

std::string LoxClass::toString() const
{
	return name;
}

Object LoxClass::call(Interpreter* interpreter, std::vector<Object> arguments) const
{
	auto instance = std::make_unique<LoxInstance>(*this);

	if (const auto initializer = findMethod("init"); initializer.type != Object::Type::NIL)
	{
		dynamic_cast<LoxFunction*>(initializer.callable.get())->bind(instance.get()).call(interpreter, arguments);
	}

	return Object(std::move(instance));
}

int LoxClass::arity() const
{
	if (const auto initializer = findMethod("init"); initializer.type != Object::Type::NIL)
	{
		return initializer.callable->arity();
	}
	return 0;
}
