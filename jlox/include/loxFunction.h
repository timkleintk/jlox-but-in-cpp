#pragma once

#include "loxCallable.h"

#include "expr.h"

class Environment;

class LoxFunction final : public LoxCallable
{
public:
	//LoxFunction(const Stmt::Function& declaration, Environment* closure);
	LoxFunction(const Stmt::Function& declaration, Environment* closure);

	Object call(Interpreter* interpreter, std::vector<Object> arguments) const override;
	int arity() const override;
	std::string toString() override;
	std::unique_ptr<LoxCallable> GetCopy() override;

	//LoxFunction& operator=(const LoxFunction&);

private:
	Stmt::Function m_declaration;
	//std::unique_ptr<Stmt::Function> m_declaration;
	//std::shared_ptr<Environment> m_closure;
	Environment* m_closure = nullptr;
};


