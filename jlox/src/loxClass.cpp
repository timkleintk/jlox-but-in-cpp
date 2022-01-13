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

// nts: make sure all uses of this function have been fixed
std::optional<LoxFunction> LoxClass::findMethod(const std::string& methodName) const
{
	if (m_methods.contains(methodName))
	{ return m_methods.at(methodName); } // nts: this returns a copy, not a reference

	if (m_superclass != nullptr)
	{ return m_superclass->findMethod(methodName); }

	return {};
}

bool LoxClass::operator==(const LoxClass& as) const
{
	return as.name == name;
}

object_t LoxClass::call(Interpreter * interpreter, const std::vector<object_t> arguments) const
{
	// nts: leak
	auto* instance = new LoxInstance(*this);

	if (const auto initializer = findMethod("init"); initializer.has_value())
	{
		// nts: double check if correct
		//as<LoxFunction>(initializer).bind(instance).call(interpreter, arguments);
		initializer.value().bind(instance).call(interpreter, arguments); // call the initializer?
	}

	return {instance};
}

int LoxClass::arity() const
{
	if (const auto initializer = findMethod("init"); initializer.has_value())
	{ return initializer->arity(); }
	return 0;
}
