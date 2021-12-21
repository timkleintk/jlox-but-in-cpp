#include "object.h"

#include "loxCallable.h"
#include "loxClass.h"
#include "loxInstance.h"

Object::Object(): type(Type::UNINITIALIZED)
{}

Object::Object(const bool value) : type(Type::BOOL), boolean(value)
{}

Object::Object(const double value) : type(Type::NUMBER), number(value)
{}

Object::Object(const int value) : type(Type::NUMBER), number(value)
{}

Object::Object(std::string value) : type(Type::STRING), string(std::move(value))
{}

Object Object::Nil()
{ Object nil; nil.type = Type::NIL; return nil; }

Object::Object(std::unique_ptr<LoxCallable>&& value): type(Type::CALLABLE), callable(std::move(value))
{}

Object::Object(std::unique_ptr<LoxClass>&& value) : type(Type::CLASS), klass(std::move(value))
{}

Object::Object(std::unique_ptr<LoxInstance>&& value): type(Type::INSTANCE), instance(std::move(value))
{}


Object::Object(const Object& object)
{
	type = object.type;

	switch (object.type)
	{
	case Type::UNINITIALIZED: break;
	case Type::BOOL: boolean = object.boolean; break;
	case Type::STRING: string = object.string; break;
	case Type::NUMBER: number = object.number; break;
	case Type::NIL: break;
	case Type::CALLABLE: callable = object.callable->GetCopy(); break;
	case Type::CLASS: klass = std::unique_ptr<LoxClass>(static_cast<LoxClass*>(object.klass->GetCopy().release())); break;
	case Type::INSTANCE: instance = std::make_unique<LoxInstance>(*object.instance); break;
	default:
		throw std::exception("Copy constructed 'Object' with unknown type");
	}
}

Object& Object::operator=(const Object& object)
{
	type = object.type;

	switch (object.type)
	{
	case Type::UNINITIALIZED: break;
	case Type::BOOL: boolean = object.boolean; break;
	case Type::STRING: string = object.string; break;
	case Type::NUMBER: number = object.number; break;
	case Type::NIL: break;
	case Type::CALLABLE: callable = object.callable->GetCopy(); break;
	case Type::CLASS: klass = std::make_unique<LoxClass>(*object.klass); break;
	case Type::INSTANCE: instance = std::make_unique<LoxInstance>(*object.instance); break;
	default:
		throw std::exception("assignment operator for 'Object' with unknown type");
	}

	return *this;
}


std::string Object::AsString() const
{
	switch (type)
	{
	case Type::UNINITIALIZED: return "uninitialized";
	case Type::BOOL: return boolean ? "true" : "false";
	case Type::STRING: return string;
	case Type::NUMBER: return std::to_string(number);
	case Type::NIL: return "nil";
	case Type::CALLABLE: return callable->toString();
	case Type::CLASS: return klass->toString();
	case Type::INSTANCE: return instance->toString();
	default: return "something went wrong here";
	}
}

bool Object::equals(const Object& other) const
{
	if (type == other.type)
	{
		switch (type)
		{
		case Type::UNINITIALIZED: return true;
		case Type::BOOL: return other.boolean == boolean;
		case Type::STRING: return other.string == string;
		case Type::NUMBER: return other.number == number;
		case Type::NIL: return true;
		//case Type::CALLABLE: return other.lcallable == lcallable;
		default: return false;
		}
	}
	return false;
}
