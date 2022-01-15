#pragma once

class Return final : public std::exception
{
public:
	Return(object_t value):value(std::move(value)) {}

	object_t value;
};
