#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	LoxFunction(Stmt::Function declaration, Environment* closure, bool isInitializer);
	LoxFunction bind(LoxInstance* instance) const;

	std::any call(Interpreter* interpreter, std::vector<std::any> arguments) const override;
	int arity() const override;


private:
	Stmt::Function m_declaration;
	Environment* m_closure = nullptr;
	bool m_isInitializer;
};


