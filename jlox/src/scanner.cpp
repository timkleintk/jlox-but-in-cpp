#include "scanner.h"

#include <unordered_map>

#include "lox.h"

// https://craftinginterpreters.com/scanning.html#the-interpreter-framework


std::vector<Token> ScanTokens(const std::string& source)
{
	// local variables
	int start = 0;
	int current = 0;
	int line = 1; // why do line numbers start at one ?!

	std::vector<Token> tokens;

	std::unordered_map<std::string, TokenType> keywords;
	keywords.try_emplace("and", AND);
	keywords.try_emplace("class", CLASS);
	keywords.try_emplace("else", ELSE);
	keywords.try_emplace("false", FALSE);
	keywords.try_emplace("for", FOR);
	keywords.try_emplace("fun", FUN);
	keywords.try_emplace("if", IF);
	keywords.try_emplace("nil", NIL);
	keywords.try_emplace("or", OR);
	keywords.try_emplace("print", PRINT);
	keywords.try_emplace("return", RETURN);
	keywords.try_emplace("super", SUPER);
	keywords.try_emplace("this", THIS);
	keywords.try_emplace("true", TRUE);
	keywords.try_emplace("var", VAR);
	keywords.try_emplace("while", WHILE);

	// helper functions
	const auto isAtEnd = [&] { return current >= static_cast<int>(source.size()); };

	const auto addToken = [&](TokenType type, const object_t& literal = {})
	{
		std::string text = source.substr(start, static_cast<size_t>(current) - static_cast<size_t>(start));
		tokens.emplace_back(type, text, literal, line);
	};


	const auto consume = [&] { current++; };

	const auto advance = [&] { return source.at(current++); };

	const auto peek = [&] { return isAtEnd() ? '\0' : source.at(current); };

	const auto peekNext = [&] { return static_cast<size_t>(current) + 1 >= source.size() ? '\0' : source.at(static_cast<size_t>(current) + 1); };

	const auto match = [&](const char c)
	{
		if (isAtEnd()) return false;
		if (source.at(current) != c) return false;

		current++;
		return true;
	};


	const auto string = [&]
	{
		while (peek() != '"' && !isAtEnd())
		{
			if (peek() == '\n') line++;
			consume();
		}

		if (isAtEnd())
		{

			Lox::Error(line, std::string("Unterminated string."));
			return;
		}

		// consume the closing '"'
		consume();

		// trim the double quotes
		addToken(STRING, source.substr(static_cast<size_t>(start) + 1, static_cast<size_t>(current) - start - 2));
	};

	const auto number = [&]
	{
		while (isdigit(peek())) consume();

		if (peek() == '.' && isdigit(peekNext()))
		{
			// consume the '.'
			consume();

			// consume decimals
			while (isdigit(peek())) consume();
		}

		addToken(NUMBER, strtod(source.substr(start, static_cast<size_t>(current) - start).c_str(), nullptr));
	};

	const auto identifier = [&]
	{
		while (isalpha(peek()) || isdigit(peek()) || peek() == '_') consume();

		const auto it = keywords.find(source.substr(start, static_cast<size_t>(current) - start));
		const TokenType t = it == keywords.end() ? IDENTIFIER : it->second;
		addToken(t);
	};


	const auto scanToken = [&]
	{
		const char c = advance();
		switch (c)
		{
			// nts: make pragma regions
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

		#pragma region whitespace

		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			line++;
			break;
		#pragma endregion

			// string literal
		case '"': string(); break;


		default:
			if (isdigit(c))
				number();
			else if (isalpha(c) || c == '_')
				identifier();
			else
				Lox::Error(line, "Unexpected character.");
			break;
		}
	};


	while (!isAtEnd())
	{
		start = current;
		scanToken();
	}


	addToken(END_OF_FILE);

	return tokens;
}
