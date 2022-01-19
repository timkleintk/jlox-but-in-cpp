#include "object.h"

#include "loxClass.h"
#include "loxFunction.h"
#include "loxInstance.h"

std::string toString(const object_t& o)
{
	if (isNull(o)) return "nil";
	if (is<std::string>(o)) return as<std::string>(o);
	if (is<double>(o))
	{
		std::string str = std::to_string(as<double>(o));

		// trim lagging zeroes
		size_t lastNonZero = str.find_last_not_of('0');
		if (lastNonZero != std::string::npos)
		{
			if (str[lastNonZero] == '.') lastNonZero--; // trim trailing '.'
			str.erase(lastNonZero + 1, str.size() - lastNonZero - 1);
		}

		return str;
	}
	if (is<bool>(o)) return as<bool>(o) ? "true" : "false";
	if (is<std::shared_ptr<LoxCallable>>(o))
	{
		LoxCallable* p = as<std::shared_ptr<LoxCallable>>(o).get(); // nts: dangling?
		if (const auto* pp = dynamic_cast<LoxFunction*>(p); pp != nullptr)
		{
			return "<fn " + pp->getDeclaration()->name.lexeme + ">";
		}
		return "<native fn>";
	}
	if (is<std::shared_ptr<LoxClass>>(o)) return as<std::shared_ptr<LoxClass>>(o)->name;
	if (is<std::shared_ptr<LoxInstance>>(o)) return as<std::shared_ptr<LoxInstance>>(o)->getClass().name + " instance";
	if (is<LoxFunction>(o)) return "<LoxFunction>";

	return R"(<???>)";
}

bool IsTruthy(const object_t& object)
{
	if (isNull(object)) return false;
	if (is<bool>(object)) return as<bool>(object);
	return true;
}

#define EqualCheck(Type) if (is<Type>(a)) return as<Type>(a) == as<Type>(b) 
bool IsEqual(const object_t& a, const object_t& b)
{
	if (!a.has_value() && !b.has_value()) { return true; } // if both nil
	if (!a.has_value()) { return false; }                  // if only one operand is nil
	if (a.type() != b.type()) { return false; }            // a and b should be the same type

	EqualCheck(bool);
	EqualCheck(std::string);
	EqualCheck(double);
	EqualCheck(std::shared_ptr<LoxClass>);
	EqualCheck(std::shared_ptr<LoxInstance>);
	EqualCheck(std::shared_ptr<LoxCallable>);
	EqualCheck(LoxFunction);


	// bug: not fully implemented
	printf("Warning: tried to compare two values of unknown type. Known types: nil, bool, string, number, class.\n");

	return false;
}
#undef EqualCheck
