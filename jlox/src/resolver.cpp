#include "resolver.h"

#include "lox.h"

Resolver::Resolver(Interpreter& interpreter) : m_interpreter(interpreter)
{}


object_t Resolver::visitAssignExpr(Expr::Assign& expr)
{
	// tell the interpreter where to find the assignment target
	resolveLocal(expr.getShared(), expr.name);

	// descend the syntax tree further
	resolve(expr.value);

	return {};
}


// expressions -----------------------------------------------------

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
		Lox::Error(expr.keyword, "Cannot use 'super' outside of a class.");
	}
	else if (m_currentClass != ClassType::SUBCLASS)
	{
		Lox::Error(expr.keyword, "Cannot use 'super' in a class with no superclass.");
	}

	// tell the interpreter where to find the baseclass method
	resolveLocal(expr.getShared(), expr.keyword);
	return {};
}

object_t Resolver::visitThisExpr(Expr::This& expr)
{
	if (m_currentClass == ClassType::NONE)
	{
		Lox::Error(expr.keyword, "Cannot use 'this' outside of a class.");
	}

	// tell the interpreter where to find the this pointer
	resolveLocal(expr.getShared(), expr.keyword);
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
		Lox::Error(expr.name, "Cannot read local variable in its own initializer.");
	}

	// tell the interpreter where to find the variable
	resolveLocal(expr.getShared(), expr.name);
	return {};
}


// statements ------------------------------------------------------

void Resolver::visitBlockStmt(Stmt::Block& stmt)
{
	beginScope();
	resolve(stmt.statements);
	endScope();
}

void Resolver::visitClassStmt(Stmt::Class& stmt)
{
	const ClassType enclosingClass = m_currentClass;
	m_currentClass = ClassType::CLASS;

	declare(stmt.name);
	define(stmt.name);

	if (stmt.superclass != nullptr)
	{
		m_currentClass = ClassType::SUBCLASS;
		// TODO: check for own name down the whole inheritance chain
		if (stmt.name.lexeme == stmt.superclass->name.lexeme)
		{
			Lox::Error(stmt.superclass->name, "A class can't inherit from itself.");
			beginScope();
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

	for (const auto& method : stmt.methods)
	{
		FunctionType declaration = FunctionType::METHOD;
		if (method->name.lexeme == "init") { declaration = FunctionType::INITIALIZER; }
		resolveFunction(*method, declaration);
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
	{
		resolve(stmt.elseBranch);
	}
}

void Resolver::visitPrintStmt(Stmt::Print& stmt)
{
	resolve(stmt.expression);
}

void Resolver::visitReturnStmt(Stmt::Return& stmt)
{
	if (m_currentFunction == FunctionType::NONE)
	{
		Lox::Error(stmt.keyword, "Cannot return from top-level code.");
	}

	if (stmt.value != nullptr)
	{
		if (m_currentFunction == FunctionType::INITIALIZER)
		{
			Lox::Error(stmt.keyword, "Cannot return a value from an initializer.");
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


// helper functions ------------------------------------------------

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
	{
		Lox::Error(name, "Variable with this name already declared in this scope.");
	}

	scope.insert_or_assign(name.lexeme, false);
}

void Resolver::define(const Token& name)
{
	if (m_scopes.empty()) return;

	m_scopes.top().insert_or_assign(name.lexeme, true);
}

void Resolver::resolveLocal(std::shared_ptr<Expr> expr, const Token& name) const
{
	for (size_t i = m_scopes.size(); i --> 0; ) // loops from m_scopes.size() - 1 to 0
	{
		// reverse loop over the scope stack and find the identifier
		if (m_scopes._Get_container()[i].contains(name.lexeme))
		{
			// store the number of steps in the interpreter
			m_interpreter.resolve(std::move(expr), m_scopes.size() - 1 - i);
			return;
		}
	}
}
