#pragma once
#include <string>

namespace nest
{
	// this was primarily used for a calculator I made in a UI class
	class StringParser
	{
		enum Operators
		{
			kMultipy,
			kDivide,
			kAdd,
			kSubtract,
			kLeftParenthesis,
			kRightParenthesis
		};
	public:
		double ParseString(const std::string& str);

	private:
		double ParseExpression(std::istringstream& stream);
		double ParseTerm(std::istringstream& stream);
		double ParseFactor(std::istringstream& stream);

	};
}

