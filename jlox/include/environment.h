#pragma once
#include <string>
#include <unordered_map>

#include "object.h"


class Token;

class Environment
{
public:
	explicit Environment(Environment* enclosing = nullptr);
	~Environment();

	object_t get(const Token& name);

	void define(const std::string& name, const object_t& value);

	object_t getAt(int distance, const std::string& name);

	void assignAt(int distance, const Token& name, const object_t& value);

	void assign(const Token& name, const object_t& value);

	Environment* getEnclosing() const { return m_enclosing; }

	void debugPrint() const;

private:
	Environment& ancestor(int distance);

	Environment* m_enclosing = nullptr;
	std::unordered_map<std::string, object_t> m_values;
};


