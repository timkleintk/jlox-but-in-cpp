#pragma once
#include "environment.h"
#include "expr.h"


class Interpreter final : public Expr::Visitor, public Stmt::Visitor
{
public:
	Interpreter();
	~Interpreter() override;

	void interpret(const std::vector<Stmt*>& statements);

#define TYPE(name, ...) object_t visit ## name ## Expr(Expr::name& expr) override;
	EXPR_TYPES;
#undef TYPE
#define TYPE(name, ...) void visit ## name ## Stmt(Stmt::name& stmt) override;
	STMT_TYPES;
#undef TYPE

	void resolve(const Expr* expr, int depth);
	object_t lookUpVariable(const Token& name, const Expr* expr);

	void executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts, Environment* environment);

	Environment globals;
	std::unordered_map<const Expr*, int> locals;
private:

	Environment* m_environment = nullptr;

	object_t evaluate(Expr* expr);

	void execute(Stmt* stmt);
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
	if (!object.has_value()) return false;
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

