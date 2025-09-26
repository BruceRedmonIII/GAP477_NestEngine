#include "StringParser.h"
#include "../Configuration/LogMacro.hint"
#include "../Managers/LogManager.h"
#include <cctype>

double nest::StringParser::ParseString(const std::string& str)
{
    std::istringstream stream(str);
    double value = ParseExpression(stream);

    // Make sure the stream is clear
    char leftover;
    if (stream >> leftover)
    {
        _LOG(LogType::kError, "Leftover numbers after expression evaluation!");
    }

    return value;
}

double nest::StringParser::ParseExpression(std::istringstream& stream)
{
    double value = ParseTerm(stream);

    while (true)
    {
        char op = static_cast<char>(stream.peek());
        if (op == '+' || op == '-')
        {
            stream.get();
            double term = ParseTerm(stream);
            if (op == '+')
                value += term;
            else
                value -= term;
        }
        else
        {
            break; // Done with Addition/Subtraction
        }
    }

    return value;
}

double nest::StringParser::ParseTerm(std::istringstream& stream)
{
    double value = ParseFactor(stream);

    while (true)
    {
        char op = static_cast<char>(stream.peek());
        if (op == '*' || op == '/')
        {
            stream.get();
            double factor = ParseFactor(stream);
            if (op == '*')
                value *= factor;
            else
            {
                if (factor == 0)
                {
                    _LOG(LogType::kError, "Division by zero!");
                }
                else
                    value /= factor;
            }
        }
        else
        {
            break; // No more Multiplication/Division
        }
    }

    return value;
}

double nest::StringParser::ParseFactor(std::istringstream& stream)
{
    char ch = static_cast<char>(stream.peek());

    if (std::isdigit(ch) || ch == '.') // if true then its a number
    {
        double value;
        stream >> value;
        return value;
    }
    else if (ch == '(') // start parenthesis evaluation
    {
        stream.get(); // clears '('
        double value = ParseExpression(stream);
        if (stream.peek() == ')')
        {
            stream.get(); // clears ')'
        }
        else
        {
            _LOG(LogType::kError, "Mismatched parentheses.");
            return value;
        }
    }
    else
    {
        _LOG(LogType::kError, "Unexpected character: " + ch);
    }

    return 0; // this should never be reached
}