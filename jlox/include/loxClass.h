#pragma once

#include <string>

#include "loxCallable.h"

class LoxFunction;

class LoxClass final : public LoxCallable
{
public:
	LoxClass(std::string name, const std::unordered_map<std::string, LoxFunction>& methods);
	~LoxClass() override;

	LoxClass(const LoxClass& klass);
	LoxClass& operator=(const LoxClass& klass) = delete;


	LoxFunction findMethod(const std::string& methodName) const;

	std::string name;

	
	//std::string toString() const;

	std::any call(Interpreter* interpreter, std::vector<std::any> arguments) const override;
	//[[nodiscard]] int arity() const override;
	//std::unique_ptr<LoxCallable> GetCopy() override;
	//std::string toString() override;
	[[nodiscard]] int arity() const override;
private:
	std::unordered_map<std::string, LoxFunction*> m_methods;
};

