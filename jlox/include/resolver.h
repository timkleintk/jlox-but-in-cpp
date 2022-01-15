#pragma once

#include "expr.h"

#include <stack>
#include <unordered_map>

class Interpreter;

class Resolver final : public Stmt::Visitor, public Expr::Visitor
{
public:
	explicit Resolver(Interpreter& interpreter);

	template <typename T>
	void resolve(const T& ptr)
	{
		ptr->accept(this);
	}

	template <typename T>
	void resolve(const std::vector<T>& stmts)
	{
		for (const auto& stmt : stmts)
			resolve(stmt);
	}

#define TYPE(name, ...) void visit ## name ## Stmt(Stmt::name& stmt) override;
	STMT_TYPES;
#undef TYPE
#define TYPE(name, ...) object_t visit ## name ## Expr(Expr::name& expr) override;
	EXPR_TYPES;
#undef TYPE

private:
	enum class FunctionType
	{
		NONE,
		FUNCTION,
		INITIALIZER,
		METHOD
	};

	enum class ClassType
	{
		NONE,
		CLASS,
		SUBCLASS
	};

	void resolveFunction(const Stmt::Function& function, FunctionType type);

	void beginScope();
	void endScope();

	void declare(const Token& name);
	void define(const Token& name);

	// resolve a l-value
	void resolveLocal(std::shared_ptr<Expr> expr, const Token& name) const;


	Interpreter& m_interpreter;
	std::stack<std::unordered_map<std::string, bool>> m_scopes;
	FunctionType m_currentFunction = FunctionType::NONE;
	ClassType m_currentClass = ClassType::NONE;
};
