#pragma once

#include <string>

//#define OBJECT_IS_ANY

#if !defined(OBJEC_IS_ANY)
#define OBJECT_IS_VARIANT
#endif

#if defined(OBJECT_IS_ANY)
#include <any>
using object_t = std::any;

#elif defined(OBJECT_IS_VARIANT)
#include <memory>
#include <variant>

class LoxFunction;
class LoxClass;
class LoxInstance;
class LoxCallable;

using object_t = std::variant
<
	std::monostate,
	bool,
	double,
	std::string,
	std::shared_ptr<LoxCallable>,
	std::shared_ptr<LoxClass>,
	std::shared_ptr<LoxInstance>,
	std::shared_ptr<LoxFunction>
>;
#endif


// forward declarations --------------------------------------------

template<typename T>
bool is(const object_t& o);

template<typename T>
T as(const object_t& o);

std::string toString(const object_t& o);

bool IsNull(const object_t& o);

bool IsEqual(const object_t& a, const object_t& b);

bool IsTruthy(const object_t& object);


// implementations -------------------------------------------------


template<typename T>
bool is(const object_t& o)
{
#if defined(OBJECT_IS_ANY)
	return std::any_cast<T>(&o) != nullptr;
#elif defined(OBJECT_IS_VARIANT)
	return std::holds_alternative<T>(o);
#endif
}

template<typename T>
T as(const object_t& o)
{
#if defined(OBJECT_IS_ANY)
	return std::any_cast<T>(o);
#elif defined(OBJECT_IS_VARIANT)
	return std::get<T>(o);
#endif
}

