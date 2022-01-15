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
		// nts: not elegant
		if (strcmp(argv[1], "test") == 0)
		{
			Lox::RunPrompt(false);
		}
		else
		{
			Lox::RunFile(argv[1]);
		}
	}
	else
	{
		Lox::RunPrompt();
	}

	return 0;
}

