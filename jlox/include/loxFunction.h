#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	//LoxFunction(const Stmt::Function& declaration, Environment* closure);
	LoxFunction(Stmt::Function declaration, Environment* closure, bool isInitializer);
	LoxFunction bind(LoxInstance* instance) const;

	std::any call(Interpreter* interpreter, std::vector<std::any> arguments) const override;
	int arity() const override;

	//std::string toString() override;
	//std::unique_ptr<LoxCallable> GetCopy() override;

	//LoxFunction& operator=(const LoxFunction&);


private:
	Stmt::Function m_declaration;
	//std::unique_ptr<Stmt::Function> m_declaration;
	//std::shared_ptr<Environment> m_closure;
	Environment* m_closure = nullptr;
	bool m_isInitializer;
};


