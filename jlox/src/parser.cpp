#include "parser.h"


#include <cassert>
#include <cstdarg>

#include "lox.h"

ParseError Error(const Token& token, const std::string& message)
{
	Lox::Error(token, message);
	return {};
};

std::vector<Stmt*> Parser::parse()
{
	std::vector<Stmt*> statements;
	while (!isAtEnd())
	{
		// nts: leak
		statements.emplace_back(declaration().release());
	}
	return statements;
}


Expr* Parser::expression()
{
	return assignment();
}

std::unique_ptr<Stmt> Parser::declaration()
{
	try
	{
		if (match(CLASS)) return classDeclaration();
		if (match(FUN)) return function("function");
		if (match(VAR)) return varDeclaration();
		return statement();
	}
	catch (ParseError&)
	{
		synchronize();
		return nullptr;
	}
}

std::unique_ptr<Stmt> Parser::classDeclaration()
{
	const Token name = consume(IDENTIFIER, "Expect class name.");

	Expr::Variable* superclass = nullptr;

	if (match(LESS))
	{
		consume(IDENTIFIER, "Expect superclass name.");
		superclass = new Expr::Variable(previous());
	}

	consume(LEFT_BRACE, "Expect '{' before class body.");

	std::vector<std::unique_ptr<Stmt::Function>> methods;
	while (!check(RIGHT_BRACE) && !isAtEnd())
	{
		// nts: this could probably be improved
		// nts: ?!?!?!?!??!!?!??!?!?!
		//std::unique_ptr<Stmt::Function> p = function("method");

		//methods.push_back(*function("method"));
		methods.emplace_back(function("method"));
	}

	consume(RIGHT_BRACE, "Expect '}' after class body.");

	//return new Stmt::Class(name, superclass, methods);
	return std::make_unique<Stmt::Class>(name, superclass, std::move(methods));
}

std::unique_ptr<Stmt> Parser::statement()
{
	if (match(FOR)) return forStatement();
	if (match(IF)) return ifStatement();
	if (match(PRINT)) return printStatement();
	if (match(RETURN)) return returnStatement();
	if (match(WHILE)) return whileStatement();
	//if (match(LEFT_BRACE)) return new Stmt::Block(block());
	if (match(LEFT_BRACE)) return std::make_unique<Stmt::Block>(block());
	return expressionStatement();
}

std::unique_ptr<Stmt> Parser::forStatement()
{
	// for (initializer; condition; increment;) body

	consume(LEFT_PAREN, "Expect '(' after 'for'");

	// initializer
	//Stmt* initializer;
	std::unique_ptr<Stmt> initializer;
	if (match(SEMICOLON))
	{
		initializer = nullptr;
	}
	else if (match(VAR))
	{
		initializer = varDeclaration();
	}
	else
	{
		initializer = expressionStatement();
	}

	// condition
	Expr* condition = nullptr;
	if (!check(SEMICOLON))
	{
		condition = expression();
	}
	consume(SEMICOLON, "Expect ';' after loop condition.");

	// increment
	Expr* increment = nullptr;
	//std::unique_ptr<Expr> increment = nullptr;
	if (!check(RIGHT_PAREN))
	{
		increment = expression();
		//increment.reset(expression());
	}

	consume(RIGHT_PAREN, "Expect ')' after for clauses.");

	// body
	//Stmt* body = statement().release();
	std::unique_ptr<Stmt> body = statement();

	// desugaring --------------------------------------------------

	// add increment at the end of loop body
	if (increment != nullptr)
	//{ body = new Stmt::Block({body,new Stmt::Expression(increment)}); }
	// nts: using std::move(body) as one of the arguments body should be nullptr before we actually make the reset call
	{
		std::vector<std::unique_ptr<Stmt>> tt;

		tt.push_back(std::move(body));
		tt.push_back(std::make_unique<Stmt::Expression>(increment));

		body.reset(new Stmt::Block(std::move(tt)));
	}

// make a while loop out of the condition and the body
	if (condition == nullptr)
	{ condition = new Expr::Literal(true); }
	//body = new Stmt::While(condition, body);
	body.reset(new Stmt::While(condition, std::move(body)));

	// add initializer before the loop
	if (initializer != nullptr)
	//{ body = new Stmt::Block({initializer, body}); }
	{
		// nts: can I clean this up?
		std::vector<std::unique_ptr<Stmt>> t;
		t.push_back(std::move(initializer));
		t.push_back(std::move(body));

		body.reset(new Stmt::Block(std::move(t)));
	}

//return std::unique_ptr<Stmt>(body);
	return body;
}

std::unique_ptr<Stmt> Parser::ifStatement()
{
	consume(LEFT_PAREN, "Expect '(' aftr 'if',");
	Expr* condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after if condition.");

	//Stmt* thenBranch = statement().release();
	//Stmt* elseBranch = nullptr;

	std::unique_ptr<Stmt> thenBranch = statement();
	std::unique_ptr<Stmt> elseBranch = nullptr;

	if (match(ELSE))
	{
		elseBranch = statement();
	}

	//return new Stmt::If(condition, thenBranch, elseBranch);
	return std::make_unique<Stmt::If>(condition, std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::printStatement()
{
	Expr* value = expression();
	consume(SEMICOLON, "Expect ';' after value");
	//return new Stmt::Print(value);
	return std::make_unique<Stmt::Print>(value);
}

std::unique_ptr<Stmt> Parser::returnStatement()
{
	const Token keyword = previous();
	Expr* value = nullptr;
	if (!check(SEMICOLON))
	{
		value = expression();
	}

	consume(SEMICOLON, "Expect ';' after return value.");
	//return new Stmt::Return(keyword, value);
	return std::make_unique<Stmt::Return>(keyword, value);
}

std::unique_ptr<Stmt> Parser::varDeclaration()
{
	const Token name = consume(IDENTIFIER, "Expect variable name.");

	Expr* initializer = nullptr;
	if (match(EQUAL)) initializer = expression();
	consume(SEMICOLON, "Expect ';' after variable declaration.");
	//return new Stmt::Var(name, initializer);
	return std::make_unique<Stmt::Var>(name, initializer);
}

std::unique_ptr<Stmt> Parser::whileStatement()
{
	consume(LEFT_PAREN, "Expect '(' after 'while'.");
	Expr* condition = expression();
	consume(RIGHT_PAREN, "Expect '(' after condition.");
	//Stmt* body = statement().release();
	std::unique_ptr<Stmt> body = statement();

	//return new Stmt::While(condition, body);
	return std::make_unique<Stmt::While>(condition, std::move(body));
}

std::unique_ptr<Stmt> Parser::expressionStatement()
{
	Expr* expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	//return new Stmt::Expression(expr);
	return std::make_unique<Stmt::Expression>(expr);
}

//Stmt::Function Parser::function(const std::string& kind)
std::unique_ptr<Stmt::Function> Parser::function(const std::string& kind)
{
	const Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
	consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");
	std::vector<Token> parameters;
	if (!check(RIGHT_PAREN))
	{
		do
		{
			if (parameters.size() >= 8)
			{
				error(peek(), "Cannot have more than 8 parameters.");
			}
			parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
		} while (match(COMMA));
	}
	consume(RIGHT_PAREN, "Expect ')' after parameters.");

	consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
	std::vector<std::unique_ptr<Stmt>> body = block();

	//return new Stmt::Function(name, std::move(parameters), std::move(body));
	return std::make_unique<Stmt::Function>(name, std::move(parameters), std::move(body));
	//return {name, std::move(parameters), std::move(body)};

}

//std::vector<Stmt*> Parser::block()
std::vector<std::unique_ptr<Stmt>> Parser::block()
{
	std::vector<std::unique_ptr<Stmt>> statements;

	while (!check(RIGHT_BRACE) && !isAtEnd())
	{
		statements.emplace_back(declaration());
	}

	consume(RIGHT_BRACE, "Expect '}' after block.");
	return statements;
}

Expr* Parser::assignment()
{
	Expr* expr = logicOr();

	if (match(EQUAL))
	{
		const Token equals = previous();
		Expr* value = assignment();

		// variable
		if (auto* var = dynamic_cast<Expr::Variable*>(expr); var != nullptr)
		{
			const Token name = var->name;
			return new Expr::Assign(name, value);
		}

		// field
		if (const auto get = dynamic_cast<Expr::Get*>(expr); get != nullptr)
		{
			return new Expr::Set(get->object, get->name, value);
		}

		error(equals, "Invalid assignment target.");
	}

	return expr;
}

Expr* Parser::logicOr()
{
	Expr* expr = logicAnd();

	while (match(OR))
	{
		const Token op = previous();
		Expr* right = logicAnd();
		expr = new Expr::Logical(expr, op, right);
	}

	return expr;
}

Expr* Parser::logicAnd()
{
	Expr* expr = equality();

	while (match(AND))
	{
		const Token op = previous();
		Expr* right = equality();
		expr = new Expr::Logical(expr, op, right);
	}

	return expr;
}

Expr* Parser::equality()
{
	Expr* expr = comparison();

	while (match(BANG_EQUAL, EQUAL_EQUAL))
	{
		const Token op = previous();
		Expr* right = comparison();
		expr = new Expr::Binary(expr, op, right);
	}

	return expr;
}

Expr* Parser::comparison()
{
	Expr* expr = term();

	while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL))
	{
		const Token op = previous();
		Expr* right = term();
		expr = new Expr::Binary(expr, op, right);
	}
	return expr;
}

Expr* Parser::term()
{
	Expr* expr = factor();

	while (match(MINUS, PLUS))
	{
		const Token op = previous();
		Expr* right = factor();
		expr = new Expr::Binary(expr, op, right);
	}

	return expr;
}

Expr* Parser::factor()
{
	Expr* expr = unary();

	while (match(SLASH, STAR))
	{
		const Token op = previous();
		Expr* right = unary();
		expr = new Expr::Binary(expr, op, right);
	}

	return expr;
}

Expr* Parser::unary()
{
	if (match(BANG, MINUS))
	{
		const Token op = previous();
		Expr* right = unary();
		return new Expr::Unary(op, right);
	}

	return call();
}

Expr* Parser::finishCall(Expr* callee)
{
	std::vector<Expr*> arguments;
	if (!check(RIGHT_PAREN))
	{
		do
		{
			if (arguments.size() >= 8) { error(peek(), "Cannot have more than 8 arguments."); }
			arguments.push_back(expression());
		} while (match(COMMA));
	}

	const Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

	return new Expr::Call(callee, paren, std::move(arguments));
}

Expr* Parser::call()
{
	Expr* expr = primary();

	while (true)
	{
		if (match(LEFT_PAREN))
		{
			expr = finishCall(expr);
		}
		else if (match(DOT))
		{
			const Token name = consume(IDENTIFIER, "Expect property name after '.'.");
			expr = new Expr::Get(expr, name);
		}
		else
		{
			break;
		}
	}

	return expr;
}

Expr* Parser::primary()
{
	if (match(FALSE)) return new Expr::Literal(false);
	if (match(TRUE)) return new Expr::Literal(true);
	if (match(NIL)) return new Expr::Literal({});

	if (match(NUMBER, STRING))
	{
		return new Expr::Literal(previous().literal);
	}

	if (match(SUPER))
	{
		const Token keyword = previous();
		consume(DOT, "Expect '.' after 'super'.");
		const Token method = consume(IDENTIFIER, "Expect superclass method name.");
		return new Expr::Super(keyword, method);
	}

	if (match(THIS)) return new Expr::This(previous());

	if (match(IDENTIFIER))
	{
		return new Expr::Variable(previous());
	}

	if (match(LEFT_PAREN))
	{
		Expr* expr = expression();
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		return new Expr::Grouping(expr);
	}

	throw error(peek(), "Expect expression.");
}



Token Parser::consume(const TokenType type, const std::string& message)
{
	if (check(type)) return advance();

	throw error(peek(), message);
}

bool Parser::check(const TokenType type)
{
	if (isAtEnd()) return false;
	return peek().type == type;
}

Token Parser::advance()
{
	if (!isAtEnd()) current++;
	return previous();
}

bool Parser::isAtEnd()
{
	return peek().type == END_OF_FILE;
}

Token Parser::peek()
{
	return tokens.at(current);
}

Token Parser::previous()
{
	return tokens.at(static_cast<size_t>(current) - 1);
}

Parser::ParseError Parser::error(const Token& token, const std::string& message) const
{
	Lox::Error(token, message);
	return {};
}

void Parser::synchronize()
{
	advance();

	while (!isAtEnd())
	{
		if (previous().type == SEMICOLON) return;

		switch (peek().type)
		{
		case CLASS:
		case FUN:
		case VAR:
		case FOR:
		case IF:
		case WHILE:
		case PRINT:
		case RETURN:
			return;
		default:
			break;
		}

		advance();
	}
}
