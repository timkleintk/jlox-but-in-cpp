#include "object.h"

#include "loxClass.h"
#include "loxFunction.h"
#include "loxInstance.h"

std::string toString(const object_t& o)
{
	if (IsNull(o)) return "nil";
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
		LoxCallable* p;
		p = as<std::shared_ptr<LoxCallable>>(o).get();
		if (const auto* pp = dynamic_cast<LoxFunction*>(p); pp != nullptr)
		{
			return "<fn " + pp->getDeclaration()->name.lexeme + ">";
		}
		return "<native fn>";
	}
	if (is<std::shared_ptr<LoxClass>>(o)) return as<std::shared_ptr<LoxClass>>(o)->name;
	if (is<std::shared_ptr<LoxInstance>>(o)) return as<std::shared_ptr<LoxInstance>>(o)->getClass().name + " instance";
	if (is<std::shared_ptr<LoxFunction>>(o)) return "<LoxFunction>";

	return R"(<???>)";
}

bool IsNull(const object_t& o)
{
#if defined(OBJECT_IS_ANY)
	return !o.has_value();
#elif defined(OBJECT_IS_VARIANT)
	return std::holds_alternative<std::monostate>(o);
#endif
}

bool IsEqual(const object_t& a, const object_t& b)
{
#if defined(OBJECT_IS_ANY)
#define EqualCheck(Type) if (is<Type>(a)) return as<Type>(a) == as<Type>(b) 
	if (IsNull(a) && IsNull(b)) { return true; } // if both nil
	if (IsNull(a)) { return false; }             // if only one operand is nil
	if (a.type() != b.type()) { return false; }  // a and b should be the same type

	EqualCheck(bool);
	EqualCheck(std::string);
	EqualCheck(double);
	EqualCheck(std::shared_ptr<LoxClass>);
	EqualCheck(std::shared_ptr<LoxInstance>);
	EqualCheck(std::shared_ptr<LoxCallable>);
	EqualCheck(std::shared_ptr<LoxFunction>);

	// bug: not fully implemented
	printf("Warning: tried to compare two values of unknown type. Known types: nil, bool, string, number, class.\n");

	return false;
#undef EqualCheck
#elif defined(OBJECT_IS_VARIANT)
	return a == b;
#endif
}

bool IsTruthy(const object_t& object)
{
	if (IsNull(object)) return false;
	if (is<bool>(object)) return as<bool>(object);
	return true;
}
