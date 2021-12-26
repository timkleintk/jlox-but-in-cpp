#include "object.h"

#include <iostream>
#include <sstream>

#include "loxCallable.h"
#include "loxClass.h"
#include "loxFunction.h"
#include "loxInstance.h"

#if defined(OBJECT_IS_ANY)

#define s(x) #x
#define xstr(x) s(x )

#define checkPtrType(type) \
if (is<type*>(o)) return xstr(<type*) " to " + (std::stringstream() << std::hex << as<type*>(o)).str() + ">"



std::string toString(const object_t& o)
{
	if (isNull(o)) return "Nill";
	if (is<std::string>(o)) return as<std::string>(o);
	if (is<double>(o)) return std::to_string(as<double>(o));
	if (is<bool>(o)) return as<bool>(o) ? "true" : "false";

	checkPtrType(LoxCallable);
	checkPtrType(LoxFunction);
	checkPtrType(LoxClass);
	checkPtrType(LoxInstance);

	if (is<LoxFunction>(o)) return "<LoxFunction>";
	if (is<LoxClass>(o)) return "<LoxClass " + as<LoxClass>(o).name + ">";
	if (is<LoxInstance>(o)) return "<LoxInstance" " of " + as<LoxInstance>(o).getClass().name + ">";

	return "<???>";
}

#endif