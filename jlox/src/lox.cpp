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

// nts: better documentation here
// returns wether or not there is enough in the source string to craft a full statement.
bool IsSourceComplete(const std::string& src)
{
	std::stack<char> braceBalance;
	char previous = 0;
	bool inComment = false;
	bool inString = false;

	// three newlines at the end should return true
	if (src.size() >= 3 && src.substr(src.size() - 3) == "\n\n\n") return true;

	for (const auto& c : src)
	{
		if (inString)
		{
			if (c == '"') inString = false;
		}
		else if (inComment)
		{
			if (c == '\n') inComment = false;
		}
		else
		{
			switch (c)
			{
			case '/':
				// peek second slash
				if (*(&c + 1) == '/') { inComment = true; }
				continue;
			case '"':
				inString = true;
				break;
			case '{':
			case '(':
				braceBalance.push(c);
				break;
			case '}':
			case ')':
			{
				const char complement = c == '}' ? '{' : '(';
				if (!braceBalance.empty() && braceBalance.top() == complement)
				{
					braceBalance.pop();
				}
				else
				{
					return true;
				}
				break;
			}
			default:
				break;
			}

			if (!std::isspace(c)) previous = c;
		}
	}

	if (inString) return false;
	if (braceBalance.empty())
	{
		if (previous == '}' || previous == ';' || previous == '\0') return true;
	}

	return false;
}

void Lox::RunPrompt(const bool qualityOfLife)
{
	std::string source;
	std::string line;

	do
	{
		if (qualityOfLife)
		{
			std::cout << "> ";
		}

		// get input
		std::getline(std::cin, source);

		// special commands
		if (std::cin.eof() || source == "exit") { break; }
		if (source == "clear") { system("CLS"); continue; }
		if (source == "status") { PrintStatus(); continue; }

		// multiline inputs
		while (!IsSourceComplete(source))
		{
			if (qualityOfLife)
			{
				std::cout << "  ";
			}
			std::getline(std::cin, line);
			source += "\n";
			source += line;
		}

		// interpret
		Run(source);

		// for unit testing
		if (!qualityOfLife)
		{
			if (m_hadError) { exit(65); }
			if (m_hadRuntimeError) { exit(70); }
		}

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

void Lox::ResetInterpreter()
{
	m_interpreter = Interpreter();
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
	const std::vector<std::shared_ptr<Stmt>> statements = parser.parse();

	// debug print statements

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
	std::cerr << "[line " << line << "] Error" << where << ": " << message << "\n";
	m_hadError = true;
}

void Lox::PrintStatus()
{
	
	// globals
	m_interpreter.globals.debugPrint();

	// locals
}

