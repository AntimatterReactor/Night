#pragma once

#include <vector>

#include "Token.h"
#include "Variable.h"

#include "MathParser.h"
#include "BoolParser.h"

void ExtractVariableValue(const std::vector<Variable> variables, std::vector<Token>& tokens);
void CheckExpression(const std::vector<Token>& tokens, TokenTypes type, int start, int end,
	std::string error);
bool CheckEquation(const std::vector<Token>& tokens, int index, TokenTypes type,
	int start, int end);

void Parser(std::vector<Token>& tokens)
{
	static std::vector<Variable> variables;

	// variable initialization
	if (tokens.size() >= 5 && tokens[1].type == TokenTypes::VARIABLE &&
		tokens[2].type == TokenTypes::ASSIGNMENT)
	{
		ExtractVariableValue(variables, tokens);

		if (tokens[0].type == TokenTypes::BOOL_TYPE)
		{
			CheckExpression(tokens, TokenTypes::BOOL_VALUE, 10, 12, "boolean");

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			BoolParser(tokens);
		}
		else if (tokens[0].type == TokenTypes::CHAR_TYPE)
		{
			// character types don't support arithmetic
			if (tokens.size() > 5)
			{
				std::cout << "Error - invalid character expression";
				exit(0);
			}
			else
			{
				if (tokens[3].type == TokenTypes::CHAR_VALUE)
				{
					variables.push_back(Variable{ "char", tokens[1].token, tokens[3].token });
				}
				else
				{
					std::cout << "Error - invalid character expression";
					exit(0);
				}
			}
		}
		else if (tokens[0].type == TokenTypes::INT_TYPE)
		{
			CheckExpression(tokens, TokenTypes::INT_VALUE, 5, 9, "integer");
			/*
			int openBrackets = 0;
			for (std::size_t a = 3; a < tokens.size() - 1; ++a)
			{
				if (!CheckEquation(tokens, a, TokenTypes::INT_VALUE, 5, 9))
				{
					std::cout << "Error - invalid integer expression";
					exit(0);
				}

				if (tokens[a].type == TokenTypes::OPEN_BRACKET)
					openBrackets += 1;
				else if (tokens[a].type == TokenTypes::CLOSE_BRACKET)
					openBrackets -= 1;
			}

			if (openBrackets > 0)
			{
				std::cout << "Error - missing closing bracket";
				exit(0);
			}
			else if (openBrackets < 0)
			{
				std::cout << "Error - missing opening bracket";
				exit(0);
			}
			*/

			// send off to math parser
			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			std::cout << "Gotcha!\n"; // testing purposes!
			MathParser(temp);
		}
		else
		{
			std::cout << "Error - invalid variable initialization";
			exit(0);
		}
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[0].type == TokenTypes::VARIABLE &&
		tokens[1].type == TokenTypes::ASSIGNMENT)
	{
		// make sure variable is defined
		bool definedVariable = false;
		for (std::size_t a = 0; a < variables.size(); ++a)
		{
			if (tokens[0].token == variables[a].name)
			{
				definedVariable = true;
				break;
			}
		}

		if (!definedVariable)
		{
			std::cout << "Error, variable not defined";
			exit(0);
		}

		ExtractVariableValue(variables, tokens);
	}
	else
	{
		std::cout << "Error - invalid language grammar\n";
		exit(0);
	}
}

// turn variables into their types
void ExtractVariableValue(const std::vector<Variable> variables, std::vector<Token>& tokens)
{
	for (std::size_t a = 2; a < tokens.size() - 1; ++a)
	{
		if (tokens[a].type != TokenTypes::VARIABLE)
			continue;

		bool definedVariable = false;
		for (std::size_t b = 0; b < variables.size(); ++b)
		{
			if (tokens[a].token == variables[a].name)
			{
				tokens[a].type = (variables[a].type == "bool" ?
					TokenTypes::BOOL_TYPE : (variables[a].type == "char" ?
						TokenTypes::CHAR_TYPE : TokenTypes::INT_TYPE));

				tokens[a].token = variables[a].value;

				break;
			}
		}

		if (!definedVariable)
		{
			std::cout << "Error - variable not defined";
			exit(0);
		}
	}
}

void CheckExpression(const std::vector<Token>& tokens, TokenTypes type, int start, int end, 
	std::string error)
{
	int openBrackets = 0;
	for (std::size_t a = 3; a < tokens.size() - 1; ++a)
	{
		if (!CheckEquation(tokens, a, type, start, end))
		{
			std::cout << "Error - invalid " << error << " expression";
			exit(0);
		}

		if (tokens[a].type == TokenTypes::OPEN_BRACKET)
			openBrackets += 1;
		else if (tokens[a].type == TokenTypes::CLOSE_BRACKET)
			openBrackets -= 1;
	}

	if (openBrackets > 0)
	{
		std::cout << "Error - missing closing bracket";
		exit(0);
	}
	else if (openBrackets < 0)
	{
		std::cout << "Error - missing opening bracket";
		exit(0);
	}
}

bool CheckEquation(const std::vector<Token>& tokens, int index, TokenTypes type, int start, int end)
{
	// checks all numbers
	if (tokens[index].type == type)
	{
		if ((tokens[index - 1].type == TokenTypes::ASSIGNMENT &&
				tokens[index + 1].type == TokenTypes::SEMICOLON) ||
			(tokens[index - 1].type == TokenTypes::ASSIGNMENT &&
				(int)tokens[index + 1].type >= start && (int)tokens[index + 1].type <= end) ||
			((int)tokens[index - 1].type >= start && (int)tokens[index - 1].type <= end &&
				(int)tokens[index + 1].type >= start && (int)tokens[index + 1].type <= end) ||
			(tokens[index - 1].type == TokenTypes::OPEN_BRACKET &&
				(int)tokens[index + 1].type >= start && (int)tokens[index + 1].type <= end) ||
			((int)tokens[index - 1].type >= start && (int)tokens[index - 1].type <= end &&
				tokens[index + 1].type == TokenTypes::CLOSE_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::OPEN_BRACKET &&
				tokens[index + 1].type == TokenTypes::CLOSE_BRACKET) ||
			((int)tokens[index - 1].type >= start && (int)tokens[index - 1].type <= end &&
				(tokens[index + 1].type == TokenTypes::SEMICOLON)))
		{
			
		}
		else
		{
			return false;
		}
	}
	// checks all operators
	else if ((int)tokens[index].type >= start && (int)tokens[index].type <= end)
	{
		if ((tokens[index - 1].type == type &&
				tokens[index + 1].type == type) ||
			(tokens[index - 1].type == type &&
				tokens[index + 1].type == TokenTypes::OPEN_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::CLOSE_BRACKET &&
				tokens[index + 1].type == type) ||
			(tokens[index - 1].type == TokenTypes::CLOSE_BRACKET &&
				tokens[index + 1].type == TokenTypes::OPEN_BRACKET))
		{

		}
		else
		{
			return false;
		}
	}
	// checks all open brackets
	else if (tokens[index].type == TokenTypes::OPEN_BRACKET)
	{
		if (((int)tokens[index - 1].type >= start && (int)tokens[index - 1].type <= end &&
				tokens[index + 1].type == type) ||
			((int)tokens[index - 1].type >= start && (int)tokens[index - 1].type <= end &&
				tokens[index + 1].type == TokenTypes::OPEN_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::OPEN_BRACKET &&
				tokens[index + 1].type == type) ||
			(tokens[index - 1].type == TokenTypes::OPEN_BRACKET &&
				tokens[index + 1].type == TokenTypes::OPEN_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::ASSIGNMENT &&
				tokens[index + 1].type == TokenTypes::OPEN_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::ASSIGNMENT &&
				tokens[index + 1].type >= type))
		{
		
		}
		else
		{
			return false;
		}
	}
	// checks all close brackets
	else if (tokens[index].type == TokenTypes::CLOSE_BRACKET)
	{
		if ((tokens[index - 1].type == type &&
				(int)tokens[index + 1].type >= start && (int)tokens[index + 1].type <= end) ||
			(tokens[index - 1].type == TokenTypes::CLOSE_BRACKET &&
				(int)tokens[index + 1].type >= start && (int)tokens[index + 1].type <= end) ||
			(tokens[index - 1].type == type &&
				tokens[index + 1].type == TokenTypes::CLOSE_BRACKET) ||
			(tokens[index - 1].type == TokenTypes::CLOSE_BRACKET &&
				tokens[index + 1].type == TokenTypes::CLOSE_BRACKET) ||
			(tokens[index - 1].type == type &&
				tokens[index + 1].type == TokenTypes::SEMICOLON) ||
			(tokens[index - 1].type == TokenTypes::CLOSE_BRACKET &&
				tokens[index + 1].type == TokenTypes::SEMICOLON))
		{

		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}