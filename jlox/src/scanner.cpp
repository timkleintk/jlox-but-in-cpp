#include "scanner.h"

#include "lox.h"

Scanner::Scanner(const std::string& source) : m_source(source)
{
	m_keywords.emplace("and", AND);
	m_keywords.emplace("class", CLASS);
	m_keywords.emplace("else", ELSE);
	m_keywords.emplace("false", FALSE);
	m_keywords.emplace("for", FOR);
	m_keywords.emplace("fun", FUN);
	m_keywords.emplace("if", IF);
	m_keywords.emplace("nil", NIL);
	m_keywords.emplace("or", OR);
	m_keywords.emplace("print", PRINT);
	m_keywords.emplace("return", RETURN);
	m_keywords.emplace("super", SUPER);
	m_keywords.emplace("this", THIS);
	m_keywords.emplace("true", TRUE);
	m_keywords.emplace("var", VAR);
	m_keywords.emplace("while", WHILE);
}

std::vector<Token> Scanner::scan()
{
	while (!isAtEnd())
	{
		m_start = m_current;
		scanToken();
	}

	addToken(END_OF_FILE);

	return std::move(m_tokens);
}


bool Scanner::isAtEnd() const
{
	return m_current >= m_source.size();
}

void Scanner::addToken(TokenType type, object_t literal)
{
	std::string lexeme = m_source.substr(m_start, m_current - m_start);
	m_tokens.emplace_back(type, std::move(lexeme), std::move(literal), m_line);
}

void Scanner::consume()
{
	m_current++;
}

char Scanner::advance()
{
	return m_source.at(m_current++);
}

char Scanner::peek() const
{
	return isAtEnd() ? '\0' : m_source.at(m_current);
}

char Scanner::peekNext() const
{
	return m_current + 1 >= m_source.size() ? '\0' : m_source.at(m_current + 1);
}

bool Scanner::match(const char c)
{
	if (isAtEnd() || m_source.at(m_current) != c) return false;
	m_current++;
	return true;
}

void Scanner::string()
{
	while (peek() != '"' && !isAtEnd())
	{
		if (peek() == '\n') m_line++;
		consume();
	}

	if (isAtEnd())
	{
		Lox::Error(m_line, "Unterminated string.");
		return;
	}

	// consume the closing '"'
	consume();

	// trim the double quotes
	addToken(STRING, m_source.substr(m_start + 1, m_current - m_start - 2));
}

void Scanner::number()
{
	while (isdigit(peek())) consume();

	if (peek() == '.' && isdigit(peekNext()))
	{
		// consume the '.'
		consume();

		// consume decimals
		while (isdigit(peek())) consume();
	}

	addToken(NUMBER, strtod(m_source.substr(m_start, m_current - m_start).c_str(), nullptr));
}

void Scanner::identifier()
{
	while (isalpha(peek()) || isdigit(peek()) || peek() == '_') consume();

	const auto it = m_keywords.find(m_source.substr(m_start, m_current - m_start));
	const TokenType t = it == m_keywords.end() ? IDENTIFIER : it->second;
	addToken(t);
}

void Scanner::scanToken()
{
	switch (const char c = advance())
	{
		// single character tokens
		{
	case '(': addToken(LEFT_PAREN); break;
	case ')': addToken(RIGHT_PAREN); break;
	case '{': addToken(LEFT_BRACE); break;
	case '}': addToken(RIGHT_BRACE); break;
	case ',': addToken(COMMA); break;
	case '.': addToken(DOT); break;
	case '-': addToken(MINUS); break;
	case '+': addToken(PLUS); break;
	case ';': addToken(SEMICOLON); break;
	case '*': addToken(STAR); break;
		}

		// one or two character tokens
		{
	case '!': addToken(match('=') ? BANG_EQUAL : BANG); break;
	case '=': addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
	case '<': addToken(match('=') ? LESS_EQUAL : LESS); break;
	case '>': addToken(match('=') ? GREATER_EQUAL : GREATER);	break;

	case '/':
		if (match('/'))
		{
			while (peek() != '\n' && !isAtEnd())
			{
				consume();
			}
		}
		else
		{
			addToken(SLASH);
		}
		break;
		}

		// whitespace
	case ' ':
	case '\r':
	case '\t':
		break;
	case '\n':
		m_line++;
		break;

		// string literal
	case '"': string(); break;

	default:
		if (isdigit(c))
			number();
		else if (isalpha(c) || c == '_')
			identifier();
		else
			Lox::Error(m_line, "Unexpected character.");
		break;
	}
}
