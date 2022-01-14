#pragma once

#include "interpreter.h"

class RuntimeError;
class Token;

class Lox
{
public:
	static void RunFile(const char* path);

	static void RunPrompt(bool qualityOfLife = true);

	static void Error(const Token& token, const std::string& message);

	static void Error(size_t line, const std::string& message);

	static void runtimeError(const RuntimeError& error);

	static void ResetInterpreter();

private:
	static Interpreter m_interpreter;

	static bool m_hadError;
	static bool m_hadRuntimeError;

	static void Run(const std::string& source);

	static void Report(size_t line, const std::string& where, const std::string& message);

	static void PrintStatus();
};
