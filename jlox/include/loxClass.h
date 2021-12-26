#pragma once

#include <string>

#include "loxCallable.h"

class LoxFunction;

class LoxClass final : public LoxCallable
{
public:
	LoxClass(std::string name, LoxClass* superclass, std::unordered_map<std::string, LoxFunction> methods);
	~LoxClass() override;

	LoxClass& operator=(const LoxClass& klass) = delete;


	object_t findMethod(const std::string& methodName) const;

	std::string name;
	LoxClass* m_superclass;

	object_t call(Interpreter* interpreter, std::vector<object_t> arguments) const override;
	//[[nodiscard]] int arity() const override;
	//std::unique_ptr<LoxCallable> GetCopy() override;
	//std::string toString() override;
	[[nodiscard]] int arity() const override;
private:
	std::unordered_map<std::string, LoxFunction> m_methods;
};

