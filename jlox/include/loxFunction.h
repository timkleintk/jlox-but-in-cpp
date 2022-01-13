#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	LoxFunction(std::shared_ptr<Stmt::Function> declaration, Environment* closure, bool isInitializer);

	LoxFunction(const LoxFunction&) = default;
	LoxFunction& operator=(const LoxFunction&) = default;
	LoxFunction(LoxFunction&&) = default;
	LoxFunction& operator=(LoxFunction&&) = default;
	~LoxFunction() override;

	bool operator==(const LoxFunction& as) const;

	LoxFunction bind(LoxInstance* instance) const;

	object_t call(Interpreter* interpreter, std::vector<object_t> arguments) const override;
	int arity() const override;

	auto getDeclaration() const { return m_declaration; }
	auto getClosure() const { return m_closure; }


private:
	std::shared_ptr<Stmt::Function> m_declaration = nullptr;

	//nts: make this a unique pointer
	Environment* m_closure = nullptr;
	bool m_isInitializer;
};


