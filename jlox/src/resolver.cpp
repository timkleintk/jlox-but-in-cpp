#include "resolver.h"

#include "lox.h"

Resolver::Resolver(Interpreter& interpreter): m_interpreter(interpreter)
{}

void Resolver::resolve(const std::vector<Stmt*>& stmts)
{
	for (const auto& stmt : stmts)
	{
		resolve(stmt);
	}
}

object_t Resolver::visitAssignExpr(Expr::Assign& expr)
{
	resolve(expr.value);
	resolveLocal(&expr, expr.name);
	return {};
}

object_t Resolver::visitBinaryExpr(Expr::Binary& expr)
{
	resolve(expr.left);
	resolve(expr.right);
	return {};
}

object_t Resolver::visitCallExpr(Expr::Call& expr)
{
	resolve(expr.callee);

	for (const auto& argument : expr.arguments)
	{
		resolve(argument);
	}
	return {};
}

object_t Resolver::visitGetExpr(Expr::Get& expr)
{
	resolve(expr.object);
	return {};
}

object_t Resolver::visitGroupingExpr(Expr::Grouping& expr)
{
	resolve(expr.expression);
	return {};
}

object_t Resolver::visitLiteralExpr(Expr::Literal&)
{
	return {};
}

object_t Resolver::visitLogicalExpr(Expr::Logical& expr)
{
	resolve(expr.left);
	resolve(expr.right);
	return {};
}

object_t Resolver::visitSetExpr(Expr::Set& expr)
{
	resolve(expr.value);
	resolve(expr.object);
	return {};
}

object_t Resolver::visitSuperExpr(Expr::Super& expr)
{
	if (m_currentClass == ClassType::NONE)
	{
		Lox::Error(expr.keyword, "Can't use 'super' outside of a class.");
	}
	else if (m_currentClass != ClassType::SUBCLASS)
	{
		Lox::Error(expr.keyword, "Can't use 'super' in a class with no superclass.");
	}

	resolveLocal(&expr, expr.keyword);
	return {};
}

object_t Resolver::visitThisExpr(Expr::This& expr)
{
	if (m_currentClass == ClassType::NONE)
	{
		Lox::Error(expr.keyword, "Can't use 'this' outside of a class.");
	}

	resolveLocal(&expr, expr.keyword);
	return {};
}

object_t Resolver::visitUnaryExpr(Expr::Unary& expr)
{
	resolve(expr.right);
	return {};
}

object_t Resolver::visitVariableExpr(Expr::Variable& expr)
{
	if (!m_scopes.empty() &&
		m_scopes.top().contains(expr.name.lexeme) &&
		m_scopes.top().at(expr.name.lexeme) == false)
	{
		Lox::Error(expr.name, "Can't read local variable in its own initializer.");
	}

	resolveLocal(&expr, expr.name);
	return {};
}

void Resolver::visitBlockStmt(Stmt::Block& stmt)
{
	beginScope();
	resolve(stmt.statements);
	endScope();
}

void Resolver::visitClassStmt(Stmt::Class& stmt)
{
	ClassType enclosingClass = m_currentClass;
	m_currentClass = ClassType::CLASS;

	declare(stmt.name);
	define(stmt.name);

	if (stmt.superclass != nullptr)
	{
		m_currentClass = ClassType::SUBCLASS;
		if (stmt.name.lexeme == stmt.superclass->name.lexeme)
		{
			Lox::Error(stmt.superclass->name, "A class can't inherit from itself.");
		}
		else
		{
			resolve(stmt.superclass);

			beginScope();
			m_scopes.top().insert_or_assign("super", true);
		}
	}

	beginScope();
	m_scopes.top().insert_or_assign("this", true);

	for (const Stmt::Function& method : stmt.methods)
	{
		FunctionType declaration = FunctionType::METHOD;
		if (method.name.lexeme == "init") { declaration = FunctionType::INITIALIZER; }
		resolveFunction(method, declaration);
	}

	endScope();

	if (stmt.superclass != nullptr) { endScope(); }

	m_currentClass = enclosingClass;
}

void Resolver::visitExpressionStmt(Stmt::Expression& stmt)
{
	resolve(stmt.expression);
}

void Resolver::visitFunctionStmt(Stmt::Function& stmt)
{
	declare(stmt.name);
	define(stmt.name);

	resolveFunction(stmt, FunctionType::FUNCTION);
}

void Resolver::visitIfStmt(Stmt::If& stmt)
{
	resolve(stmt.condition);
	resolve(stmt.thenBranch);
	if (stmt.elseBranch != nullptr)
	{ resolve(stmt.elseBranch); }
}

void Resolver::visitPrintStmt(Stmt::Print& stmt)
{
	resolve(stmt.expression);
}

void Resolver::visitReturnStmt(Stmt::Return& stmt)
{
	if (m_currentFunction == FunctionType::NONE)
	{ Lox::Error(stmt.keyword, "Can't return from top-level code."); }
	if (stmt.value != nullptr)
	{
		if (m_currentFunction == FunctionType::INITIALIZER)
		{
			Lox::Error(stmt.keyword, "Can't return a value from an initializer.");
		}

		resolve(stmt.value);
	}
}

void Resolver::visitVarStmt(Stmt::Var& stmt)
{
	declare(stmt.name);
	if (stmt.initializer != nullptr)
	{
		resolve(stmt.initializer);
	}
	define(stmt.name);
}

void Resolver::visitWhileStmt(Stmt::While& stmt)
{
	resolve(stmt.condition);
	resolve(stmt.body);
}

void Resolver::resolve(Stmt* stmt) { stmt->accept(this); }

void Resolver::resolve(Expr* expr) { expr->accept(this); }

void Resolver::resolveFunction(const Stmt::Function& function, const FunctionType type)
{
	const FunctionType enclosingFunction = m_currentFunction;
	m_currentFunction = type;

	beginScope();
	for (auto& param : function.params)
	{
		declare(param);
		define(param);
	}
	resolve(function.body);
	endScope();
	m_currentFunction = enclosingFunction;
}

void Resolver::beginScope()
{
	m_scopes.emplace();
}

void Resolver::endScope()
{
	m_scopes.pop();
}

void Resolver::declare(const Token& name)
{
	if (m_scopes.empty()) return;

	auto& scope = m_scopes.top();
	if (scope.contains(name.lexeme))
	{ Lox::Error(name, "Already a variable with this name in this scope."); }
	scope.insert_or_assign(name.lexeme, false);
}

void Resolver::define(const Token& name)
{
	if (m_scopes.empty()) return;
	//m_scopes.top().at(name.lexeme) = true;
	// I chose insert_or_assign, because it is closer to java's put
	m_scopes.top().insert_or_assign(name.lexeme, true);
}

void Resolver::resolveLocal(const Expr* expr, const Token& name) const
{
	for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; i--)
	{
		if (m_scopes._Get_container()[i].contains(name.lexeme))
		{
			m_interpreter.resolve(expr, static_cast<int>(m_scopes.size()) - 1 - i);
			return;
		}
	}
}
