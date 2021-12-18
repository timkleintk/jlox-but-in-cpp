#pragma once
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <string>

#include "expr.h"



class AstPrinter : public Expr::Visitor
{
public:
	std::string print(Expr* expr)
	{
		std::string result;
		expr->accept(this, &result);
		return result;
	}

	// pretty print syntax tree
	std::string parenthesize(const std::string& name, int count, ...)
	{
		std::string result = "(" + name;

		va_list args;
		va_start(args, count);

		for (int i = 0; i < count; i++)
		{
			Expr* expr = va_arg(args, Expr*);

			std::string r;
			expr->accept(this, &r);

			result += " ";
			result += r;
		}

		va_end(args);

		result += ")";

		return result;
	}

	// syntactic niceties
	std::string parenthesize(const std::string& name, Expr* e0) { return parenthesize(name, 1, e0); }
	std::string parenthesize(const std::string& name, Expr* e0, Expr* e1) { return parenthesize(name, 2, e0, e1); }
	std::string parenthesize(const std::string& name, Expr* e0, Expr* e1, Expr* e2) { return parenthesize(name, 3, e0, e1, e2); }

	void Test()
	{
		// nts: MASSIVE MEMORY LEAK
		auto expression =
			Expr::Binary(
				new Expr::Unary(
					Token(MINUS, "-", Object(), 1),
					new Expr::Literal(Object(static_cast<double>(123)))
				),
				Token(STAR, "*", {}, 1),
				new Expr::Grouping(
					new Expr::Literal(Object(45.67))
				)
			);

		std::cout << print(&expression);
	}

	// we need to implement all visit methods
#define RETURN *static_cast<std::string*>(returnValue) =
	void visitBinaryExpr(Expr::Binary& expr, void* returnValue) override
	{
		RETURN parenthesize(expr.op.lexeme, expr.left, expr.right);
	}

	void visitGroupingExpr(Expr::Grouping& expr, void* returnValue) override
	{
		RETURN parenthesize("group", expr.expression);
	}

	void visitLiteralExpr(Expr::Literal& expr, void* returnValue) override
	{
		RETURN isnan(expr.value.number) ? expr.value.string : std::to_string(expr.value.number);
	}

	void visitUnaryExpr(Expr::Unary& expr, void* returnValue) override
	{
		RETURN parenthesize(expr.op.lexeme, expr.right);
	}

};