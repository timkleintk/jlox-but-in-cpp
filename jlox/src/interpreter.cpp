#include "interpreter.h"

#include <any>
#include <chrono>
#include <iostream>
#include <vector>

#include "lox.h"
#include "loxCallable.h"
#include "loxFunction.h"
#include "return.h"
#include "RuntimeError.h"


Interpreter::Interpreter()
{
	//m_environment = new Environment(&globals);
	m_environment = &globals;

	globals.define("clock", Object(
		std::make_unique<LoxCallable>(
			[](Interpreter*, const std::vector<Object>&) -> Object
	{
		return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	},
			0
		)
	));

}

Interpreter::~Interpreter()
{
	delete m_environment;
}

Object Interpreter::evaluate(Expr* expr)
{
	Object result;
	expr->accept(this, &result);
	return result;
}

void Interpreter::execute(Stmt* stmt)
{
	stmt->accept(this);
}

//void Interpreter::executeBlock(const std::vector<Stmt*>& stmts, const std::shared_ptr<Environment> environment)
void Interpreter::executeBlock(const std::vector<Stmt*>& stmts, Environment* environment)
{
	Environment* previous = m_environment;

	// make a new scope
	//m_environment = environment.get();
	m_environment = environment;

	try
	{
		for (const auto& stmt : stmts)
		{
			execute(stmt);
		}
	}
	catch (...)
	{
		m_environment = previous;
		throw;
	}

	m_environment = previous;

}

void Interpreter::interpret(const std::vector<Stmt*>& statements)
{
	try
	{
		for (Stmt* statement : statements)
		{
			execute(statement);
		}
	}
	catch (RuntimeError& error)
	{
		Lox::runtimeError(error);
	}
}


// Expressions
#define RET(value) *static_cast<Object*>(returnValue) = Object(value); return
void Interpreter::visitBinaryExpr(Expr::Binary& expr, void* returnValue)
{
	const Object left = evaluate(expr.left);
	const Object right = evaluate(expr.right);

	switch (expr.op.type)
	{
		// comparisons
	case GREATER:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) > ToNumber(right));
	case GREATER_EQUAL:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) >= ToNumber(right));
	case LESS:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) < ToNumber(right));
	case LESS_EQUAL:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) <= ToNumber(right));
	case BANG_EQUAL:
		checkNumberOperands(expr.op, left, right);
		RET(!IsEqual(left, right));
	case EQUAL_EQUAL:
		checkNumberOperands(expr.op, left, right);
		RET(IsEqual(left, right));

		// arithmetic
	case MINUS:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) - ToNumber(right));
	case PLUS:
		if (IsNumber(left) && IsNumber(right))
		{ RET(ToNumber(left) + ToNumber(right)); }
		if (IsString(left) && IsString(right))
		{ RET(ToString(left) + ToString(right)); }
		throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
	case SLASH:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) / ToNumber(right));
	case STAR:
		checkNumberOperands(expr.op, left, right);
		RET(ToNumber(left) * ToNumber(right));
	}

	__debugbreak();
}

void Interpreter::visitCallExpr(Expr::Call& expr, void* returnValue)
{
	const Object callee = evaluate(expr.callee);

	std::vector<Object> arguments;
	for (const auto& arg : expr.arguments)
	{
		arguments.push_back(evaluate(arg));
	}

	if (callee.type != Object::Type::CALLABLE)
	{
		throw RuntimeError(std::move(expr.paren), "Can only call functions and classes.");
	}

	const LoxCallable* function = callee.callable.get();

	if (arguments.size() != function->arity())
	{
		throw RuntimeError(std::move(expr.paren), "Expected " + std::to_string(function->arity()) + " arguments but got " + std::to_string(arguments.size()) + ".");
	}

	RET(function->call(this, arguments));
}

void Interpreter::visitGroupingExpr(Expr::Grouping& expr, void* returnValue)
{
	RET(evaluate(expr.expression));
}
void Interpreter::visitLiteralExpr(Expr::Literal& expr, void* returnValue)
{
	RET(expr.value);
}

void Interpreter::visitLogicalExpr(Expr::Logical& expr, void* returnValue)
{
	const Object left = evaluate(expr.left);

	if (expr.op.type == OR)
	{
		// OR
		if (IsTruthy(left)) { RET(left); }
	}
	else
	{
		// AND
		if (!IsTruthy(left)) { RET(left); }
	}

	RET(evaluate(expr.right));
}

void Interpreter::visitUnaryExpr(Expr::Unary& expr, void* returnValue)
{
	const Object right = evaluate(expr.right);
	switch (expr.op.type)
	{
	case MINUS:
		checkNumberOperand(expr.op, right);
		RET(-ToNumber(right));
	case BANG:
		RET(!IsTruthy(right));
	}

	__debugbreak(); // returning null is not really a thing
	//returnValue = nullptr;
	//RET( LiteralValue::Nil();
}
void Interpreter::visitVariableExpr(Expr::Variable& expr, void* returnValue)
{
	RET(m_environment->get(expr.name));
}
void Interpreter::visitAssignExpr(Expr::Assign& expr, void* returnValue)
{
	const Object value = evaluate(expr.value);
	m_environment->assign(expr.name, value);
	RET(value);
}
#undef RET

// Statements
void Interpreter::visitBlockStmt(Stmt::Block& stmt)
{
	//executeBlock(stmt.statements, std::make_unique<Environment>(m_environment));
	executeBlock(stmt.statements, new Environment(m_environment));
}
void Interpreter::visitExpressionStmt(Stmt::Expression& stmt)
{
	evaluate(stmt.expression);
}

void Interpreter::visitFunctionStmt(Stmt::Function& stmt)
{
	//const LoxFunction function(stmt);

	//m_environment->define(stmt.name.lexeme, std::make_unique<LoxCallable>(function));
	m_environment->define(stmt.name.lexeme, std::unique_ptr<LoxCallable>(std::make_unique<LoxFunction>(stmt, m_environment)));
}

void Interpreter::visitIfStmt(Stmt::If& stmt)
{
	if (IsTruthy(evaluate(stmt.condition)))
	{
		execute(stmt.thenBranch);
	}
	else if (stmt.elseBranch != nullptr)
	{
		execute(stmt.elseBranch);
	}
}

void Interpreter::visitPrintStmt(Stmt::Print& stmt)
{
	const Object value = evaluate(stmt.expression);
	std::cout << value.AsString() << std::endl;
}

void Interpreter::visitReturnStmt(Stmt::Return& stmt)
{
	Object value = Object::Nil();
	if (stmt.value != nullptr) { value = evaluate(stmt.value); }

	throw Return(value);
}

void Interpreter::visitVarStmt(Stmt::Var& stmt)
{
	Object value = Object::Nil();
	if (stmt.initializer != nullptr)
	{
		value = evaluate(stmt.initializer);
	}
	m_environment->define(stmt.name.lexeme, value);
}

void Interpreter::visitWhileStmt(Stmt::While& stmt)
{
	while (IsTruthy(evaluate(stmt.condition)))
	{
		execute(stmt.body);
	}
}


void Interpreter::checkNumberOperand(const Token& op, const Object& operand)
{
	if (operand.type == Object::Type::NUMBER) return;
	throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& op, const Object& left, const Object& right)
{
	if (IsNumber(left) && IsNumber(right)) return;
	throw(RuntimeError(op, "Operands must be numbers."));
}
