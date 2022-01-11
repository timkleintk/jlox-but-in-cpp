#include "interpreter.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include "lox.h"
#include "loxClass.h"
#include "loxFunction.h"
#include "loxInstance.h"
#include "return.h"
#include "RuntimeError.h"


class ClockFunction final : public LoxCallable
{
public:
	object_t call(Interpreter*, std::vector<object_t>) const override
	{
		return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
	[[nodiscard]] int arity() const override { return 0; }
};

Interpreter::Interpreter(): m_environment(&globals)
{
	globals.define("clock", object_t(static_cast<LoxCallable*>(new ClockFunction())));
}

Interpreter::~Interpreter()
{
	assert(m_environment == &globals);
}

object_t Interpreter::evaluate(Expr* expr) { return expr->accept(this); }

void Interpreter::execute(Stmt* stmt) { stmt->accept(this); }

#define EqualCheck(T) if (is<T>(a)) return as<T>(a) == as<T>(b) 
bool IsEqual(const object_t& a, const object_t& b)
{
	if (!a.has_value() && !b.has_value()) { return true; } // if both nil
	if (!a.has_value()) { return false; }                  // if only one operand is nil
	if (a.type() != b.type()) { return false; }            // a and b should be the same type

	EqualCheck(bool);
	EqualCheck(std::string);
	EqualCheck(double);
	EqualCheck(LoxClass*);
	EqualCheck(LoxInstance*);
	EqualCheck(LoxCallable*);
	EqualCheck(LoxFunction);
	

	// bug: not fully implemented
	printf("Warning: tried to compare two values of unknown type. Known types: nil, bool, string, number, class.\n");

	return false;
}
#undef EqualCheck

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts, Environment* environment)
{
	Environment* previous = m_environment;
	m_environment = environment;

	try
	{
		for (const auto& stmt : stmts)
		{
			execute(stmt.get());
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
		for (const auto& statement : statements)
		{
			execute(statement);
		}
	}
	catch (RuntimeError& error)
	{
		Lox::runtimeError(error);
	}
}

// helper functions
void checkNumberOperand(const Token& op, const object_t& operand)
{ if (!is<double>(operand)) { throw RuntimeError(op, "Operand must be a number."); } }

void checkNumberOperands(const Token& op, const object_t& left, const object_t& right)
{ if (!is<double>(left) || !is<double>(right)) { throw RuntimeError(op, "Operands must be numbers."); } }

// Expressions
object_t Interpreter::visitBinaryExpr(Expr::Binary& expr)
{
	const auto left = evaluate(expr.left);
	const auto right = evaluate(expr.right);

	switch (expr.op.type)
	{
		// comparisons
	case GREATER:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) > as<double>(right);
	case GREATER_EQUAL:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) >= as<double>(right);
	case LESS:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) < as<double>(right);
	case LESS_EQUAL:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) <= as<double>(right);
	case BANG_EQUAL:
		//checkNumberOperands(expr.op, left, right);
		return!IsEqual(left, right);
	case EQUAL_EQUAL:
		//checkNumberOperands(expr.op, left, right);
		return IsEqual(left, right);

		// arithmetic
	case MINUS:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) - as<double>(right);
	case PLUS:
		if (is<double>(left) && is<double>(right))
		{ return as<double>(left) + as<double>(right); }
		if (is<std::string>(left) && is<std::string>(right))
		{ return as<std::string>(left) + as<std::string>(right); }
		throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
	case SLASH:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) / as<double>(right);
	case STAR:
		checkNumberOperands(expr.op, left, right);
		return as<double>(left) * as<double>(right);
	default:
		throw RuntimeError(expr.op, "Unknown binary operator.");
	}
}

object_t Interpreter::visitCallExpr(Expr::Call& expr)
{
	const object_t callee = evaluate(expr.callee);

	std::vector<object_t> arguments;
	for (const auto& arg : expr.arguments)
	{
		arguments.push_back(evaluate(arg));
	}

	LoxCallable* callable = nullptr;
	// nts: make this more robust
	if (is<LoxCallable*>(callee)) { callable = as<LoxCallable*>(callee); }
	//else if (is<LoxFunction*>(callee)) { callable = as<LoxFunction*>(callee); }
	else if (is<LoxClass*>(callee)) { callable = as<LoxClass*>(callee); }
												// nts: leak
	else if (is<LoxFunction>(callee)) { callable = new LoxFunction(as<LoxFunction>(callee)); }

	if (callable == nullptr)
	{ throw RuntimeError(expr.paren, "Can only call functions and classes."); }

	if (static_cast<int>(arguments.size()) != callable->arity())
	{
		throw RuntimeError(std::move(expr.paren), "Expected " + std::to_string(callable->arity()) + " arguments but got " + std::to_string(arguments.size()) + ".");
	}

	return callable->call(this, std::move(arguments));
}

object_t Interpreter::visitGetExpr(Expr::Get& expr)
{
	const object_t object = evaluate(expr.object);
	if (is<LoxInstance*>(object))
	{
		return as<LoxInstance*>(object)->get(expr.name);
	}

	throw RuntimeError(expr.name, "Only instances have properties.");
}

object_t Interpreter::visitGroupingExpr(Expr::Grouping& expr) { return evaluate(expr.expression); }

object_t Interpreter::visitLiteralExpr(Expr::Literal& expr) { return expr.value; }

object_t Interpreter::visitLogicalExpr(Expr::Logical& expr)
{
	object_t left = evaluate(expr.left);

	if (expr.op.type == OR)
	{
		// OR
		if (IsTruthy(left)) { return left; }
	}
	else
	{
		// AND
		if (!IsTruthy(left)) { return left; }
	}

	return evaluate(expr.right);
}

object_t Interpreter::visitSetExpr(Expr::Set& expr)
{
	const object_t object = evaluate(expr.object);

	if (!is<LoxInstance*>(object))
	{ throw RuntimeError(expr.name, "Only instances have fields."); }

	object_t value = evaluate(expr.value);
	as<LoxInstance*>(object)->set(expr.name, value);
	return value;
}

object_t Interpreter::visitSuperExpr(Expr::Super& expr)
{
	const int distance = locals.at(&expr);
	const LoxClass* const superclass = as<LoxClass*>(m_environment->getAt(distance, "super"));

	LoxInstance* instance = as<LoxInstance*>(m_environment->getAt(distance - 1, "this"));

	const object_t method = superclass->findMethod(expr.method.lexeme);

	if (isNull(method))
	{ throw RuntimeError(expr.method, "Undefined property '" + expr.method.lexeme + "'."); }

	return {as<LoxFunction>(method).bind(instance)};
}

object_t Interpreter::visitThisExpr(Expr::This& expr) { return lookUpVariable(expr.keyword, &expr); }

object_t Interpreter::visitUnaryExpr(Expr::Unary& expr)
{
	const object_t right = evaluate(expr.right);
	switch (expr.op.type)
	{
	case MINUS:
		checkNumberOperand(expr.op, right);
		return -as<double>(right);
	case BANG:
		return !IsTruthy(right);
	default: return {};
	}

}

object_t Interpreter::visitVariableExpr(Expr::Variable& expr) { return lookUpVariable(expr.name, &expr); }

object_t Interpreter::visitAssignExpr(Expr::Assign& expr)
{
	object_t value = evaluate(expr.value);

	const auto it = locals.find(&expr);
	if (it != locals.end())
	{
		m_environment->assignAt(it->second, expr.name, value);
	}
	else
	{
		globals.assign(expr.name, value);
	}

	return value;
}

// Statements
void Interpreter::visitBlockStmt(Stmt::Block& stmt)
{
	executeBlock(stmt.statements, new Environment(m_environment));
}

void Interpreter::visitClassStmt(Stmt::Class& stmt)
{
	LoxClass* superclass = nullptr;

	if (stmt.superclass != nullptr)
	{
		const object_t lc = evaluate(stmt.superclass);
		if (!is<LoxClass*>(lc))
		{
			throw RuntimeError(stmt.superclass->name, "Superclass must be a class.");
		}
		superclass = as<LoxClass*>(lc);
	}

	m_environment->define(stmt.name.lexeme, {});

	if (stmt.superclass != nullptr)
	{
		m_environment = new Environment(m_environment);
		m_environment->define("super", superclass);
	}

	std::unordered_map<std::string, LoxFunction> methods;
	for (const auto& method : stmt.methods)
	{
		LoxFunction function(method.get(), m_environment, method->name.lexeme == "init");
		methods.insert_or_assign(method->name.lexeme, function);
	}

	if (superclass != nullptr)
	{
		// nts free the created environment?
		m_environment = m_environment->getEnclosing();
	}

	// nts: leak
	m_environment->assign(stmt.name, object_t(new LoxClass(stmt.name.lexeme, superclass, std::move(methods))));
}

void Interpreter::visitExpressionStmt(Stmt::Expression& stmt) { evaluate(stmt.expression); }

void Interpreter::visitFunctionStmt(Stmt::Function& stmt)
{
	// nts: leak
	m_environment->define(stmt.name.lexeme, object_t(static_cast<LoxCallable*>(new LoxFunction(&stmt, m_environment, false))));
}

void Interpreter::visitIfStmt(Stmt::If& stmt)
{
	if (IsTruthy(evaluate(stmt.condition)))
	{
		execute(stmt.thenBranch.get());
	}
	else if (stmt.elseBranch != nullptr)
	{
		execute(stmt.elseBranch.get());
	}
}

void Interpreter::visitPrintStmt(Stmt::Print& stmt)
{
	const object_t value = evaluate(stmt.expression);
	std::cout << toString(value) << std::endl;
}

void Interpreter::visitReturnStmt(Stmt::Return& stmt)
{
	object_t value = {};
	if (stmt.value != nullptr) { value = evaluate(stmt.value); }

	throw Return(value);
}

void Interpreter::visitVarStmt(Stmt::Var& stmt)
{
	object_t value = {};
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
		execute(stmt.body.get());
	}
}

void Interpreter::resolve(const Expr* expr, int depth)
{
	locals.insert_or_assign(expr, depth);
}

object_t Interpreter::lookUpVariable(const Token& name, const Expr* expr)
{
	if (const auto it = locals.find(expr); it != locals.end())
	{ return m_environment->getAt(it->second, name.lexeme); }
	return globals.get(name);
}
