// https://craftinginterpreters.com/scanning.html#the-interpreter-framework

#include "lox.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "astPrinter.h"
#include "parser.h"
#include "resolver.h"
#include "RuntimeError.h"
#include "scanner.h"


void Lox::RunFile(const char* path)
{
	// https://stackoverflow.com/questions/18398167/how-to-copy-a-txt-file-to-a-char-array-in-c/18398230

	if (std::ifstream inputStream(path); inputStream.is_open())
	{
		const std::string source((std::istreambuf_iterator(inputStream)), std::istreambuf_iterator<char>());
		Run(source);

		if (m_hadError) { exit(65); }
		if (m_hadRuntimeError) { exit(70); }
	}
}

// returns the number of '{' + '(' - ')' - '}'
int getBraceBalance(const std::string& src)
{
	int braceCount = 0;
	for (const auto& c : src)
	{
		switch (c)
		{
		case '{':
		case '(':
			braceCount++;
			break;
		case '}':
		case ')':
			braceCount--;
			break;
		default:
			break;
		}
	}
	return braceCount;
}

void Lox::RunPrompt()
{
	std::string source;
	std::string line;

	do
	{
		std::cout << "> ";
		std::getline(std::cin, source);


		while (getBraceBalance(source) > 0)
		{
			std::cout << "  ";
			std::getline(std::cin, line);
			source += "\n";
			source += line;
		}

		if (std::cin.eof() || source == "exit" ) { break; }

		Run(source);
		m_hadError = false;

	} while (true);
}

void Lox::Error(const int line, const std::string& message)
{
	Report(line, "", message);
}

void Lox::runtimeError(const RuntimeError& error)
{
	std::cerr << error.what() << std::endl << "[line " << error.token.line << "]" << std::endl;
	m_hadRuntimeError = true;
}

void Lox::Error(const Token& token, const std::string& message)
{
	if (token.type == END_OF_FILE)
	{
		Report(token.line, " at end", message);
	}
	else
	{
		Report(token.line, " at '" + token.lexeme + "'", message);
	}
}


Interpreter Lox::m_interpreter = Interpreter();

bool Lox::m_hadError = false;
bool Lox::m_hadRuntimeError = false;

void Lox::Run(const std::string& source)
{
	// tokenize string
	const std::vector<Token> tokens = ScanTokens(source);

	// parse tokens
	Parser parser(tokens);
	const std::vector<Stmt*> statements = parser.parse();

	// Stop if there was a syntax error.
	if (m_hadError) { return; }

	// resolve variable names
	Resolver resolver(m_interpreter);
	resolver.resolve(statements);

	// Stop if there was a resolution error.
	if (m_hadError) { return; }

	// interpret
	m_interpreter.interpret(statements);
}

void Lox::Report(const int line, const std::string& where, const std::string& message)
{
	std::cout << "[line " << line << "] Error" << where << ": " << message << std::endl;
	m_hadError = true;
}

