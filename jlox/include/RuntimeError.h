#pragma once
#include <stdexcept>

#include <utility>

#include "token.h"

class RuntimeError : public std::runtime_error
{
public:
	Token token;

	RuntimeError(Token token, const std::string& message): std::runtime_error(message), token(std::move(token))
	{
	}

private:

};


