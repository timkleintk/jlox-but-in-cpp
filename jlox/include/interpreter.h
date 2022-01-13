#pragma once

#include "environment.h"
#include "expr.h"
#include "loxCallable.h"


class Interpreter final : public Expr::Visitor, public Stmt::Visitor
{
public:
	Interpreter();
	~Interpreter() override = default;

	void interpret(const std::vector<std::shared_ptr<Stmt>>& statements);

#define TYPE(name, ...) object_t visit ## name ## Expr(Expr::name& expr) override;
	EXPR_TYPES;
#undef TYPE
#define TYPE(name, ...) void visit ## name ## Stmt(Stmt::name& stmt) override;
	STMT_TYPES;
#undef TYPE

	void resolve(std::shared_ptr<Expr> expr, int depth);
	object_t lookUpVariable(const Token& name, const std::shared_ptr<Expr>& expr);

	void executeBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, Environment* environment);

	Environment globals;
	std::unordered_map<std::shared_ptr<Expr>, int> locals; // nts: make it weak pointer?
private:

	Environment* m_environment = nullptr;
	std::shared_ptr<LoxCallable> m_clockFunction = nullptr;

	template <typename Ptr>
	object_t evaluate(Ptr expr) { return expr->accept(this); }
	//object_t evaluate(const std::unique_ptr<Expr>& expr) { return evaluate(expr.get()); }
	//object_t evaluate(const std::shared_ptr<Expr>& expr) { return evaluate(expr.get()); }

	template <typename Ptr>
	void execute(Ptr stmt) { stmt->accept(this); }
	//void execute(const std::unique_ptr<Stmt>& stmt) { execute(stmt.get()); }
	//void execute(const std::shared_ptr<Stmt>& stmt) { execute(stmt.get()); }
};





