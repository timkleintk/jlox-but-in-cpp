#pragma once

#include <string>
#include <vector>

#include "environment.h"
#include "expr.h"
#include "object.h"


inline bool IsNumber(const Object& object)
{
	return object.type == Object::Type::NUMBER;
}
inline double ToNumber(const Object& object)
{
	if (object.type == Object::Type::NUMBER)
	{
		return object.number;
	}
	return NAN;

}

inline bool IsString(const Object& object)
{
	return object.type == Object::Type::STRING;
}
inline const std::string& ToString(const Object& object) // nts: returntype?
{
	return object.string;
}

inline bool IsTruthy(const Object& object)
{
	if (object.type == Object::Type::NIL) { return false; }
	if (object.type == Object::Type::BOOL) { return object.boolean; }
	return true;
}
inline bool IsEqual(const Object& left, const Object& right)
{
	// if (left.isNull() && right.isN
	return left.equals(right);
}

class Interpreter final : public Expr::Visitor, public Stmt::Visitor
{
public:
	Interpreter();

	~Interpreter() override;
	void interpret(const std::vector<Stmt*>& statements);
	
#define TYPE(name, ...) void visit ## name ## Expr(Expr::name& expr, void* returnValue) override;
	EXPR_TYPES
#undef TYPE
#define TYPE(name, ...) void visit ## name ## Stmt(Stmt::name& stmt) override;
	STMT_TYPES
#undef TYPE

	Environment globals;

	void executeBlock(const std::vector<Stmt*>& stmts, std::unique_ptr<Environment> environment);
private:

	Environment* m_environment = nullptr;

	void checkNumberOperand(const Token& op, const Object& operand);
	void checkNumberOperands(const Token& op, const Object& left, const Object& right);

	Object evaluate(Expr* expr);

	void execute(Stmt* stmt);

};

