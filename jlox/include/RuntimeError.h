#pragma once
#include <stdexcept>

#include "token.h"

class RuntimeError final : public std::runtime_error
{
public:

	RuntimeError(Token token, const std::string& message) : std::runtime_error(message), token(std::move(token))
	{}

	Token token;
};


