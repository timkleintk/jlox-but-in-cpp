#pragma once
#include <memory>
#include <string>

#include "loxCallable.h"


class Object
{
public:
	enum class Type { UNINITIALIZED, BOOL, STRING, NUMBER, NIL, CALLABLE };

	// constructors ------------------------------------------------
	Object();
	Object(bool value);
	Object(double value);
	Object(int value);
	Object(std::string value);
	static Object Nil();
	Object(std::unique_ptr<LoxCallable>&& value);
	//Object(std::unique_ptr<LoxCallable>&& callable);


	// copy constructor
	Object(const Object& object);

	// assignment operator
	Object& operator=(const Object& object);
	//Object& operator=(const Object& object);


	// output
	std::string AsString() const;

	bool equals(const Object& other) const;
	Type type;

	// TODO: make this a union?
	bool boolean = false;
	std::string string;
	double number = 0;
	std::unique_ptr<LoxCallable> callable;
};

