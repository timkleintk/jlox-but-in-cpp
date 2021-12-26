#pragma once

#include <string>

#define OBJECT_IS_ANY

#if defined(OBJECT_IS_ANY)
#include <any>
using object_t = std::any;
#endif

// forward declarations --------------------------------------------

template<typename T>
bool is(const object_t& o);

template<typename T>
T as(const object_t& o);

std::string toString(const object_t& o);

bool isNull(const object_t& o);

#if defined(OBJECT_IS_ANY)

template<typename T>
bool is(const object_t& o)
{ return std::any_cast<T>(&o) != nullptr; }

template<typename T>
T as(const object_t& o)
{ return std::any_cast<T>(o); }

inline bool isNull(const object_t& o)
{ return !o.has_value(); }


#endif
