#pragma once


class Return final : public std::exception
{
public:
	Return(const object_t& value):value(value) {}
	object_t value;
};
