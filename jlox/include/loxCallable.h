#pragma once
#include <functional>

class Interpreter;
class Object;

//using loxFuc = 

class LoxCallable
{
public:
	LoxCallable();
	LoxCallable(const std::function<Object(Interpreter*, std::vector<Object>)>& call, int arity);
	virtual ~LoxCallable();

	virtual Object call(Interpreter* interpreter, std::vector<Object> arguments) const;
	[[nodiscard]] virtual int arity() const;

	virtual std::unique_ptr<LoxCallable> GetCopy();

	virtual std::string toString();

private:

	int m_arity;
	const std::function<Object(Interpreter*, std::vector<Object>)>& m_call;
};
