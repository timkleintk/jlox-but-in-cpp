#pragma once

#include "token.h"


// nts: make one more abstraction so that expr and stmt are automatically handled correctly

// !! WHEN MAKING CHANGES TO TYPES, PLEASE MAKE SURE TO REEXPAND THE MACROS THROUGHOUT THIS FILE !!

#define EXPR_TYPES \
	TYPE(Assign, 2, Token, name, Expr*, value) \
    TYPE(Binary, 3, Expr*, left, Token, op, Expr*, right) \
	TYPE(Call, 3, Expr*, callee, Token, paren, std::vector<Expr*>, arguments) \
	TYPE(Get, 2, Expr*, object, Token, name) \
	TYPE(Grouping, 1, Expr*, expression) \
	TYPE(Literal, 1, Object, value) \
	TYPE(Logical, 3, Expr*, left, Token, op, Expr*, right) \
	TYPE(Set, 3, Expr*, object, Token, name, Expr*, value) \
	TYPE(Unary, 2, Token, op, Expr*, right) \
	TYPE(Variable, 1, Token, name)

#define STMT_TYPES \
	TYPE(Block, 1, std::vector<Stmt*>, statements) \
	TYPE(Class, 2, Token, name, std::vector<Stmt::Function>, methods) \
	TYPE(Expression, 1, Expr*, expression) \
	TYPE(Function, 3, Token, name, std::vector<Token>, params, std::vector<Stmt*>, body) \
	TYPE(If, 3, Expr*, condition, Stmt*, thenBranch, Stmt*, elseBranch) \
	TYPE(Print, 1, Expr*, expression) \
	TYPE(Return, 2, Token, keyword, Expr*, value) \
	TYPE(Var, 2, Token, name, Expr*, initializer) \
	TYPE(While, 2, Expr*, condition, Stmt*, body)


class Expr
{
public:
	// forward declaring of nested classes
#define TYPE(name, ...) class name;
	EXPR_TYPES
	#undef TYPE

		// visitor pattern
		class Visitor
	{
	public:
		virtual ~Visitor() = default;
	#define TYPE(name, ...) virtual void visit ##name ##Expr(name& expr, void* returnValue) = 0;
		EXPR_TYPES
		#undef TYPE
	};

	// visitor pattern
	virtual void accept(Visitor* visitor, void* returnValue) = 0;
};

class Stmt
{
public:
	// forward declaring of nested classes
#define TYPE(name, ...) class name;
	STMT_TYPES
	#undef TYPE

		// visitor pattern
		class Visitor
	{
	public:
		virtual ~Visitor() = default;
	#define TYPE(name, ...) virtual void visit ##name ##Stmt(name& expr) = 0;
		STMT_TYPES
		#undef TYPE
	};

	// visitor pattern
	virtual void accept(Visitor* visitor) = 0;
};

#define PARAMETER_LIST1(t0, n0) t0 n0
#define PARAMETER_LIST2(t0, n0, t1, n1) t0 n0, t1 n1
#define PARAMETER_LIST3(t0, n0, t1, n1, t2, n2) t0 n0, t1 n1, t2 n2

#define INITIALIZER_LIST1(t0, n0) n0(n0)
#define INITIALIZER_LIST2(t0, n0, t1, n1) n0(n0), n1(n1)
#define INITIALIZER_LIST3(t0, n0, t1, n1, t2, n2) n0(n0), n1(n1), n2(n2)

#define FIELDS1(t0, n0) t0 n0;
#define FIELDS2(t0, n0, t1, n1) t0 n0; t1 n1;
#define FIELDS3(t0, n0, t1, n1, t2, n2) t0 n0; t1 n1; t2 n2;

#define TYPE(name, nfields, ...) \
class Expr::name: public Expr \
{ \
	public: \
	name(PARAMETER_LIST ## nfields ## (__VA_ARGS__)): INITIALIZER_LIST ## nfields ## (__VA_ARGS__) {} \
	void accept(Visitor* visitor, void* returnValue) override { visitor->visit ## name ## Expr(*this, returnValue); } \
	FIELDS ## nfields ## (__VA_ARGS__) \
};

//EXPR_TYPES
class Expr::Assign: public Expr { public: Assign(Token name, Expr* value): name(name), value(value) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitAssignExpr(*this, returnValue); } Token name; Expr* value; }; class Expr::Binary: public Expr { public: Binary(Expr* left, Token op, Expr* right): left(left), op(op), right(right) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitBinaryExpr(*this, returnValue); } Expr* left; Token op; Expr* right; }; class Expr::Call: public Expr { public: Call(Expr* callee, Token paren, std::vector<Expr*> arguments): callee(callee), paren(paren), arguments(arguments) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitCallExpr(*this, returnValue); } Expr* callee; Token paren; std::vector<Expr*> arguments; }; class Expr::Get: public Expr { public: Get(Expr* object, Token name): object(object), name(name) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitGetExpr(*this, returnValue); } Expr* object; Token name; }; class Expr::Grouping: public Expr { public: Grouping(Expr* expression): expression(expression) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitGroupingExpr(*this, returnValue); } Expr* expression; }; class Expr::Literal: public Expr { public: Literal(Object value): value(value) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitLiteralExpr(*this, returnValue); } Object value; }; class Expr::Logical: public Expr { public: Logical(Expr* left, Token op, Expr* right): left(left), op(op), right(right) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitLogicalExpr(*this, returnValue); } Expr* left; Token op; Expr* right; }; class Expr::Set: public Expr { public: Set(Expr* object, Token name, Expr* value): object(object), name(name), value(value) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitSetExpr(*this, returnValue); } Expr* object; Token name; Expr* value; }; class Expr::Unary: public Expr { public: Unary(Token op, Expr* right): op(op), right(right) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitUnaryExpr(*this, returnValue); } Token op; Expr* right; }; class Expr::Variable: public Expr { public: Variable(Token name): name(name) {} void accept(Visitor* visitor, void* returnValue) override { visitor->visitVariableExpr(*this, returnValue); } Token name; };


#undef TYPE
#define TYPE(name, nfields, ...) \
class Stmt::name: public Stmt \
{ \
	public: \
	name(PARAMETER_LIST ## nfields ## (__VA_ARGS__)): INITIALIZER_LIST ## nfields ## (__VA_ARGS__) {} \
	void accept(Visitor* visitor) override { visitor->visit ## name ## Stmt(*this); } \
	FIELDS ## nfields ## (__VA_ARGS__) \
};

//STMT_TYPES
class Stmt::Block: public Stmt { public: Block(std::vector<Stmt*> statements): statements(statements) {} void accept(Visitor* visitor) override { visitor->visitBlockStmt(*this); } std::vector<Stmt*> statements; }; class Stmt::Class: public Stmt { public: Class(Token name, std::vector<Stmt::Function> methods): name(name), methods(methods) {} void accept(Visitor* visitor) override { visitor->visitClassStmt(*this); } Token name; std::vector<Stmt::Function> methods; }; class Stmt::Expression: public Stmt { public: Expression(Expr* expression): expression(expression) {} void accept(Visitor* visitor) override { visitor->visitExpressionStmt(*this); } Expr* expression; }; class Stmt::Function: public Stmt { public: Function(Token name, std::vector<Token> params, std::vector<Stmt*> body): name(name), params(params), body(body) {} void accept(Visitor* visitor) override { visitor->visitFunctionStmt(*this); } Token name; std::vector<Token> params; std::vector<Stmt*> body; }; class Stmt::If: public Stmt { public: If(Expr* condition, Stmt* thenBranch, Stmt* elseBranch): condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {} void accept(Visitor* visitor) override { visitor->visitIfStmt(*this); } Expr* condition; Stmt* thenBranch; Stmt* elseBranch; }; class Stmt::Print: public Stmt { public: Print(Expr* expression): expression(expression) {} void accept(Visitor* visitor) override { visitor->visitPrintStmt(*this); } Expr* expression; }; class Stmt::Return: public Stmt { public: Return(Token keyword, Expr* value): keyword(keyword), value(value) {} void accept(Visitor* visitor) override { visitor->visitReturnStmt(*this); } Token keyword; Expr* value; }; class Stmt::Var: public Stmt { public: Var(Token name, Expr* initializer): name(name), initializer(initializer) {} void accept(Visitor* visitor) override { visitor->visitVarStmt(*this); } Token name; Expr* initializer; }; class Stmt::While: public Stmt { public: While(Expr* condition, Stmt* body): condition(condition), body(body) {} void accept(Visitor* visitor) override { visitor->visitWhileStmt(*this); } Expr* condition; Stmt* body; };

#undef TYPE

#undef FIELDS3
#undef FIELDS2
#undef FIELDS1

#undef INITIALIZER_LIST3
#undef INITIALIZER_LIST2
#undef INITIALIZER_LIST1

#undef PARAMETER_LIST3
#undef PARAMETER_LIST2
#undef PARAMETER_LIST1
