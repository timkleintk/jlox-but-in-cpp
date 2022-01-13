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

	std::vector<std::shared_ptr<Stmt>> parse();

private:
	std::unique_ptr<Expr> expression();
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
	std::unique_ptr<Expr> assignment();
	std::unique_ptr<Expr> logicOr();
	std::unique_ptr<Expr> logicAnd();
	std::unique_ptr<Expr> equality();
	std::unique_ptr<Expr> comparison();
	std::unique_ptr<Expr> term();
	std::unique_ptr<Expr> factor();
	std::unique_ptr<Expr> unary();
	std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
	std::unique_ptr<Expr> call();
	std::unique_ptr<Expr> primary();

	template <typename ... Ts>
	bool match(Ts ... args);

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


template <typename ... Ts>
bool Parser::match(Ts... args)
{
	for (const auto& t : {args...})
	{
		if (check(t))
		{
			advance();
			return true;
		}
	}
	return false;
}
