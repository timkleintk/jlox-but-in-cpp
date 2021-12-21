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

void Resolver::visitAssignExpr(Expr::Assign& expr, void*)
{
	resolve(expr.value);
	resolveLocal(&expr, expr.name);
}

void Resolver::visitBinaryExpr(Expr::Binary& expr, void*)
{
	resolve(expr.left);
	resolve(expr.right);
}

void Resolver::visitCallExpr(Expr::Call& expr, void*)
{
	resolve(expr.callee);

	for (const auto& argument : expr.arguments)
	{
		resolve(argument);
	}
}

void Resolver::visitGetExpr(Expr::Get& expr, void* )
{
	resolve(expr.object);
}

void Resolver::visitGroupingExpr(Expr::Grouping& expr, void*)
{
	resolve(expr.expression);
}

void Resolver::visitLiteralExpr(Expr::Literal&, void*)
{}

void Resolver::visitLogicalExpr(Expr::Logical& expr, void*)
{
	resolve(expr.left);
	resolve(expr.right);
}

void Resolver::visitSetExpr(Expr::Set& expr, void* )
{
	resolve(expr.value);
	resolve(expr.object);
}


void Resolver::visitUnaryExpr(Expr::Unary& expr, void*)
{
	resolve(expr.right);
}

void Resolver::visitVariableExpr(Expr::Variable& expr, void*)
{
	if (!m_scopes.empty() && 
		m_scopes.top().contains(expr.name.lexeme) && 
		m_scopes.top().at(expr.name.lexeme) == false)
	{
		// nts: change to something like "illegal use of undefined variable
		Lox::Error(expr.name, "Can't read local variable in its own initializer.");
	}

	resolveLocal(&expr, expr.name);
}

void Resolver::visitBlockStmt(Stmt::Block& stmt)
{
	beginScope();
	resolve(stmt.statements);
	endScope();
}

void Resolver::visitClassStmt(Stmt::Class& stmt)
{
	declare(stmt.name);
	define(stmt.name);
	for (const Stmt::Function& method : stmt.methods)
	{
		const FunctionType declaration = FunctionType::METHOD;
		resolveFunction(method, declaration);
	}
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
	{ resolve(stmt.value); }
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

// nts: can this be moved to the visitor base class?
void Resolver::resolve(Stmt* stmt)
{
	stmt->accept(this);
}

void Resolver::resolve(Expr* expr)
{
	expr->accept(this, nullptr);
}

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
	//m_scopes.emplace();
	m_scopes.push(std::unordered_map<std::string, bool>());
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
	//m_scopes.top().insert_or_assign(name.lexeme, true);
	m_scopes.top().at(name.lexeme) = true;
	//m_scopes.top().insert({name.lexeme, true});
}

void Resolver::resolveLocal(Expr* expr, const Token& name) const
{
	// nts: look at the types in this for loop
	for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; i--)
	{
		if (m_scopes._Get_container()[i].contains(name.lexeme))
		{
			m_interpreter.resolve(expr, static_cast<int>(m_scopes.size()) - 1 - i);
			return;
		}
	}
}
