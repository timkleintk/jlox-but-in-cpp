#pragma once

#include <string>

#include "loxCallable.h"

class LoxFunction;
class Object;



class LoxClass final : public LoxCallable
{
public:
	LoxClass(std::string name, const std::unordered_map<std::string, LoxFunction>& methods);
	~LoxClass() override;
	LoxClass(const LoxClass& klass);
	LoxClass& operator=(const LoxClass& klass) = delete;


	Object findMethod(const std::string& methodName) const;

	std::string name;

	
	std::string toString() const;

	Object call(Interpreter* interpreter, std::vector<Object> arguments) const override;
	//[[nodiscard]] int arity() const override;
	std::unique_ptr<LoxCallable> GetCopy() override;
	std::string toString() override;
	[[nodiscard]] int arity() const override;
private:
	std::unordered_map<std::string, LoxFunction*> m_methods;
};

