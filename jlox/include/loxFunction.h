#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	LoxFunction(const Stmt::Function* declaration, Environment* closure, bool isInitializer);

	LoxFunction(const LoxFunction&) = default;
	LoxFunction& operator=(const LoxFunction&) = default;
	LoxFunction(LoxFunction&&) = default;
	LoxFunction& operator=(LoxFunction&&) = default;
	~LoxFunction() override;

	bool operator==(const LoxFunction& as) const;

	LoxFunction bind(LoxInstance* instance) const;

	object_t call(Interpreter* interpreter, std::vector<object_t> arguments) const override;
	int arity() const override;

	const Stmt::Function* getDeclaration() const { return m_declaration; }
	Environment* getClosure() const { return m_closure; }


private:
	//nts: can this be a reference?
	const Stmt::Function* m_declaration = nullptr;
	//nts: make this a unique pointer
	Environment* m_closure = nullptr;
	bool m_isInitializer;
};


