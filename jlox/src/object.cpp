#include "object.h"

#include <sstream>

#include "loxCallable.h"
#include "loxClass.h"
#include "loxFunction.h"
#include "loxInstance.h"

#if defined(OBJECT_IS_ANY)

#define s(x) #x
#define xstr(x) s(x )

// mainly for debugging purpose
#define checkPtrType(type) \
if (is<type*>(o)) return xstr(<type*) " to " + (std::stringstream() << std::hex << as<type*>(o)).str() + ">"


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


	//checkPtrType(LoxCallable);
	if (is<LoxCallable*>(o))
	{
		LoxCallable* p = as<LoxCallable*>(o);
		if (const auto* pp = dynamic_cast<LoxFunction*>(p))
		{
			return "<fn " + pp->getDeclaration()->name.lexeme + ">";
		}
		return "<native fn>";
	}

	checkPtrType(LoxFunction);
	//checkPtrType(LoxClass);
	if (is<LoxClass*>(o)) return as<LoxClass*>(o)->name;
	//checkPtrType(LoxInstance);
	if (is<LoxInstance*>(o)) return as<LoxInstance*>(o)->getClass().name + " instance";

	if (is<LoxFunction>(o)) return "<LoxFunction>";
	if (is<LoxClass>(o)) return "<LoxClass " + as<LoxClass>(o).name + ">";
	if (is<LoxInstance>(o)) return "<LoxInstance" " of " + as<LoxInstance>(o).getClass().name + ">";

	return R"(<???>)";
}

#endif