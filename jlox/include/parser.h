// https://craftinginterpreters.com/parsing-expressions.html#the-parser-class

#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "expr.h"
#include "token.h"

class ParseError final : public std::exception {};

class Parser
{
public:

	std::vector<Token> tokens;
	int current = 0;

	explicit Parser(std::vector<Token> tokens): tokens(std::move(tokens))
	{}

	std::vector<Stmt*> parse();

private:
	Expr* expression();
	std::unique_ptr<Stmt> declaration();
	std::unique_ptr<Stmt> classDeclaration();
	std::unique_ptr<Stmt> statement();
	std::unique_ptr<Stmt> forStatement();
	std::unique_ptr<Stmt> ifStatement();
	std::unique_ptr<Stmt> printStatement();
	std::unique_ptr<Stmt> returnStatement();
	std::unique_ptr<Stmt> varDeclaration();
	std::unique_ptr<Stmt> whileStatement();
	std::unique_ptr<Stmt> expressionStatement();
	std::unique_ptr<Stmt::Function> function(const std::string& kind);
	std::vector<std::unique_ptr<Stmt>> block();
	Expr* assignment();
	Expr* logicOr();
	Expr* logicAnd();
	Expr* equality();
	Expr* comparison();
	Expr* term();
	Expr* factor();
	Expr* unary();
	Expr* finishCall(Expr* callee);
	Expr* call();
	Expr* primary();

	template <typename T, typename ... Ts>
	bool match(T head, Ts ... tail);

	template <typename T>
	bool match(T head);

	Token consume(TokenType type, const std::string& message);


	bool check(TokenType type);
	Token advance();
	bool isAtEnd();
	Token peek();
	Token previous();


	class ParseError final : public std::exception {};

	ParseError error(const Token& token, const std::string& message) const;
	void synchronize();
};


template <typename T, typename ... Ts>
bool Parser::match(T head, Ts... tail)
{
	if (check(head))
	{
		advance();
		return true;
	}
	return match(tail...);
}

template <typename T>
bool Parser::match(T head)
{
	if (check(head))
	{
		advance();
		return true;
	}
	return false;
}


