#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "loxCallable.h"

class LoxFunction;

class LoxClass final : public LoxCallable
{
public:
	LoxClass(std::string name, std::shared_ptr<LoxClass> superclass, std::unordered_map<std::string, LoxFunction> methods);
	// nts: is this needed?
	LoxClass& operator=(const LoxClass&) = delete;

	bool operator==(const LoxClass& as) const;

	std::optional<LoxFunction> findMethod(const std::string& methodName) const;

	object_t call(Interpreter* interpreter, const std::vector<object_t>& arguments) const override;
	size_t arity() const override;

	std::string name;
	std::shared_ptr<LoxClass> m_superclass = nullptr;
private:
	std::unordered_map<std::string, LoxFunction> m_methods;
};

