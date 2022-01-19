#include "loxClass.h"

#include <utility>

#include "loxFunction.h"
#include "loxInstance.h"
#include "object.h"

LoxClass::LoxClass(std::string name, std::shared_ptr<LoxClass> superclass, std::unordered_map<std::string, LoxFunction> methods) :
	name(std::move(name)),
	m_superclass(std::move(superclass)),
	m_methods(std::move(methods))
{}

bool LoxClass::operator==(const LoxClass& as) const
{
	return as.name == name;
}

std::optional<LoxFunction> LoxClass::findMethod(const std::string& methodName) const
{
	if (m_methods.contains(methodName))
	{
		// nts: this returns a copy, not a reference
		return m_methods.at(methodName);
	}

	if (m_superclass != nullptr)
	{
		return m_superclass->findMethod(methodName);
	}

	return {};
}

object_t LoxClass::call(Interpreter* interpreter, const std::vector<object_t>& arguments) const
{
	std::shared_ptr<LoxInstance> instance = newShared<LoxInstance>(*this);

	if (const auto initializer = findMethod("init"); initializer.has_value())
	{
		initializer.value().bind(*instance).call(interpreter, arguments); // call the initializer
	}

	return { instance };
}

size_t LoxClass::arity() const
{
	if (const auto initializer = findMethod("init"); initializer.has_value())
	{
		return initializer->arity();
	}
	return 0;
}
