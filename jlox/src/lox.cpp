// https://craftinginterpreters.com/scanning.html#the-interpreter-framework

#include "lox.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "astPrinter.h"
#include "parser.h"
#include "RuntimeError.h"
#include "scanner.h"

// Class Lox -------------------------------------------------------


// Public: ---------------------------------------------------------

void Lox::RunFile(const char* path)
{
	// https://stackoverflow.com/questions/18398167/how-to-copy-a-txt-file-to-a-char-array-in-c/18398230

	if (std::ifstream inputStream(path); inputStream.is_open())
	{
		const std::string source((std::istreambuf_iterator(inputStream)), std::istreambuf_iterator<char>());
		Run(source);

		if (hadError) { exit(65); }
		if (hadRuntimeError) { exit(70); }
	}
	//nts: add error handling
}

void Lox::RunPrompt()
{
	// TODO: make this pretty
	std::string line;

	std::cout << "> ";
	std::getline(std::cin, line);

	while (!std::cin.eof() && line != "exit")
	{
		Run(line);
		hadError = false;

		std::cout << "> ";
		std::getline(std::cin, line);
	}
}

void Lox::Error(const int line, const std::string& message)
{
	Report(line, "", message);
}

void Lox::runtimeError(RuntimeError& error)
{
	std::cerr << error.what() << std::endl << "[line " << error.token.line << "]" << std::endl;
	hadRuntimeError = true;
}

void Lox::Error(const Token& token, const std::string& message)
{
	if (token.type == TokenType::END_OF_FILE)
	{
		Report(token.line, " at end", message);
	}
	else
	{
		Report(token.line, " at '" + token.lexeme + "'", message);
	}
}


// Private: --------------------------------------------------------

Interpreter Lox::interpreter = Interpreter();

bool Lox::hadError = false;
bool Lox::hadRuntimeError = false;

void Lox::Run(const std::string& source)
{
	const std::vector<Token> tokens = ScanTokens(source);

	const std::vector<Stmt*> statements = ParseTokens(tokens);

	// Stop if there was a syntax error.
	if (hadError) { return; }

	interpreter.interpret(statements);
}

void Lox::Report(const int line, const std::string& where, const std::string& message)
{
	std::cout << "[line " << line << "] Error" << where << ": " << message << std::endl;
	hadError = true;
}

