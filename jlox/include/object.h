/*
#pragma once
#include <memory>
#include <string>

//#include "loxCallable.h"
//#include "loxClass.h"
#include "loxInstance.h"


class LoxInstance;
class LoxClass;
class LoxCallable;

class Object
{
public:
	enum class Type { UNINITIALIZED, BOOL, STRING, NUMBER, NIL, CALLABLE, CLASS, INSTANCE };

	// constructors ------------------------------------------------
	Object();
	Object(bool value);
	Object(double value);
	Object(int value);
	Object(std::string value);
	static Object Nil();
	Object(std::unique_ptr<LoxCallable>&& value);
	Object(std::unique_ptr<LoxClass>&& value);
	Object(std::unique_ptr<LoxInstance>&& value);

	// copy constructor
	Object(const Object& object);

	// assignment operator
	Object& operator=(const Object& object);
	//Object& operator=(const Object& object);


	// output
	std::string AsString() const;

	bool equals(const Object& other) const;

	// for the loxfunction
	//Object& bind(LoxInstance* loxInstance);

	Type type;

	// TODO: make this a union?
	bool boolean = false;
	std::string string;
	double number = 0;
	std::unique_ptr<LoxCallable> callable;
	std::unique_ptr<LoxClass> klass;
	std::unique_ptr<LoxInstance> instance;
};

*/