#pragma once

#include <vector>
#include "object.h"
class Interpreter;

class LoxCallable
{
public:
	LoxCallable() = default;
	virtual ~LoxCallable() = default;

	virtual object_t call(Interpreter* interpreter, std::vector<object_t> arguments) const = 0;
	virtual size_t arity() const = 0;
};
