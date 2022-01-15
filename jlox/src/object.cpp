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
	if (is<LoxCallable*>(o))
	{
		LoxCallable* p = as<LoxCallable*>(o);
		if (const auto* pp = dynamic_cast<LoxFunction*>(p))
		{
			return "<fn " + pp->getDeclaration()->name.lexeme + ">";
		}
		return "<native fn>";
	}
	if (is<LoxClass*>(o)) return as<LoxClass*>(o)->name;
	if (is<LoxInstance*>(o)) return as<LoxInstance*>(o)->getClass().name + " instance";
	if (is<LoxFunction>(o)) return "<LoxFunction>";
	if (is<LoxClass>(o)) return "<LoxClass " + as<LoxClass>(o).name + ">";
	if (is<LoxInstance>(o)) return "<LoxInstance" " of " + as<LoxInstance>(o).getClass().name + ">";

	return R"(<???>)";
}

bool IsTruthy(const object_t& object)
{
	if (isNull(object)) return false;
	if (is<bool>(object)) return as<bool>(object);
	return true;
}

#define EqualCheck(T) if (is<T>(a)) return as<T>(a) == as<T>(b) 
bool IsEqual(const object_t& a, const object_t& b)
{
	if (!a.has_value() && !b.has_value()) { return true; } // if both nil
	if (!a.has_value()) { return false; }                  // if only one operand is nil
	if (a.type() != b.type()) { return false; }            // a and b should be the same type

	EqualCheck(bool);
	EqualCheck(std::string);
	EqualCheck(double);
	EqualCheck(LoxClass*);
	EqualCheck(LoxInstance*);
	EqualCheck(LoxCallable*);
	EqualCheck(LoxFunction);


	// bug: not fully implemented
	printf("Warning: tried to compare two values of unknown type. Known types: nil, bool, string, number, class.\n");

	return false;
}
#undef EqualCheck
