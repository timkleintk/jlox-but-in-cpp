#include "parser.h"


#include <cassert>
#include <cstdarg>

#include "lox.h"

ParseError Error(const Token& token, const std::string& message)
{
	Lox::Error(token, message);
	return {};
};

std::vector<std::shared_ptr<Stmt>> Parser::parse()
{
	std::vector<std::shared_ptr<Stmt>> statements;
	while (!isAtEnd())
	{
		//statements.emplace_back(declaration()->getShared()); // nts: getShared?
		std::shared_ptr<Stmt> stmt = declaration(); // nts: scuffed
		if (stmt != nullptr)
		{
			stmt->setShared(stmt);
			statements.emplace_back(std::move(stmt)); // nts: getShared?
		}
	}
	return statements;
}


std::unique_ptr<Expr> Parser::expression()
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

	//Expr::Variable* superclass = nullptr;
	//std::shared_ptr<Expr::Variable> superclass = nullptr;
	std::unique_ptr<Expr::Variable> superclass = nullptr;

	if (match(LESS))
	{
		consume(IDENTIFIER, "Expect superclass name.");
		//superclass = new Expr::Variable(previous());
		//superclass = newShared<Expr::Variable>(previous());
		superclass = std::make_unique<Expr::Variable>(previous());
	}

	consume(LEFT_BRACE, "Expect '{' before class body.");


	// nts: class methods are shared pointers, because instances have a shared pointer to them
	std::vector<std::shared_ptr<Stmt::Function>> methods;
	while (!check(RIGHT_BRACE) && !isAtEnd())
	{
		// nts: this could probably be improved
		methods.emplace_back(function("method"));
	}

	consume(RIGHT_BRACE, "Expect '}' after class body.");

	//return new Stmt::Class(name, superclass, methods);
	return std::make_unique<Stmt::Class>(name, std::move(superclass), std::move(methods));
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
	//Expr* condition = nullptr;
	std::unique_ptr<Expr> condition = nullptr;

	if (!check(SEMICOLON))
	{
		condition = expression();
		//condition.reset(expression());
	}
	consume(SEMICOLON, "Expect ';' after loop condition.");

	// increment
	//Expr* increment = nullptr;
	std::unique_ptr<Expr> increment = nullptr;

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
		std::vector<std::unique_ptr<Stmt>> statements;

		statements.push_back(std::move(body));
		statements.push_back(std::make_unique<Stmt::Expression>(std::move(increment)));

		body.reset(new Stmt::Block(std::move(statements)));
	}

	// make a while loop out of the condition and the body
	if (condition == nullptr)
	{ condition = std::make_unique<Expr::Literal>(true); }

	//body = new Stmt::While(condition, body);
	body.reset(new Stmt::While(std::move(condition), std::move(body)));

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

	//Expr* condition = expression();
	std::unique_ptr<Expr> condition(expression());

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
	return std::make_unique<Stmt::If>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::printStatement()
{
	//Expr* value = expression();
	std::unique_ptr<Expr> value(expression());

	consume(SEMICOLON, "Expect ';' after value");

	//return new Stmt::Print(value);
	return std::make_unique<Stmt::Print>(std::move(value));
}

std::unique_ptr<Stmt> Parser::returnStatement()
{
	const Token keyword = previous();

	//Expr* value = nullptr;
	std::unique_ptr<Expr> value = nullptr;

	if (!check(SEMICOLON))
	{
		value = expression();
		//value.reset(expression());
	}

	consume(SEMICOLON, "Expect ';' after return value.");

	//return new Stmt::Return(keyword, value);
	return std::make_unique<Stmt::Return>(keyword, std::move(value)); // nts: does move do anythig here?
}

std::unique_ptr<Stmt> Parser::varDeclaration()
{
	const Token name = consume(IDENTIFIER, "Expect variable name.");

	//Expr* initializer = nullptr;
	std::unique_ptr<Expr> initializer = nullptr;


	if (match(EQUAL)) initializer = expression();
	//if (match(EQUAL)) initializer.reset(expression());

	consume(SEMICOLON, "Expect ';' after variable declaration.");

	//return new Stmt::Var(name, initializer);
	return std::make_unique<Stmt::Var>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::whileStatement()
{
	consume(LEFT_PAREN, "Expect '(' after 'while'.");

	//Expr* condition = expression();
	std::unique_ptr<Expr> condition(expression());

	consume(RIGHT_PAREN, "Expect '(' after condition.");

	//Stmt* body = statement().release();
	std::unique_ptr<Stmt> body = statement();

	//return new Stmt::While(condition, body);
	return std::make_unique<Stmt::While>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::expressionStatement()
{
	//Expr* expr = expression();
	std::unique_ptr<Expr> expr(expression());


	consume(SEMICOLON, "Expect ';' after expression.");

	//return new Stmt::Expression(expr);
	return std::make_unique<Stmt::Expression>(std::move(expr));
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

std::unique_ptr<Expr> Parser::assignment()
{
	auto expr = logicOr();

	if (match(EQUAL))
	{
		const Token equals = previous();
		auto value = assignment();

		// variable
		//if (auto* var = dynamic_cast<Expr::Variable*>(expr); var != nullptr)
		if (auto* var = dynamic_cast<Expr::Variable*>(expr.get()); var != nullptr)
		{
			const Token name = var->name;
			//return new Expr::Assign(name, value);
			return std::make_unique<Expr::Assign>(name, std::move(value));
		}

		// field
		//if (const auto get = dynamic_cast<Expr::Get*>(expr); get != nullptr)
		if (const auto get = dynamic_cast<Expr::Get*>(expr.get()); get != nullptr) // nts: look at ownership here
		{
			//return new Expr::Set(get->object, get->name, value);
			return std::make_unique<Expr::Set>(std::move(get->object), get->name, std::move(value));
		}

		error(equals, "Invalid assignment target.");
	}

	return expr;
}

std::unique_ptr<Expr> Parser::logicOr()
{
	auto expr = logicAnd();

	while (match(OR))
	{
		const Token op = previous();
		auto right = logicAnd();
		//expr = new Expr::Logical(expr, op, right);
		expr = std::make_unique<Expr::Logical>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::logicAnd()
{
	auto expr = equality();

	while (match(AND))
	{
		const Token op = previous();
		auto right = equality();
		//expr = new Expr::Logical(expr, op, right);
		expr = std::make_unique<Expr::Logical>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::equality()
{
	auto expr = comparison();

	while (match(BANG_EQUAL, EQUAL_EQUAL))
	{
		const Token op = previous();
		auto right = comparison();
		//expr = new Expr::Binary(expr, op, right);
		expr = std::make_unique<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::comparison()
{
	auto expr = term();

	while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL))
	{
		const Token op = previous();
		auto right = term();
		//expr = new Expr::Binary(expr, op, right);
		expr = std::make_unique<Expr::Binary>(std::move(expr), op, std::move(right));
	}
	return expr;
}

std::unique_ptr<Expr> Parser::term()
{
	auto expr = factor();

	while (match(MINUS, PLUS))
	{
		const Token op = previous();
		auto right = factor();
		//expr = new Expr::Binary(expr, op, right);
		expr = std::make_unique<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::factor()
{
	auto expr = unary();

	while (match(SLASH, STAR))
	{
		const Token op = previous();
		auto right = unary();
		//expr = new Expr::Binary(expr, op, right);
		expr = std::make_unique<Expr::Binary>(std::move(expr), op, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expr> Parser::unary()
{
	if (match(BANG, MINUS))
	{
		const Token op = previous();
		auto right = unary();
		//return new Expr::Unary(op, right);
		return std::make_unique<Expr::Unary>(op, std::move(right));
	}

	return call();
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) // nts: look at callee type
{
	//std::vector<Expr*> arguments;
	std::vector<std::unique_ptr<Expr>> arguments;
	if (!check(RIGHT_PAREN))
	{
		do
		{
			if (arguments.size() >= 8) { error(peek(), "Cannot have more than 8 arguments."); }
			//arguments.push_back(expression());
			arguments.emplace_back(expression());
		} while (match(COMMA));
	}

	const Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

	//return new Expr::Call(callee, paren, std::move(arguments));
	return std::make_unique<Expr::Call>(std::move(callee), paren, std::move(arguments));
}

std::unique_ptr<Expr> Parser::call()
{
	auto expr = primary();

	while (true)
	{
		if (match(LEFT_PAREN))
		{
			expr = finishCall(std::move(expr));
		}
		else if (match(DOT))
		{
			const Token name = consume(IDENTIFIER, "Expect property name after '.'.");
			//expr = new Expr::Get(expr, name);
			expr = std::make_unique<Expr::Get>(std::move(expr), name);
		}
		else
		{
			break;
		}
	}

	return expr;
}

std::unique_ptr<Expr> Parser::primary()
{
	//if (match(FALSE)) return new Expr::Literal(false);
	if (match(FALSE)) return std::make_unique<Expr::Literal>(false);
	//if (match(TRUE)) return new Expr::Literal(true);
	if (match(TRUE)) return std::make_unique<Expr::Literal>(true);
	//if (match(NIL)) return new Expr::Literal({});
	if (match(NIL)) return std::make_unique<Expr::Literal>(object_t());

	if (match(NUMBER, STRING))
	{
		//return new Expr::Literal(previous().literal);
		return std::make_unique<Expr::Literal>(previous().literal);
	}

	if (match(SUPER))
	{
		const Token keyword = previous();
		consume(DOT, "Expect '.' after 'super'.");
		const Token method = consume(IDENTIFIER, "Expect superclass method name.");
		//return new Expr::Super(keyword, method);
		return std::make_unique<Expr::Super>(keyword, method);
	}

	//if (match(THIS)) return new Expr::This(previous());
	if (match(THIS)) return std::make_unique<Expr::This>(previous());

	if (match(IDENTIFIER))
	{
		//return new Expr::Variable(previous());
		return std::make_unique<Expr::Variable>(previous());
	}

	if (match(LEFT_PAREN))
	{
		auto expr = expression();
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		//return new Expr::Grouping(expr);
		return std::make_unique<Expr::Grouping>(std::move(expr));
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
