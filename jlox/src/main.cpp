// https://craftinginterpreters.com/scanning.html#the-interpreter-framework

#include <iostream>

#include "lox.h"

int main(const int argc, char** argv)
{

	if (argc > 2)
	{
		std::cout << "Usage: jlox [script]";
		return 64;
	}

	if (argc == 2)
	{
		Lox::RunFile(argv[1]);
	}
	else
	{
		Lox::RunPrompt();
	}

	return 0;
}

