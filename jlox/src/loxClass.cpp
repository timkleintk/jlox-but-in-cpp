#include "loxClass.h"

#include <utility>

#include "loxFunction.h"
#include "loxInstance.h"
#include "object.h"

LoxClass::LoxClass(std::string name, std::shared_ptr<LoxClass> superclass, std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods) :
	name(std::move(name)),
	superclass(std::move(superclass)),
	m_methods(std::move(methods))
{}

bool LoxClass::operator==(const LoxClass& klass) const
{
	return klass.name == name;
}

std::shared_ptr<LoxFunction> LoxClass::findMethod(const std::string& methodName) const
{
	// try to find function in current class
	if (m_methods.contains(methodName))
	{
		return m_methods.at(methodName);
	}

	// resort to parent class
	if (superclass != nullptr)
	{
		return superclass->findMethod(methodName);
	}

	return nullptr;
}

object_t LoxClass::call(Interpreter* interpreter, const std::vector<object_t>& arguments) const
{
	std::shared_ptr<LoxInstance> instance = newShared<LoxInstance>(std::dynamic_pointer_cast<LoxClass>(getShared()));

	if (const auto initializer = findMethod("init"); initializer != nullptr)
	{
		// call the initializer
		initializer->bind(*instance)->call(interpreter, arguments); 
	}

	return  instance;
}

size_t LoxClass::arity() const
{
	if (const auto initializer = findMethod("init"); initializer != nullptr)
	{
		return initializer->arity();
	}
	return 0;
}
