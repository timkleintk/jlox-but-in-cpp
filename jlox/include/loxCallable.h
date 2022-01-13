#pragma once

#include <vector>

class Interpreter;

class LoxCallable
{
public:
	LoxCallable() = default;
	virtual ~LoxCallable() = default;

	virtual object_t call(Interpreter* interpreter, std::vector<object_t> arguments) const = 0;
	virtual int arity() const = 0;
};
