#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "token.h"


class Scanner
{
public:
	Scanner(const std::string& source);

	std::vector<Token> scan();

private:
	bool isAtEnd() const;
	void addToken(TokenType type, object_t literal = {});
	void consume();
	char advance();
	char peek() const;
	char peekNext() const;
	bool match(char c);

	void string();
	void number();
	void identifier();
	void scanToken();


	size_t m_start = 0;
	size_t m_current = 0;
	size_t m_line = 1;

	const std::string& m_source;
	std::vector<Token> m_tokens;
	std::unordered_map<std::string, TokenType> m_keywords;
};
