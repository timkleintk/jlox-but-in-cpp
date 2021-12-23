#pragma once
#include <string>
#include <unordered_map>


class Object;
class Token;

class Environment
{
public:
	explicit Environment(Environment* enclosing = nullptr);
	Environment(const Environment&) = delete;

	Environment* enclosing = nullptr;


	Object get(const Token& name);

	void define(const std::string& name, const Object& value);

	Environment& ancestor(const int distance);

	Object getAt(const int distance, const std::string& name);

	void assignAt(const int distance, const Token& name, const Object& value);

	void assign(const Token& name, const Object& value);

private:
	std::unordered_map<std::string, Object> m_values;
};


