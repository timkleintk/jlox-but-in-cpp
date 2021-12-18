#pragma once
#include "object.h"

class Return final : public std::exception
{
public:
	Return(const Object& value):value(value) {}
	Object value;
};
