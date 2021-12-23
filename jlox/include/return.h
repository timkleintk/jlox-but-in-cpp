#pragma once
#include <any>

class Return final : public std::exception
{
public:
	Return(const std::any& value):value(value) {}
	std::any value;
};
