#pragma once
#include <any>
#include <functional>

class Interpreter;



class LoxCallable
{
public:
	LoxCallable();
	virtual ~LoxCallable();
	//LoxCallable(const std::function<Object(Interpreter*, std::vector<Object>)>& call, int arity);

	virtual std::any call(Interpreter* interpreter, std::vector<std::any> arguments) const;
	[[nodiscard]] virtual int arity() const;

	//virtual std::unique_ptr<LoxCallable> GetCopy();

	//virtual std::string toString();

private:
	//int m_arity;
	//std::function<Object(Interpreter*, std::vector<Object>)> m_call;
};
