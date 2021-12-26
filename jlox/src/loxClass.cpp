#include "loxClass.h"

#include <utility>

#include "loxFunction.h"
#include "loxInstance.h"
#include "object.h"

LoxClass::LoxClass(std::string name, LoxClass* superclass, std::unordered_map<std::string, LoxFunction> methods):
	name(std::move(name)),
	m_superclass(superclass),
	m_methods(std::move(methods))
{
}

LoxClass::~LoxClass() = default;

//LoxClass::LoxClass(const LoxClass & klass): name(klass.name)
//{
//	for (const auto& [methodName, func] : klass.m_methods)
//	{
//		m_methods.insert_or_assign(methodName, func);
//	}
//}

object_t LoxClass::findMethod(const std::string & methodName) const
{
	if (m_methods.contains(methodName))
	{ return {m_methods.at(methodName)}; } // nts: this returns a copy, not a reference

	if (m_superclass != nullptr)
	{
		return m_superclass->findMethod(methodName);
	}

	return {};
}

object_t LoxClass::call(Interpreter * interpreter, const std::vector<object_t> arguments) const
{
	// nts: leak
	auto* instance = new LoxInstance(*this);

	if (const auto initializer = findMethod("init"); !isNull(initializer))
	{
		as<LoxFunction>(initializer).bind(instance).call(interpreter, arguments);
	}

	return {instance};
}

int LoxClass::arity() const
{
	if (const auto initializer = findMethod("init"); !isNull(initializer))
	{
		return as<LoxFunction>(initializer).arity();
	}

	return 0;
}
