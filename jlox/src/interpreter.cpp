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
	object_t call(Interpreter*, const std::vector<object_t>&) const override
	{
		return static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
	size_t arity() const override { return 0; }
};


// helper functions
void CheckNumberOperand(const Token& op, const object_t& operand)
{ if (!is<double>(operand)) { throw RuntimeError(op, "Operand must be a number."); } }
void CheckNumberOperands(const Token& op, const object_t& left, const object_t& right)
{ if (!is<double>(left) || !is<double>(right)) { throw RuntimeError(op, "Operands must be numbers."); } }

// constructor
Interpreter::Interpreter(): m_environment(&globals)
{
	m_clockFunction = std::make_unique<ClockFunction>();
	globals.define("clock", object_t(m_clockFunction.get()));
}

// where the magic starts
void Interpreter::interpret(const std::vector<std::shared_ptr<Stmt>>& statements)
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
		methods.insert_or_assign(method->name.lexeme, LoxFunction(method, m_environment, method->name.lexeme == "init"));
	}

	if (superclass != nullptr)
	{
		// nts free the created environment?
		m_environment = m_environment->getEnclosing();
	}

	// nts: leak
	m_environment->assign(stmt.name, object_t(new LoxClass(stmt.name.lexeme, superclass, std::move(methods))));
}

void Interpreter::visitExpressionStmt(Stmt::Expression& stmt)
{
	evaluate(stmt.expression);
}

void Interpreter::visitFunctionStmt(Stmt::Function& stmt)
{
	// nts: leak
	LoxCallable* lc = new LoxFunction(std::dynamic_pointer_cast<Stmt::Function>(stmt.getShared()), m_environment, false);

	m_environment->define(stmt.name.lexeme, lc);
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
	const object_t value = evaluate(stmt.expression.get());
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
		execute(stmt.body);
	}
}


// Expressions
object_t Interpreter::visitAssignExpr(Expr::Assign& expr)
{
	object_t value = evaluate(expr.value);

	if (const auto it = locals.find(expr.getShared()); it != locals.end())
	{
		m_environment->assignAt(it->second, expr.name, value);
	}
	else
	{
		globals.assign(expr.name, value);
	}

	return value;
}

object_t Interpreter::visitBinaryExpr(Expr::Binary& expr)
{
	const object_t left = evaluate(expr.left);
	const object_t right = evaluate(expr.right);

	switch (expr.op.type)
	{
		// comparisons
	case GREATER:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) > as<double>(right);
	case GREATER_EQUAL:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) >= as<double>(right);
	case LESS:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) < as<double>(right);
	case LESS_EQUAL:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) <= as<double>(right);
	case BANG_EQUAL:
		return!IsEqual(left, right);
	case EQUAL_EQUAL:
		return IsEqual(left, right);

		// arithmetic
	case MINUS:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) - as<double>(right);
	case PLUS:
		if (is<double>(left) && is<double>(right))
		{ return as<double>(left) + as<double>(right); }
		if (is<std::string>(left) && is<std::string>(right))
		{ return as<std::string>(left) + as<std::string>(right); }
		throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
	case SLASH:
		CheckNumberOperands(expr.op, left, right);
		return as<double>(left) / as<double>(right);
	case STAR:
		CheckNumberOperands(expr.op, left, right);
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
	LoxFunction func(nullptr, nullptr, false); // for if the returnvalue callee turns out to be a function instance
	// nts: make this more robust
	if (is<LoxCallable*>(callee)) { callable = as<LoxCallable*>(callee); }
	else if (is<LoxClass*>(callee)) { callable = as<LoxClass*>(callee); }
	else if (is<LoxFunction>(callee)) { callable = new LoxFunction(as<LoxFunction>(callee)); }

	if (callable == nullptr)
	{ throw RuntimeError(expr.paren, "Can only call functions and classes."); }

	if (arguments.size() != callable->arity())
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

object_t Interpreter::visitGroupingExpr(Expr::Grouping& expr)
{ return evaluate(expr.expression); }

object_t Interpreter::visitLiteralExpr(Expr::Literal& expr)
{ return expr.value; }

object_t Interpreter::visitLogicalExpr(Expr::Logical& expr)
{
	object_t left = evaluate(expr.left);

	if (expr.op.type == OR)
	{
		if (IsTruthy(left)) { return left; }
	}
	else
	{
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
	const size_t distance = locals.at(expr.getShared());
	const LoxClass* const superclass = as<LoxClass*>(m_environment->getAt(distance, "super"));

	LoxInstance* instance = as<LoxInstance*>(m_environment->getAt(distance - 1, "this"));

	const std::optional<LoxFunction> method = superclass->findMethod(expr.method.lexeme);

	if (!method)
	{ throw RuntimeError(expr.method, "Undefined property '" + expr.method.lexeme + "'."); }


	return {method->bind(instance)};
}

object_t Interpreter::visitThisExpr(Expr::This& expr)
{ return lookUpVariable(expr.keyword, expr.getShared()); }

object_t Interpreter::visitUnaryExpr(Expr::Unary& expr)
{
	const object_t right = evaluate(expr.right);
	switch (expr.op.type)
	{
	case MINUS:
		CheckNumberOperand(expr.op, right);
		return -as<double>(right);
	case BANG:
		return !IsTruthy(right);
	default: return {};
	}
}

object_t Interpreter::visitVariableExpr(Expr::Variable& expr)
{ return lookUpVariable(expr.name, expr.getShared()); }


void Interpreter::resolve(std::shared_ptr<Expr> expr, size_t depth)
{ locals.emplace(std::move(expr), depth); }

object_t Interpreter::lookUpVariable(const Token& name, const std::shared_ptr<Expr>& expr)
{
	if (const auto it = locals.find(expr); it != locals.end())
	{ return m_environment->getAt(it->second, name.lexeme); }
	return globals.get(name);
}


void Interpreter::executeBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, Environment* environment)
{
	Environment* previous = m_environment;
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





