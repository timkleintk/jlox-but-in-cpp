#pragma once

#include <vector>

#include "garbageCollector.h"
#include "object.h"

class Interpreter;

class LoxCallable : public GarbageCollectable<LoxCallable>
{
public:
	LoxCallable() = default;
	~LoxCallable() override = default;

	virtual object_t call(Interpreter* interpreter, const std::vector<object_t>& arguments) const = 0;
	virtual size_t arity() const = 0;
};
