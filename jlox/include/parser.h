#pragma once

#include <memory>
#include <vector>

#include "expr.h"
#include "token.h"


class ParseError final : public std::exception {};


class Parser
{
public:
	explicit Parser(std::vector<Token> tokens);

	std::vector<std::shared_ptr<Stmt>> parse();

private:
	std::shared_ptr<Stmt> declaration();
	std::shared_ptr<Stmt> classDeclaration();
	std::shared_ptr<Stmt::Function> function(const std::string& kind);
	std::vector<std::shared_ptr<Stmt>> block();
	std::shared_ptr<Stmt> varDeclaration();
	std::shared_ptr<Stmt> statement();
	std::shared_ptr<Stmt> forStatement();
	std::shared_ptr<Stmt> ifStatement();
	std::shared_ptr<Stmt> printStatement();
	std::shared_ptr<Stmt> returnStatement();
	std::shared_ptr<Stmt> whileStatement();
	std::shared_ptr<Stmt> expressionStatement();

	std::shared_ptr<Expr> expression();
	std::shared_ptr<Expr> assignment();
	std::shared_ptr<Expr> logicOr();
	std::shared_ptr<Expr> logicAnd();
	std::shared_ptr<Expr> equality();
	std::shared_ptr<Expr> comparison();
	std::shared_ptr<Expr> term();
	std::shared_ptr<Expr> factor();
	std::shared_ptr<Expr> unary();
	std::shared_ptr<Expr> call();
	std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr>&& callee);
	std::shared_ptr<Expr> primary();

	template <typename ... Ts>
	bool match(Ts ... args);

	Token consume(TokenType type, const std::string& message);
	bool check(TokenType type);
	Token advance();
	bool isAtEnd();
	Token peek();
	Token previous();

	ParseError error(const Token& token, const std::string& message) const;
	void synchronize();

	std::vector<Token> tokens;
	int current = 0;
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
