#pragma once

#include "../error.hpp"

#include <memory>
#include <functional>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct CheckVariable;
struct CheckFunction;
struct CheckClass;

using CheckVariableContainer = std::unordered_map<std::string, CheckVariable>;
using CheckFunctionContainer = std::unordered_map<std::string, CheckFunction>;
using CheckClassContainer    = std::unordered_map<std::string, CheckClass>;

enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGN,

	COLON, COMMA,

	BOOL, INT, FLOAT, STRING,

	VARIABLE,

	SET,

	IF, ELSE,

	WHILE, FOR,

	DEF,
	RETURN,

	IMPORT,

	EOL
};

struct Token
{
	Location loc;

	TokenType type;
	std::string data;


	bool operator==(const TokenType& _type) const;
};

enum class ValueType
{
	BOOL, INT, FLOAT,
	STRING, ARRAY,

	VARIABLE, CALL,

	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET
};

struct Value
{
	ValueType type;
	std::string data;

	// arrays are classified as one value, so their elements are stored
	// in 'extras'; same for function calls, as 'extras' stores their
	// arguments
	std::vector<std::vector<Value> > extras;
};

struct VariableType
{
	enum Type {
		BOOL,
		INT,
		FLOAT,
		STRING,
		ARRAY,
		CLASS
	} type;

private:
	// std::variant?
	// and getter methods like get_class_name() and get_elem_types()
	std::variant<
		// note about array types:
		/*
		// if this type is an array, it will also contain all the types of the
		// elements as well
		//
		// this is used in determining the types of for loop iterators
		//
			arr = [ true, 7 ]   # 'arr' has type:  'array'
			for (x : arr) {}	# 'x'   has types: 'bool', 'int'
		//
		// it is also used in determining the value of a subscript operator
		//
			arr = [ true, 7 ]   # 'arr' has type:  'array'
			x = arr[0]			# 'x'   has types: 'bool', 'int'
		//
		*/
		std::unordered_set<VariableType, HashVariableType>,

		// if this is an object, then this variable stores the name of the class
		std::string
	> special;

public:
	VariableType(
		const Type& _type = {}
	);

	VariableType(
		const std::unordered_set<VariableType, HashVariableType>& _elem_types
	);

	VariableType(
		const std::string& _class_name
	);

	~VariableType();

	std::string to_str() const;
	std::unordered_set<VariableType, HashVariableType>& get_elem_types() const;

	bool operator==(const VariableType& _type) const;
	bool operator!=(const VariableType& _type) const;
};

struct HashVariableType
{
	std::size_t operator()(const VariableType& _type) const;
};

using VariableTypeContainer = std::unordered_set<VariableType, HashVariableType>;

struct Statement;

struct Expression
{
	const Location loc;

	ValueType type;
	std::string data;

	std::vector<std::shared_ptr<Expression> > extras;

	std::shared_ptr<Expression> left, right;
};

// struct ExpressionNode;

struct Scope
{
	const std::shared_ptr<Scope> upper_scope;

	std::vector<Statement> statements;

	CheckVariableContainer variables;

	Scope(const std::shared_ptr<Scope>& _upper_scope);
};

struct Variable
{
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Assignment
{
	enum {
		ASSIGN,
		PLUS,
		MINUS,
		TIMES,
		DIVIDE,
		MOD
	} assign_type;

	std::string variable_name;
	std::shared_ptr<Expression> assign_expr;
};

struct Conditional
{
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Scope> body;
};

struct IfStatement
{
	std::vector<Conditional> chains;
};

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;

	std::shared_ptr<Scope> body;
};

struct FunctionCall
{
	std::string name;
	std::vector<std::shared_ptr<Expression> > arguments;
};

struct Return
{
	std::shared_ptr<Expression> expression;
};

struct WhileLoop
{
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Scope> body;
};

struct ForLoop
{
	std::string iterator_name;
	std::shared_ptr<Expression> range;

	std::shared_ptr<Scope> body;
};

struct Element
{
	std::string name;
	std::vector<std::shared_ptr<Expression> > index;
	std::shared_ptr<Expression> assign;
};

struct MethodCall
{
	std::string name;
	std::shared_ptr<Expression> assign_expr;
};

enum class StatementType
{
	VARIABLE,
	ASSIGNMENT,
	IF_STATEMENT,
	FUNCTION_DEF,
	FUNCTION_CALL,
	RETURN,
	WHILE_LOOP,
	FOR_LOOP,
	ELEMENT,
	METHOD_CALL
};

struct Statement
{
	const Location loc;

	const StatementType type;

	std::variant<
		Variable,
		Assignment,
		IfStatement,
		FunctionDef,
		FunctionCall,
		Return,
		WhileLoop,
		ForLoop,
		Element,
		MethodCall
	> stmt;
};



struct CheckVariable
{
	CheckVariable(
		const VariableTypeContainer& _types = {}
	);

	// a note about parameters:
	/*
	// to perform type checking, parameters' types must be evaluated when the
	// function is defined
	//
	// they are stored in the same container as normal variables, so the only
	// difference is that they don't have a type
	//
	// they can be differentiated from normal variables using the method:
	// 'needs_types()'
	//
	// their types are giving to them through the expressions they encounter,
	// for example 'param || true' would mean 'param' is a boolean
	//
	// if a parameter still doesn't have a type at the end of the function,
	// then it is given all the types
	//
	// once a parameter has types, it then behaves like a normal variable
	*/
	VariableTypeContainer types;
};

struct CheckFunction
{
	std::vector<VariableTypeContainer> parameters;

	// a note about return types:
	/*
	// function return types have to be deduced when they are defined
	//
	// this is done by examining the return statement(s) of the function
	//
	// if no return types can be deduced, then the return type is treated as
	// whatever type is required for it to be
	*/
	VariableTypeContainer return_types;

	bool is_void;
};

// constructor for CheckFunction
std::pair<const std::string, CheckFunction> make_check_function(
	const std::string& name,
	const std::vector<VariableTypeContainer>& params = {},
	const VariableTypeContainer& rtn_types = {}
);

struct CheckClass
{
	CheckVariableContainer variables;
	CheckFunctionContainer methods;
};

// constructor for CheckClass
std::pair<const std::string, CheckClass> make_check_class(
	const std::string& name,
	const CheckVariableContainer& vars = {},
	const CheckFunctionContainer& methods = {}
);