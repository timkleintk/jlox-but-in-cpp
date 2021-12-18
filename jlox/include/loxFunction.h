#pragma once
#include "environment.h"
#include "expr.h"
#include "loxCallable.h"

class LoxFunction : public LoxCallable
{
public:
	LoxFunction(const Stmt::Function& declaration, Environment* closure);

	Object call(Interpreter* interpreter, std::vector<Object> arguments) const override;
	int arity() const override;
	std::string toString() override;
	std::unique_ptr<LoxCallable> GetCopy() override;
private:
	std::unique_ptr<Stmt::Function> m_declaration;
	//std::shared_ptr<Environment> m_closure;
	Environment* m_closure = nullptr;
};


