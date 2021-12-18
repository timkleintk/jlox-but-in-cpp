#include "loxCallable.h"

#include "object.h"

LoxCallable::LoxCallable(): m_arity(-1), m_call([](Interpreter*, const std::vector<Object>&) { return Object::Nil(); })
{
}

LoxCallable::LoxCallable(const std::function<Object(Interpreter*, std::vector<Object>)>& call, const int arity): m_arity(arity), m_call(call)
{}

LoxCallable::~LoxCallable() = default;

Object LoxCallable::call(Interpreter* interpreter, std::vector<Object> arguments) const
{ return m_call(interpreter, std::move(arguments)); }

int LoxCallable::arity() const
{ return m_arity; }

std::unique_ptr<LoxCallable> LoxCallable::GetCopy()
{
	return std::make_unique<LoxCallable>(m_call, m_arity);
}

std::string LoxCallable::toString()
{
	return "<fn>";
}
