#pragma once

#include <string>


//#define OBJECT_IS_ANY
#define OBJECT_IS_VARIANT

#if defined(OBJECT_IS_ANY)
#include <any>
using object_t = std::any;
#elif defined(OBJECT_IS_VARIANT)
#include <variant>
#include <any>

//class LoxFunction;
class LoxClass;
class LoxInstance;
class LoxCallable;

#include "loxFunction.h"
using object_t = std::variant<std::monostate, bool, double, std::string, LoxCallable*, LoxClass*, LoxInstance*, LoxFunction>;
#endif

// forward declarations --------------------------------------------

template<typename T>
bool is(const object_t& o);

template<typename T>
T as(const object_t& o);

std::string toString(const object_t& o);

bool isNull(const object_t& o);

// implementations -------------------------------------------------

#if defined(OBJECT_IS_ANY)

template<typename T>
bool is(const object_t& o)
{ return std::any_cast<T>(&o) != nullptr; }

template<typename T>
T as(const object_t& o)
{ return std::any_cast<T>(o); }

inline bool isNull(const object_t& o)
{ return !o.has_value(); }

#elif defined(OBJECT_IS_VARIANT)

template<typename T>
bool is(const object_t& o)
{
	if (std::holds_alternative<T>(o)) return true;

	if (std::holds_alternative<std::any>(o))
	{ return std::any_cast<T>(&std::get<std::any>(o)) != nullptr; }

	return false;
}

template <typename T>
T as(const object_t& o)
{
	if (std::holds_alternative<T>(o))
	{ return std::get<T>(o); }

	if (std::holds_alternative<std::any>(o))
	{
		return std::any_cast<T>(std::get<std::any>(o));
	}

	throw;
}

bool isNull(const object_t& o)
{ return std::holds_alternative<std::monostate>(o); }

#endif
