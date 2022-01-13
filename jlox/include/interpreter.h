#pragma once
#include "environment.h"
#include "expr.h"


class Interpreter final : public Expr::Visitor, public Stmt::Visitor
{
public:
	Interpreter();
	~Interpreter() override;

	void interpret(const std::vector<Stmt*>& statements);
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
	std::unordered_map<std::shared_ptr<Expr>, int> locals; // nts: is shared_ptr<Expr> the right type here?
private:

	Environment* m_environment = nullptr;

	object_t evaluate(Expr* expr);
	object_t evaluate(const std::unique_ptr<Expr>& expr) { return evaluate(expr.get()); }
	object_t evaluate(const std::shared_ptr<Expr>& expr) { return evaluate(expr.get()); }

	void execute(Stmt* stmt);
	void execute(const std::unique_ptr<Stmt>& stmt) { execute(stmt.get()); }
	void execute(const std::shared_ptr<Stmt>& stmt) { execute(stmt.get()); }
};



/*
  private boolean isTruthy(Object object) {
	if (object == null) return false;
	if (object instanceof Boolean) return (boolean)object;
	return true;
  }
*/

inline bool IsTruthy(const object_t& object)
{
	if (isNull(object)) return false;
	if (is<bool>(object)) return as<bool>(object);
	return true;
}

/*
  private boolean isEqual(Object a, Object b) {
	if (a == null && b == null) return true;
	if (a == null) return false;

	return a.equals(b);
  }
*/

inline bool IsEqual(const object_t& a, const object_t& b);

