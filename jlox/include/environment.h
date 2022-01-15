#pragma once
#include <string>
#include <unordered_map>

#include "object.h"


class Token;

class Environment
{
public:
	explicit Environment(Environment* enclosing = nullptr);

	Environment* getEnclosing() const;

	object_t get(const Token& name);
	object_t getAt(size_t distance, const std::string& name);

	void assign(const Token& name, object_t value);
	void assignAt(size_t distance, const Token& name, object_t value);

	void define(const std::string& name, object_t value);


	void debugPrint() const;

private:
	Environment& ancestor(const size_t distance);

	Environment* m_enclosing = nullptr;
	std::unordered_map<std::string, object_t> m_values;
};


