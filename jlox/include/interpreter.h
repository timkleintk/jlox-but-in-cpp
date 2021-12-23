#pragma once
#include "environment.h"
#include "expr.h"


class Interpreter final : public Expr::Visitor, public Stmt::Visitor
{
public:
	Interpreter();

	~Interpreter() override;
	void interpret(const std::vector<Stmt*>& statements);

#define TYPE(name, ...) void visit ## name ## Expr(Expr::name& expr, void* returnValue) override;
	EXPR_TYPES;
#undef TYPE
#define TYPE(name, ...) void visit ## name ## Stmt(Stmt::name& stmt) override;
	STMT_TYPES;
#undef TYPE

	void resolve(const Expr* expr, int depth);
	std::any lookUpVariable(const Token& name, const Expr* expr);

	Environment globals;
	std::unordered_map<const Expr*, int> locals;

	void executeBlock(const std::vector<Stmt*>& stmts, Environment* environment);
private:

	Environment* m_environment = nullptr;

	//void checkNumberOperand(const Token& op, const std::any& operand);
	//void checkNumberOperands(const Token& op, const std::any& left, const std::any& right);

	std::any evaluate(Expr* expr);

	void execute(Stmt* stmt);

};

template<typename T>
bool is(const std::any& any)
{ return std::any_cast<T>(&any) != nullptr; }

// basically a wrapper for any_cast
template<typename T>
T as(const std::any& any) { return std::any_cast<T>(any); }

/*
  private boolean isTruthy(Object object) {
	if (object == null) return false;
	if (object instanceof Boolean) return (boolean)object;
	return true;
  }
*/

inline bool IsTruthy(const std::any& object)
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
template<typename T>
inline bool IsEqual(const std::any& a, const std::any& b)
{
	if (!a.has_value() && !b.has_value()) { return true; }
	if (!a.has_value()) { return false; }
	if (a.type() != b.type()) { return false; }
	if (is<T>(a)) { return as<T>(a) == as<T>(b); }

	return false;
}


