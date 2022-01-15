#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	LoxFunction(std::shared_ptr<Stmt::Function> declaration, Environment* closure, bool isInitializer);
	~LoxFunction() override = default;

	bool operator==(const LoxFunction& as) const;

	LoxFunction bind(LoxInstance* instance) const;

	object_t call(Interpreter* interpreter, const std::vector<object_t>& arguments) const override;
	size_t arity() const override;

	auto getDeclaration() const { return m_declaration; }
	auto getClosure() const { return m_closure; }

private:
	std::shared_ptr<Stmt::Function> m_declaration = nullptr;

	//nts: make this a smart pointer
	Environment* m_closure = nullptr;
	bool m_isInitializer;
};


