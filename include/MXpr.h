/*
	MXpr.h

	Author: Ian Agar
  Date: Nov 2006
  
  class to evaluate mathematical expressions of variable lengths and precision
  also supports variables and error reporting, and it follows order of operations
  parenthesis are the only grouping symbol, as opposed to brackets for example
  example expressions:
		2+(3.457*(pi+2))
		fg = 129+pi
		19340.43/fg
		2+341.45+93457.3*10234.5/1000-3^5+4.2435
*/

//TODO: add ability to work with functions, such as sqrt() for example

/*
	solve() is the main solving function; its only argument is a std::string
	containing the mathematical or boolean expression, or variable assignment.
	Variables are created or modified using the form VAR[=VAL], and are referenced
	using the name alone, such as "2*testVar+3". Only the most recent error message
	is stored, and can be retrieved using getLastError(). The error message should
	be cleared using clearError(). To check for an error, call checkErr(), which returns
	boolean true or false. solve() returns 0 when an error has happened.
	
	pi is the only default variable, and goes under the variable name "pi" and "PI".
*/

#ifndef MXPR_H
#define MXPR_H

#include <map>
#include <string>
#include <sstream>

#include <math.h>
#include <ctype.h>
#include <stdlib.h>

class MXpr
{
    public:
        MXpr();
        void MXpr::getArg(std::string&, std::string&, std::string&, char&, unsigned long);
        bool isLineDigits(std::string);
        bool fetchVal(std::string, double&);
        bool addVar(std::string, std::string);
        bool editVar(std::string, std::string);
        inline void clearError();
        inline void eraseVar(std::string);
        inline bool checkErr();
        inline std::map<std::string, std::string> dumpVars();
        inline std::string getLastError();
        double solve(std::string);
        double quickSolve(std::string, std::string, char);
    private:
        inline void setError(std::string);
    protected:
        std::string error;
        std::map<std::string, std::string> vars;
};

MXpr::MXpr()
{
	std::ostringstream temp;
    temp << M_PI;
    vars["pi"] = temp.str(), vars["PI"] = temp.str();
    error = "";
}

inline bool ismath(char what)
{
    if (what == '*' || what == '/' || what == '+' || what == '-' || what == '%' || what == '^')
        return true;
    return false;
}

inline std::string dtos(double what)
{
    std::ostringstream temp;
    temp << what;
    return temp.str();
}

inline bool MXpr::checkErr()
{
	return error.size() > 0;
}

inline void MXpr::clearError()
{
    error = "";
}

inline std::map<std::string, std::string> MXpr::dumpVars()
{
	return vars;
}

bool MXpr::isLineDigits(std::string what)
{
    register unsigned short signs = 0, puntos = 0;
    if (what[0] == '-' || what[0] == '+') ++signs;
    const register unsigned long sizer = what.size() - 1;
    for (register unsigned int a = 0;a <= sizer;a++)
    {
        if (what[a] == '.')
            ++puntos;
        if ((!isdigit(what[a]) || (puntos > 1 && what[a] == '.')) || (signs == 1 && (what[a] == '-' || what[a] == '+')))
            return false;
    }
    return true;
}

bool MXpr::addVar(std::string var, std::string val)
{
    if (vars.find(var) != vars.end())
    {
        editVar(var, val); //variable already found, assume user wants to modify
        return true;
    }
    if (var.find_first_of("*/+-%^") != std::string::npos)
    {
        error = "MXpr::addNewVar(): variable names may not contain mathematical operators";
        return false;
    }
    if (!isalpha(var[0]))
    {
        error = "MXpr::addNewVar(): variable names must start with a letter";
        return false;
    }
    if (var.find_first_of(" \t\n\r") != std::string::npos)
    {
        error = "MXpr::addNewVar(): variable names may not contain whitespace";
        return false;
    }
    vars[var] = val;
    return true;
}

bool MXpr::editVar(std::string var, std::string val)
{
    if (vars.find(var) == vars.end())
    {
        error = "MXpr::editVar(): variable \"" + var + "\" not found";
        return false;
    }
    vars[var] = val;
    return true;
}

bool MXpr::fetchVal(std::string var, double &val)
{
    if (vars.find(var) == vars.end())
    {
        error = "MXpr::fetchVal(): variable \"" + var + "\" not found";
        return false;
    }
    val = solve(vars[var]); //the only time values are solved
    return true;
}

inline void MXpr::eraseVar(std::string var)
{
    vars.erase(vars.find(var));
}

inline std::string MXpr::getLastError()
{
    return error;
}

inline void MXpr::setError(std::string what)
{
    error = what;
}

void MXpr::getArg(std::string &what, std::string &arg1,
                         std::string &arg2, char &oper, unsigned long where=0)
{
    if (where == what.size() - 1) //at the end
    {
        arg1 = "#END"; //notify the user that someone goofed
        return;
    }
    arg1 = "", arg2 = "";
    std::string::iterator iter = what.begin(), beg = iter, ender = what.end();
    iter += where, beg += where;
    if (*iter == '-' || *iter == '+') //negative/positive sign
    {
        arg1 += *iter++;
        if (*iter == '-' || *iter == '+') //that wasn't a neg/pos sign after all
        {
            error = "MXpr::getArg(): expression has too many operators at position " + dtos((double) where);
            return;
        }
    }
    while (!ismath(*iter) && iter != ender)
        arg1 += *iter++;
    if (iter == ender) goto fetchVars; //only one arg, just close things up
    oper = *iter++;
    if (iter == ender)
    {
        error = "MXpr::getArg(): expected argument after operator";
        return;
    }
    if (*iter == '-' || *iter == '+')
    {
        arg2 += *iter++;
        if (*iter == '-' || *iter == '+')
        {
            error = "MXpr::getArg(): expression has too many operators";
            return;
        }
    }
    while (!ismath(*iter) && iter != ender)
        arg2 += *iter++;
    what.erase(beg, iter);
    if (isalpha(arg1[0]) || isalpha(arg2[0])) goto fetchVars;
    return;
    fetchVars:
        double temp;
        if (isalpha(arg1[0]))
        {
            fetchVal(arg1, temp);
            arg1 = dtos(temp);
        }
        if (isalpha(arg2[0]))
        {
            fetchVal(arg2, temp);
            arg2 = dtos(temp);
        }
        return;
}

double MXpr::quickSolve(std::string arg1, std::string arg2, char oper)
{
    double expr;
    const double first = atof(arg1.c_str()), second = atof(arg2.c_str());
    if (oper == '/')
    {
        if (second == 0)
        {
            error = "MXpr: division by zero";
            return 0;
        }
        expr = first / second;
    }
    else if (oper == '*') expr = first * second;
    else if (oper == '%') expr = static_cast<long> (first) % static_cast<long> (second);
    else if (oper == '+') expr = first + second;
    else if (oper == '-') expr = first - second;
    else if (oper == '^') expr = pow(first, second);
    else
    {
        error = "MXpr:quickSolve(): function was provided with an unknown operator";
        return 0;
    }
    return expr;
}

double MXpr::solve(std::string what)
{
    for (std::string::iterator x = what.begin();x <= what.end();x++) //eliminate whitespace
      if (isspace(*x))
        what.erase(x);
    if (isLineDigits(what)) //it is already solved as it is just numbers
        return atof(what.c_str());
    if (what.find('=') != std::string::npos) //a variable is being assigned
    {
		std::string var = "";
		std::string::iterator f = what.begin(), beginner = f;
		while (*f != '=')
		{
			var += *f;
			++f;
		}
		++f;
		what.erase(beginner, f);
		f = what.begin(); //start over after all that erasing
		if (*f == '=') //boolean comparison
		{
			what.erase(f);
			double val = 0;
			fetchVal(var, val);
			return (solve(what) == val);
		}
		addVar(var, what);
		return 0;
	}
    { //main solving scope; it is a valid expression, so far
        unsigned int pos = what.find('('), amt = 1;
        while (pos != std::string::npos) //there are parenthesis; solve these first
        {
            const unsigned int old = pos;
            ++pos;
            std::string excerpt = "";
            what += "\n"; //marker for the end
            const std::string::iterator beg = what.begin();
            while (amt > 0 && what[pos] != '\n')
            { //in this scope, "amt" is used to count how many parenthesis are opened
                if (what[pos] == '(')
                    ++amt;
                else if (what[pos] == ')')
                {
                    if (isdigit(what[pos+1]))
                    {
                        error = "MXpr: missing operator after closing parenthesis";
                        return 0;
                    }
                    --amt;
                    if (amt == 0)
                        goto end;
                }
                pos++;
                continue;
                end: //(amt == 0)
                    for (unsigned int c = old + 1;c <= pos - 1;c++)
                        excerpt += what[c];
                    what.erase(beg + old, beg + pos);
                    break;
            }
            if (amt > 0)
            {
                error = "MXpr: missing closing parentheses";
                return 0;
            }
            what.insert(old, dtos(solve(excerpt)));
            if (error != "") return 0;
            pos = what.find('(');
        }
        std::string arg1, arg2;
        char oper;
        while ((amt = what.find_first_of("^")) != std::string::npos &&
            what.find_first_of("*/%+-") != std::string::npos) //if we have exponent and others
        {
            --amt;
            if (ismath(what[amt]))
            {
                error = "MXpr: no argument before ^ symbol";
                return 0;
            }
            while (!ismath(what[amt]) && amt != 0) --amt; //work backwards
            if (ismath(what[amt])) ++amt;
            getArg(what, arg1, arg2, oper, amt);
            if (arg2 == "" && arg1 != "") return atof(arg1.c_str());
            else if (arg2 == "") return 0;
            what.insert(amt, dtos((quickSolve(arg1, arg2, oper))));
        }
        while ((amt = what.find_first_of("*/%")) != std::string::npos &&
            what.find_first_of("+-") != std::string::npos) //if we have mult/div and +/-
        {
            --amt;
            if (ismath(what[amt]))
            {
                error = "MXpr: no argument before *, /, or % symbol";
                return 0;
            }
            while (!ismath(what[amt]) && amt != 0) --amt;
            if (ismath(what[amt])) ++amt;
            getArg(what, arg1, arg2, oper, amt);
            if (arg2 == "" && arg1 != "") return atof(arg1.c_str());
            else if (arg2 == "") return 0;
            what.insert(amt, dtos((quickSolve(arg1, arg2, oper))));
        }
        while (!isLineDigits(what) && what.size() > 0)
        {
            getArg(what, arg1, arg2, oper);
            double temp = quickSolve(arg1, arg2, oper);
            if (arg2 == "" && arg1 != "")
            {
                error = ""; //ignore bogus error (FIXME: ignorance is not the answer!)
                return atof(arg1.c_str());
            }
            else if (arg2 == "") return 0;
            if (error != "") return 0;
            what.insert(0, dtos(temp));
        }
        return atof(what.c_str());
    }
}

#endif /* MXPR_H */
