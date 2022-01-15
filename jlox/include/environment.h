#pragma once
#include <string>
#include <unordered_map>

#include "garbageCollector.h"
#include "object.h"


class Token;

class Environment : public GarbageCollectable<Environment>
{
public:
	explicit Environment(std::shared_ptr<Environment> enclosing = nullptr);

	std::shared_ptr<Environment> getEnclosing() const;

	object_t get(const Token& name);
	object_t getAt(size_t distance, const std::string& name);

	void assign(const Token& name, object_t value);
	void assignAt(size_t distance, const Token& name, object_t value);

	void define(const std::string& name, object_t value);


	void debugPrint() const;

private:
	Environment& ancestor(size_t distance);

	std::shared_ptr<Environment> m_enclosing = nullptr;
	std::unordered_map<std::string, object_t> m_values;
};


