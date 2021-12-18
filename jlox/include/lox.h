#pragma once

#include "interpreter.h"

#include <string>

class RuntimeError;
class Token;

class Lox
{
public:
	static void RunFile(const char* path);

	static void RunPrompt();

	static void Error(const Token& token, const std::string& message);

	static void Error(int line, const std::string& message);

	static void runtimeError(RuntimeError& error);

private:
	static Interpreter interpreter;

	static bool hadError;
	static bool hadRuntimeError;

	static void Run(const std::string& source);

	static void Report(int line, const std::string& where, const std::string& message);

};
