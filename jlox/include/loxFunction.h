#pragma once

#include "loxCallable.h"

#include "expr.h"

class LoxInstance;
class Environment;

class LoxFunction final : public LoxCallable
{
public:
	LoxFunction(std::shared_ptr<Stmt::Function> declaration, std::shared_ptr<Environment> closure, bool isInitializer);
	~LoxFunction() override = default;

	bool operator==(const LoxFunction& other) const;

	std::shared_ptr<LoxFunction> bind(const LoxInstance& instance) const;

	object_t call(Interpreter* interpreter, const std::vector<object_t>& arguments) const override;
	size_t arity() const override;

	auto getDeclaration() const { return m_declaration; }
	auto getClosure() const { return m_closure; }

private:
	std::shared_ptr<Stmt::Function> m_declaration = nullptr;
	std::shared_ptr<Environment> m_closure = nullptr;
	bool m_isInitializer;
};


