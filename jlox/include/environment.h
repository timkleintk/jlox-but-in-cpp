#pragma once
#include <any>
#include <string>
#include <unordered_map>


class Token;

class Environment
{
public:
	explicit Environment(Environment* enclosing = nullptr);
	Environment(const Environment&) = delete;
	~Environment() = default;

	Environment* enclosing = nullptr;


	std::any get(const Token& name);

	void define(const std::string& name, const std::any& value);

	Environment& ancestor(const int distance);

	std::any getAt(const int distance, const std::string& name);

	void assignAt(const int distance, const Token& name, const std::any& value);

	void assign(const Token& name, const std::any& value);

private:
	std::unordered_map<std::string, std::any> m_values;
};


