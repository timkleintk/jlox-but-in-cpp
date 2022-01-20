#include "parser.h"

#include "lox.h"


Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens)
{}

std::vector<std::shared_ptr<Stmt>> Parser::parse()
{
	std::vector<std::shared_ptr<Stmt>> statements;
	while (!isAtEnd())
	{
		statements.push_back(declaration());
	}
	return statements;
}


// statements ------------------------------------------------------

std::shared_ptr<Stmt> Parser::declaration()
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

std::shared_ptr<Stmt> Parser::classDeclaration()
{
	const Token name = consume(IDENTIFIER, "Expect class name.");

	std::shared_ptr<Expr::Variable> superclass = nullptr;

	if (match(LESS))
	{
		consume(IDENTIFIER, "Expect superclass name.");
		superclass = newShared<Expr::Variable>(previous());
	}

	consume(LEFT_BRACE, "Expect '{' before class body.");
	std::vector<std::shared_ptr<Stmt::Function>> methods;
	while (!check(RIGHT_BRACE) && !isAtEnd())
	{
		std::shared_ptr<Stmt::Function> method = function("method");
		methods.push_back(std::move(method));
	}
	consume(RIGHT_BRACE, "Expect '}' after class body.");

	return newShared<Stmt::Class>(name, std::move(superclass), std::move(methods));
}

std::shared_ptr<Stmt::Function> Parser::function(const std::string& kind)
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
				(void)error(peek(), "Cannot have more than 8 parameters.");
			}
			parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
		} while (match(COMMA));
	}
	consume(RIGHT_PAREN, "Expect ')' after parameters.");

	consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
	std::vector<std::shared_ptr<Stmt>> body = block();

	return newShared<Stmt::Function>(name, std::move(parameters), std::move(body));
}

std::vector<std::shared_ptr<Stmt>> Parser::block()
{
	std::vector<std::shared_ptr<Stmt>> statements;

	while (!check(RIGHT_BRACE) && !isAtEnd())
	{
		statements.push_back(declaration());
	}
	consume(RIGHT_BRACE, "Expect '}' after block.");

	return statements;
}

std::shared_ptr<Stmt> Parser::varDeclaration()
{
	const Token name = consume(IDENTIFIER, "Expect variable name.");

	std::shared_ptr<Expr> initializer = nullptr;
	if (match(EQUAL))
	{
		initializer = expression();
	}
	consume(SEMICOLON, "Expect ';' after variable declaration.");

	return newShared<Stmt::Var>(name, std::move(initializer));
}

std::shared_ptr<Stmt> Parser::statement()
{
	if (match(FOR)) return forStatement();
	if (match(IF)) return ifStatement();
	if (match(PRINT)) return printStatement();
	if (match(RETURN)) return returnStatement();
	if (match(WHILE)) return whileStatement();
	if (match(LEFT_BRACE)) return newShared<Stmt::Block>(block());
	return expressionStatement();
}

std::shared_ptr<Stmt> Parser::forStatement()
{
	// for (initializer; condition; increment) body

	consume(LEFT_PAREN, "Expect '(' after 'for'");

	// initializer
	std::shared_ptr<Stmt> initializer = nullptr;
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
	std::shared_ptr<Expr> condition = nullptr;
	if (!check(SEMICOLON))
	{
		condition = expression();
	}
	consume(SEMICOLON, "Expect ';' after loop condition.");

	// increment
	std::shared_ptr<Expr> increment = nullptr;
	if (!check(RIGHT_PAREN))
	{
		increment = expression();
	}
	consume(RIGHT_PAREN, "Expect ')' after for clauses.");

	// body
	std::shared_ptr<Stmt> body = statement();


	// desugaring --------------------------------------------------

	// add increment at the end of loop body
	if (increment != nullptr)
	{
		body = newShared<Stmt::Block>(std::vector<std::shared_ptr<Stmt>> {
			std::move(body),
			newShared<Stmt::Expression>(std::move(increment))
		});
	}

	// make a while loop out of the condition and the body
	if (condition == nullptr)
	{
		condition = newShared<Expr::Literal>(true);
	}
	body = newShared<Stmt::While>(std::move(condition), std::move(body));

	// add initializer before the loop
	if (initializer != nullptr)
	{
		body = newShared<Stmt::Block>(std::vector<std::shared_ptr<Stmt>>
		{
			std::move(initializer),
				std::move(body)
		});
	}

	return body;
}

std::shared_ptr<Stmt> Parser::ifStatement()
{
	consume(LEFT_PAREN, "Expect '(' after 'if',");

	std::shared_ptr<Expr> condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after if condition.");

	std::shared_ptr<Stmt> thenBranch = statement();
	std::shared_ptr<Stmt> elseBranch = nullptr;

	if (match(ELSE))
	{
		elseBranch = statement();
	}

	return newShared<Stmt::If>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::shared_ptr<Stmt> Parser::printStatement()
{
	std::shared_ptr<Expr> value = expression();
	consume(SEMICOLON, "Expect ';' after value");

	return newShared<Stmt::Print>(std::move(value));
}

std::shared_ptr<Stmt> Parser::returnStatement()
{
	const Token keyword = previous();

	std::shared_ptr<Expr> value = nullptr;
	if (!check(SEMICOLON))
	{
		value = expression();
	}
	consume(SEMICOLON, "Expect ';' after return value.");

	return newShared<Stmt::Return>(keyword, std::move(value));
}

std::shared_ptr<Stmt> Parser::whileStatement()
{
	consume(LEFT_PAREN, "Expect '(' after 'while'.");

	std::shared_ptr<Expr> condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after condition.");

	std::shared_ptr<Stmt> body = statement();

	return newShared<Stmt::While>(std::move(condition), std::move(body));
}

std::shared_ptr<Stmt> Parser::expressionStatement()
{
	std::shared_ptr<Expr> expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");

	return newShared<Stmt::Expression>(std::move(expr));
}


// expressions -----------------------------------------------------

std::shared_ptr<Expr> Parser::expression()
{
	return assignment();
}

std::shared_ptr<Expr> Parser::assignment()
{
	std::shared_ptr<Expr> expr = logicOr();

	if (match(EQUAL))
	{
		const Token equals = previous();

		std::shared_ptr<Expr> value = assignment();

		// variable
		if (const auto* var = dynamic_cast<Expr::Variable*>(expr.get()); var != nullptr)
		{
			return newShared<Expr::Assign>(var->name, std::move(value));
		}

		// field
		if (const auto* field = dynamic_cast<Expr::Get*>(expr.get()); field != nullptr)
		{
			return newShared<Expr::Set>(field->object, field->name, std::move(value));
		}

		(void)error(equals, "Invalid assignment target.");
	}

	return expr;
}

std::shared_ptr<Expr> Parser::logicOr()
{
	std::shared_ptr<Expr> expr = logicAnd();

	while (match(OR))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = logicAnd();
		expr = newShared<Expr::Logical>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::shared_ptr<Expr> Parser::logicAnd()
{
	std::shared_ptr<Expr> expr = equality();

	while (match(AND))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = equality();
		expr = newShared<Expr::Logical>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::shared_ptr<Expr> Parser::equality()
{
	std::shared_ptr<Expr> expr = comparison();

	while (match(BANG_EQUAL, EQUAL_EQUAL))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = comparison();
		expr = newShared<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::shared_ptr<Expr> Parser::comparison()
{
	std::shared_ptr<Expr> expr = term();

	while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = term();
		expr = newShared<Expr::Binary>(std::move(expr), op, std::move(right));
	}
	return expr;
}

std::shared_ptr<Expr> Parser::term()
{
	std::shared_ptr<Expr> expr = factor();

	while (match(MINUS, PLUS))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = factor();
		expr = newShared<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::shared_ptr<Expr> Parser::factor()
{
	std::shared_ptr<Expr> expr = unary();

	while (match(SLASH, STAR))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = unary();
		expr = newShared<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::shared_ptr<Expr> Parser::unary()
{
	if (match(BANG, MINUS))
	{
		const Token op = previous();
		std::shared_ptr<Expr> right = unary();
		return newShared<Expr::Unary>(op, std::move(right));
	}

	return call();
}

std::shared_ptr<Expr> Parser::call()
{
	// function( args...)

	std::shared_ptr<Expr> expr = primary();

	while (true)
	{
		if (match(LEFT_PAREN))
		{
			expr = finishCall(std::move(expr));
		}
		else if (match(DOT))
		{
			const Token name = consume(IDENTIFIER, "Expect property name after '.'.");
			expr = newShared<Expr::Get>(std::move(expr), name);
		}
		else
		{
			break;
		}
	}

	return expr;
}

std::shared_ptr<Expr> Parser::finishCall(std::shared_ptr<Expr> callee)
{
	// "functionName(" has already been consumed and is in callee,
	// this function will take care of the arguments and the closing ')'

	std::vector<std::shared_ptr<Expr>> arguments;

	if (!check(RIGHT_PAREN))
	{
		do
		{
			if (arguments.size() >= 8) { (void)error(peek(), "Cannot have more than 8 arguments."); }
			arguments.push_back(expression());
		} while (match(COMMA));
	}
	const Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

	return newShared<Expr::Call>(std::move(callee), paren, std::move(arguments));
}

std::shared_ptr<Expr> Parser::primary()
{
	if (match(FALSE)) return newShared<Expr::Literal>(false);
	if (match(TRUE)) return newShared<Expr::Literal>(true);
	if (match(NIL)) return newShared<Expr::Literal>(object_t());

	if (match(NUMBER, STRING))
	{
		return newShared<Expr::Literal>(previous().literal);
	}

	if (match(SUPER))
	{
		const Token keyword = previous();
		consume(DOT, "Expect '.' after 'super'.");

		const Token method = consume(IDENTIFIER, "Expect superclass method name.");

		return newShared<Expr::Super>(keyword, method);
	}

	if (match(THIS))
	{
		return newShared<Expr::This>(previous());
	}

	if (match(IDENTIFIER))
	{
		return newShared<Expr::Variable>(previous());
	}

	if (match(LEFT_PAREN))
	{
		std::shared_ptr<Expr> expr = expression();
		consume(RIGHT_PAREN, "Expect ')' after expression.");

		return newShared<Expr::Grouping>(std::move(expr));
	}

	throw error(peek(), "Expect expression.");
}


// helper functions ------------------------------------------------

Token Parser::consume(const TokenType type, const std::string& message)
{
	if (check(type)) return advance();

	throw error(peek(), message);
}

bool Parser::check(const TokenType type) const
{
	if (isAtEnd()) return false;
	return peek().type == type;
}

Token Parser::advance()
{
	if (!isAtEnd()) m_current++;
	return previous();
}

bool Parser::isAtEnd() const
{
	return peek().type == END_OF_FILE;
}

Token Parser::peek() const
{
	return m_tokens.at(m_current);
}

Token Parser::previous() const
{
	return m_tokens.at(m_current - 1);
}


// error handling --------------------------------------------------

ParseError Parser::error(const Token& token, const std::string& message) const
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
