#include "token.h"

#include <utility>
#include <iostream>

const std::string TokenTypeNames[] =
{
	"INVALID", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "BANG", "BANG_EQUAL", "EQUAL", "EQUAL_EQUAL", "GREATER", "GREATER_EQUAL", "LESS", "LESS_EQUAL", "IDENTIFIER", "STRING", "NUMBER", "AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR", "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE", "END_OF_FILE"
};

//std::ostream& operator<<(std::ostream& os, const Token& token)
//{
//	os << TokenTypeNames[static_cast<size_t>(token.type)] << " " << token.lexeme << " ";
//	switch (token.literal.type)
//	{
//	case Object::Type::BOOL:
//		os << token.literal.boolean ;
//		break;
//	case Object::Type::STRING:
//		os << token.literal.string;
//		break;
//	case Object::Type::NIL:
//		os << "nil";
//		break;
//	case Object::Type::NUMBER:
//		os << token.literal.number;
//		break;
//	default:
//		break;
//	}
//
//	return os;
//}


Token::Token(const TokenType type, std::string lexeme, object_t literal, const int line):
	type(type),
	lexeme(std::move(lexeme)),
	literal(std::move(literal)),
	line(line)
{}
