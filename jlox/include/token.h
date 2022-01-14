#pragma once

#include <string>

#include "object.h"

enum TokenType
{
	// reserved
	INVALID,

	// single character tokens
	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

	// One or two character tokens
	BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

	// Literals
	IDENTIFIER, STRING, NUMBER,

	// Keywords
	AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR, PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

	END_OF_FILE
};


class Token
{
public:
	Token(const TokenType type, std::string lexeme, object_t literal, const size_t line) :
		type(type),
		lexeme(std::move(lexeme)),
		literal(std::move(literal)),
		line(line)
	{}

	TokenType type;
	std::string lexeme;
	object_t literal;
	size_t line;
};
