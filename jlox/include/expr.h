#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "object.h"
#include "token.h"


// !! WHEN MAKING CHANGES TO TYPES, PLEASE MAKE SURE TO REEXPAND THE MACROS THROUGHOUT THIS FILE !!

#define EXPR_TYPES \
	TYPE(Assign, 2, Token, name, std::shared_ptr<Expr>, value) \
    TYPE(Binary, 3, std::shared_ptr<Expr>, left, Token, op, std::shared_ptr<Expr>, right) \
	TYPE(Call, 3, std::shared_ptr<Expr>, callee, Token, paren, std::vector<std::shared_ptr<Expr>>, arguments) \
	TYPE(Get, 2, std::shared_ptr<Expr>, object, Token, name) \
	TYPE(Grouping, 1, std::shared_ptr<Expr>, expression) \
	TYPE(Literal, 1, object_t, value) \
	TYPE(Logical, 3, std::shared_ptr<Expr>, left, Token, op, std::shared_ptr<Expr>, right) \
	TYPE(Set, 3, std::shared_ptr<Expr>, object, Token, name, std::shared_ptr<Expr>, value) \
	TYPE(Super, 2, Token, keyword, Token, method) \
	TYPE(This, 1, Token, keyword) \
	TYPE(Unary, 2, Token, op, std::shared_ptr<Expr>, right) \
	TYPE(Variable, 1, Token, name)

#define STMT_TYPES \
	TYPE(Block, 1, std::vector<std::shared_ptr<Stmt>>, statements) \
	TYPE(Class, 3, Token, name, std::shared_ptr<Expr::Variable>, superclass, std::vector<std::shared_ptr<Stmt::Function>>, methods) \
	TYPE(Expression, 1, std::shared_ptr<Expr>, expression) \
	TYPE(Function, 3, Token, name, std::vector<Token>, params, std::vector<std::shared_ptr<Stmt>>, body) \
	TYPE(If, 3, std::shared_ptr<Expr>, condition, std::shared_ptr<Stmt>, thenBranch, std::shared_ptr<Stmt>, elseBranch) \
	TYPE(Print, 1, std::shared_ptr<Expr>, expression) \
	TYPE(Return, 2, Token, keyword, std::shared_ptr<Expr>, value) \
	TYPE(Var, 2, Token, name, std::shared_ptr<Expr>, initializer) \
	TYPE(While, 2, std::shared_ptr<Expr>, condition, std::shared_ptr<Stmt>, body)


// utility function to serve as a bootleg garbage collector
template<class T, class... Args>
std::shared_ptr<T> newShared(Args&&... args)
{
	std::shared_ptr<T> ptr = std::make_shared<T>(args...);
	ptr->setShared(ptr);
	return ptr;
}


class Expr
{
public:

	virtual ~Expr() = default;

	// forward declaring of nested classes
#define TYPE(name, ...) class name;
	EXPR_TYPES;
#undef TYPE

	// visitor pattern
	class Visitor
	{
	public:
		virtual ~Visitor() = default;
	#define TYPE(name, ...) virtual object_t visit ##name ##Expr(name& expr) = 0;
		EXPR_TYPES;
	#undef TYPE
	};

	// visitor pattern
	virtual object_t accept(Visitor* visitor) = 0;

	// memory management
	std::shared_ptr<Expr> getShared() const
	{
		if (!weakptr.lock())
		{
			// nts: better error message
			throw std::exception("didn't call setShared();");
		}
		return weakptr.lock();
	}
	void setShared(const std::shared_ptr<Expr>& ptr)
	{
		assert(ptr.get() == this);
		weakptr = ptr;
	}

private:
	std::weak_ptr<Expr> weakptr;
};

class Stmt
{
public:

	virtual ~Stmt() {}

	// forward declaring of nested classes
#define TYPE(name, ...) class name;
	STMT_TYPES;
#undef TYPE

	// visitor pattern
	class Visitor
	{
	public:
		virtual ~Visitor() = default;
	#define TYPE(name, ...) virtual void visit ##name ##Stmt(name& stmt) = 0;
		STMT_TYPES;
	#undef TYPE
	};

	// visitor pattern
	virtual void accept(Visitor* visitor) = 0;

	// memory management
	std::shared_ptr<Stmt> getShared() const
	{
		if (!weakptr.lock())
		{
			// nts: better error message
			throw std::exception("didn't call setShared();");
		}
		return weakptr.lock();
	}
	void setShared(const std::shared_ptr<Stmt>& ptr)
	{
		assert(ptr.get() == this);
		weakptr = ptr;
	}

private:
	std::weak_ptr<Stmt> weakptr;
};

#define PARAMETER_LIST1(t0, n0) t0 n0
#define PARAMETER_LIST2(t0, n0, t1, n1) t0 n0, t1 n1
#define PARAMETER_LIST3(t0, n0, t1, n1, t2, n2) t0 n0, t1 n1, t2 n2

#define INITIALIZER_LIST1(t0, n0) n0(std::move(n0))
#define INITIALIZER_LIST2(t0, n0, t1, n1) n0(std::move(n0)), n1(std::move(n1))
#define INITIALIZER_LIST3(t0, n0, t1, n1, t2, n2) n0(std::move(n0)), n1(std::move(n1)), n2(std::move(n2))

#define FIELDS1(t0, n0) t0 n0;
#define FIELDS2(t0, n0, t1, n1) t0 n0; t1 n1;
#define FIELDS3(t0, n0, t1, n1, t2, n2) t0 n0; t1 n1; t2 n2;


#define TYPE(name, nfields, ...) \
class Expr::name: public Expr \
{ \
	public: \
	name(PARAMETER_LIST ## nfields ## (__VA_ARGS__)): INITIALIZER_LIST ## nfields ## (__VA_ARGS__) {} /*construction*/\
	name(const name&) = delete; name& operator=(const name&) = delete; /*copying*/ \
	name(name&&) = default; name& operator=(name&&) = default; /*moving*/ \
	~name() override = default; /*destruction*/ \
	object_t accept(Visitor* visitor) override { return visitor->visit ## name ## Expr(*this); } \
	FIELDS ## nfields ## (__VA_ARGS__) \
	std::shared_ptr<name> getShared() const { return std::dynamic_pointer_cast<name>(Expr::getShared()); } \
};


//EXPR_TYPES expands to:
class Expr::Assign: public Expr { public: Assign(Token name, std::shared_ptr<Expr> value): name(std::move(name)), value(std::move(value)) {} Assign(const Assign&) = delete; Assign& operator=(const Assign&) = delete; Assign(Assign&&) = default; Assign& operator=(Assign&&) = default; ~Assign() override = default; object_t accept(Visitor* visitor) override { return visitor->visitAssignExpr(*this); } Token name; std::shared_ptr<Expr> value; std::shared_ptr<Assign> getShared() const { return std::dynamic_pointer_cast<Assign>(Expr::getShared()); } }; class Expr::Binary: public Expr { public: Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right): left(std::move(left)), op(std::move(op)), right(std::move(right)) {} Binary(const Binary&) = delete; Binary& operator=(const Binary&) = delete; Binary(Binary&&) = default; Binary& operator=(Binary&&) = default; ~Binary() override = default; object_t accept(Visitor* visitor) override { return visitor->visitBinaryExpr(*this); } std::shared_ptr<Expr> left; Token op; std::shared_ptr<Expr> right; std::shared_ptr<Binary> getShared() const { return std::dynamic_pointer_cast<Binary>(Expr::getShared()); } }; class Expr::Call: public Expr { public: Call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments): callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {} Call(const Call&) = delete; Call& operator=(const Call&) = delete; Call(Call&&) = default; Call& operator=(Call&&) = default; ~Call() override = default; object_t accept(Visitor* visitor) override { return visitor->visitCallExpr(*this); } std::shared_ptr<Expr> callee; Token paren; std::vector<std::shared_ptr<Expr>> arguments; std::shared_ptr<Call> getShared() const { return std::dynamic_pointer_cast<Call>(Expr::getShared()); } }; class Expr::Get: public Expr { public: Get(std::shared_ptr<Expr> object, Token name): object(std::move(object)), name(std::move(name)) {} Get(const Get&) = delete; Get& operator=(const Get&) = delete; Get(Get&&) = default; Get& operator=(Get&&) = default; ~Get() override = default; object_t accept(Visitor* visitor) override { return visitor->visitGetExpr(*this); } std::shared_ptr<Expr> object; Token name; std::shared_ptr<Get> getShared() const { return std::dynamic_pointer_cast<Get>(Expr::getShared()); } }; class Expr::Grouping: public Expr { public: Grouping(std::shared_ptr<Expr> expression): expression(std::move(expression)) {} Grouping(const Grouping&) = delete; Grouping& operator=(const Grouping&) = delete; Grouping(Grouping&&) = default; Grouping& operator=(Grouping&&) = default; ~Grouping() override = default; object_t accept(Visitor* visitor) override { return visitor->visitGroupingExpr(*this); } std::shared_ptr<Expr> expression; std::shared_ptr<Grouping> getShared() const { return std::dynamic_pointer_cast<Grouping>(Expr::getShared()); } }; class Expr::Literal: public Expr { public: Literal(object_t value): value(std::move(value)) {} Literal(const Literal&) = delete; Literal& operator=(const Literal&) = delete; Literal(Literal&&) = default; Literal& operator=(Literal&&) = default; ~Literal() override = default; object_t accept(Visitor* visitor) override { return visitor->visitLiteralExpr(*this); } object_t value; std::shared_ptr<Literal> getShared() const { return std::dynamic_pointer_cast<Literal>(Expr::getShared()); } }; class Expr::Logical: public Expr { public: Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right): left(std::move(left)), op(std::move(op)), right(std::move(right)) {} Logical(const Logical&) = delete; Logical& operator=(const Logical&) = delete; Logical(Logical&&) = default; Logical& operator=(Logical&&) = default; ~Logical() override = default; object_t accept(Visitor* visitor) override { return visitor->visitLogicalExpr(*this); } std::shared_ptr<Expr> left; Token op; std::shared_ptr<Expr> right; std::shared_ptr<Logical> getShared() const { return std::dynamic_pointer_cast<Logical>(Expr::getShared()); } }; class Expr::Set: public Expr { public: Set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value): object(std::move(object)), name(std::move(name)), value(std::move(value)) {} Set(const Set&) = delete; Set& operator=(const Set&) = delete; Set(Set&&) = default; Set& operator=(Set&&) = default; ~Set() override = default; object_t accept(Visitor* visitor) override { return visitor->visitSetExpr(*this); } std::shared_ptr<Expr> object; Token name; std::shared_ptr<Expr> value; std::shared_ptr<Set> getShared() const { return std::dynamic_pointer_cast<Set>(Expr::getShared()); } }; class Expr::Super: public Expr { public: Super(Token keyword, Token method): keyword(std::move(keyword)), method(std::move(method)) {} Super(const Super&) = delete; Super& operator=(const Super&) = delete; Super(Super&&) = default; Super& operator=(Super&&) = default; ~Super() override = default; object_t accept(Visitor* visitor) override { return visitor->visitSuperExpr(*this); } Token keyword; Token method; std::shared_ptr<Super> getShared() const { return std::dynamic_pointer_cast<Super>(Expr::getShared()); } }; class Expr::This: public Expr { public: This(Token keyword): keyword(std::move(keyword)) {} This(const This&) = delete; This& operator=(const This&) = delete; This(This&&) = default; This& operator=(This&&) = default; ~This() override = default; object_t accept(Visitor* visitor) override { return visitor->visitThisExpr(*this); } Token keyword; std::shared_ptr<This> getShared() const { return std::dynamic_pointer_cast<This>(Expr::getShared()); } }; class Expr::Unary: public Expr { public: Unary(Token op, std::shared_ptr<Expr> right): op(std::move(op)), right(std::move(right)) {} Unary(const Unary&) = delete; Unary& operator=(const Unary&) = delete; Unary(Unary&&) = default; Unary& operator=(Unary&&) = default; ~Unary() override = default; object_t accept(Visitor* visitor) override { return visitor->visitUnaryExpr(*this); } Token op; std::shared_ptr<Expr> right; std::shared_ptr<Unary> getShared() const { return std::dynamic_pointer_cast<Unary>(Expr::getShared()); } }; class Expr::Variable: public Expr { public: Variable(Token name): name(std::move(name)) {} Variable(const Variable&) = delete; Variable& operator=(const Variable&) = delete; Variable(Variable&&) = default; Variable& operator=(Variable&&) = default; ~Variable() override = default; object_t accept(Visitor* visitor) override { return visitor->visitVariableExpr(*this); } Token name; std::shared_ptr<Variable> getShared() const { return std::dynamic_pointer_cast<Variable>(Expr::getShared()); } };

#undef TYPE


#define TYPE(name, nfields, ...) \
class Stmt::name final : public Stmt \
{ \
	public: \
	name(PARAMETER_LIST ## nfields ## (__VA_ARGS__)): INITIALIZER_LIST ## nfields ## (__VA_ARGS__) {} /*construction*/ \
	name(const name&) = delete; name& operator=(const name&) = delete; /*copying*/ \
	name(name&&) = default; name& operator=(name&&) = default; /*moving*/ \
	~name() override = default; /*destruction*/ \
	void accept(Visitor* visitor) override { visitor->visit ## name ## Stmt(*this); } \
	FIELDS ## nfields ## (__VA_ARGS__) \
	std::shared_ptr<name> getShared() const { return std::dynamic_pointer_cast<name>(Stmt::getShared()); } \
};

//STMT_TYPES expands to:
class Stmt::Block final : public Stmt { public: Block(std::vector<std::shared_ptr<Stmt>> statements): statements(std::move(statements)) {} Block(const Block&) = delete; Block& operator=(const Block&) = delete; Block(Block&&) = default; Block& operator=(Block&&) = default; ~Block() override = default; void accept(Visitor* visitor) override { visitor->visitBlockStmt(*this); } std::vector<std::shared_ptr<Stmt>> statements; std::shared_ptr<Block> getShared() const { return std::dynamic_pointer_cast<Block>(Stmt::getShared()); } }; class Stmt::Class final : public Stmt { public: Class(Token name, std::shared_ptr<Expr::Variable> superclass, std::vector<std::shared_ptr<Stmt::Function>> methods): name(std::move(name)), superclass(std::move(superclass)), methods(std::move(methods)) {} Class(const Class&) = delete; Class& operator=(const Class&) = delete; Class(Class&&) = default; Class& operator=(Class&&) = default; ~Class() override = default; void accept(Visitor* visitor) override { visitor->visitClassStmt(*this); } Token name; std::shared_ptr<Expr::Variable> superclass; std::vector<std::shared_ptr<Stmt::Function>> methods; std::shared_ptr<Class> getShared() const { return std::dynamic_pointer_cast<Class>(Stmt::getShared()); } }; class Stmt::Expression final : public Stmt { public: Expression(std::shared_ptr<Expr> expression): expression(std::move(expression)) {} Expression(const Expression&) = delete; Expression& operator=(const Expression&) = delete; Expression(Expression&&) = default; Expression& operator=(Expression&&) = default; ~Expression() override = default; void accept(Visitor* visitor) override { visitor->visitExpressionStmt(*this); } std::shared_ptr<Expr> expression; std::shared_ptr<Expression> getShared() const { return std::dynamic_pointer_cast<Expression>(Stmt::getShared()); } }; class Stmt::Function final : public Stmt { public: Function(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body): name(std::move(name)), params(std::move(params)), body(std::move(body)) {} Function(const Function&) = delete; Function& operator=(const Function&) = delete; Function(Function&&) = default; Function& operator=(Function&&) = default; ~Function() override = default; void accept(Visitor* visitor) override { visitor->visitFunctionStmt(*this); } Token name; std::vector<Token> params; std::vector<std::shared_ptr<Stmt>> body; std::shared_ptr<Function> getShared() const { return std::dynamic_pointer_cast<Function>(Stmt::getShared()); } }; class Stmt::If final : public Stmt { public: If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch): condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {} If(const If&) = delete; If& operator=(const If&) = delete; If(If&&) = default; If& operator=(If&&) = default; ~If() override = default; void accept(Visitor* visitor) override { visitor->visitIfStmt(*this); } std::shared_ptr<Expr> condition; std::shared_ptr<Stmt> thenBranch; std::shared_ptr<Stmt> elseBranch; std::shared_ptr<If> getShared() const { return std::dynamic_pointer_cast<If>(Stmt::getShared()); } }; class Stmt::Print final : public Stmt { public: Print(std::shared_ptr<Expr> expression): expression(std::move(expression)) {} Print(const Print&) = delete; Print& operator=(const Print&) = delete; Print(Print&&) = default; Print& operator=(Print&&) = default; ~Print() override = default; void accept(Visitor* visitor) override { visitor->visitPrintStmt(*this); } std::shared_ptr<Expr> expression; std::shared_ptr<Print> getShared() const { return std::dynamic_pointer_cast<Print>(Stmt::getShared()); } }; class Stmt::Return final : public Stmt { public: Return(Token keyword, std::shared_ptr<Expr> value): keyword(std::move(keyword)), value(std::move(value)) {} Return(const Return&) = delete; Return& operator=(const Return&) = delete; Return(Return&&) = default; Return& operator=(Return&&) = default; ~Return() override = default; void accept(Visitor* visitor) override { visitor->visitReturnStmt(*this); } Token keyword; std::shared_ptr<Expr> value; std::shared_ptr<Return> getShared() const { return std::dynamic_pointer_cast<Return>(Stmt::getShared()); } }; class Stmt::Var final : public Stmt { public: Var(Token name, std::shared_ptr<Expr> initializer): name(std::move(name)), initializer(std::move(initializer)) {} Var(const Var&) = delete; Var& operator=(const Var&) = delete; Var(Var&&) = default; Var& operator=(Var&&) = default; ~Var() override = default; void accept(Visitor* visitor) override { visitor->visitVarStmt(*this); } Token name; std::shared_ptr<Expr> initializer; std::shared_ptr<Var> getShared() const { return std::dynamic_pointer_cast<Var>(Stmt::getShared()); } }; class Stmt::While final : public Stmt { public: While(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body): condition(std::move(condition)), body(std::move(body)) {} While(const While&) = delete; While& operator=(const While&) = delete; While(While&&) = default; While& operator=(While&&) = default; ~While() override = default; void accept(Visitor* visitor) override { visitor->visitWhileStmt(*this); } std::shared_ptr<Expr> condition; std::shared_ptr<Stmt> body; std::shared_ptr<While> getShared() const { return std::dynamic_pointer_cast<While>(Stmt::getShared()); } };

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
