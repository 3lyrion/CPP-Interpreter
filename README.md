# An interpetator for my simplified variant of JavaScript language (Easy JS)

## Description

- Interpetator language: C++
- OS: Windows (others were not tested)
- Features of Easy JS:
	1. Alphabet
		- Keywords: bool, const, false, float, else, if, int, print, true, string, while
		- Identificators: `[A-Za-z_]+[A-Za-z0-9_]*`
		- Integer type: `-?([1-9][0-9]*|0)`
		- Real type: `-?[0-9]*\.[0-9]*`
		- String type: `[^"\n\r\t]*`
		- Meaningful  separators: '{', '}', '(', ')', '.', ',', ';', '−', '+', '*', '/', '%', '^', '!', '!=', '==', '&&', '||', '>=', '<=', '>', '<', '='
		- Minor separators: space, tab, line break
		- Comments: same as in JS
	2. Syntax
		program = { declaration | statement | block }
		declaration = ( typeSpec **id**  [ "=" expression ] ) | ( "const" typeSpec **id**  "=" expression ) ";"
		typeSpec = "bool" | "int" | "float" | "string"
		expression = logic1 { "||" logic1 }
		logic1 = logic2 { "&&" logic2 }
		logic2 = logic3 [ ( "==" | "!=" ) logic3 ]
		logic3 = term [ ( "<" | ">" | "<=" | "=>" ) term ]
		term = factor { ( "+" | "-" ) factor }
		factor = power { ( "*" | "/" | "%" ) power }
		power = operand { "^" operand }
		operand = [ "-" | "!" ] ( **id** | literal | "(" expression ")" )
		literal = "true" | "false" | **float** | **int** | **string**
		statement = selStmt | iterStmt | printStmt | exprStmt
		selStmt = "if" "(" expression ")" block [ "else" block ]
		block = "{" program "}"
		iterStmt = "while" "(" expression ")" block
		printStmt = "print" expression ";"
		exprStmt = **id** "=" expression ";"
	3. Semantics
		1) Constants
		They can be either boolean (“false”, “true") or integers. (“-1”, “203”, ...) and floating point numbers (“-1030.02”, “0.0”, ...), so are the strings (“abc", “V1a", ...).
		2) Expressions
		They are made up of brackets, variables, constants, and algebraic operations.
		3) Algebraic operations
		"+" – addition
		"−" – subtraction
		"*" – multiplication
		"/" – division (the result can be decimal)
		"%" – remainder of division
		"^" – exponentiation
		4) Operators
			1) Assignment operator
			The assignment operator is used to assign a value to a variable. It has the form subject = object. The operator can assign the result of evaluating an expression to a variable.
			Example:
			```
			float fS = f1 + f2;
			```
			2) Logical operators
			"!" – logical NOT
			"&&" – logical AND
			"||" – logical OR
			"!=" – not equal
			"==" – equal
			"<=" – less or equal
			">=" – greater or equal
			"<" – less
			">" – greater
			3) The output operator
			The output operator outputs the value of a variable from a new line.
			Example:
			```
			print num;
			print “output”;
			```
			4) The loop
			Organizes the repetition of the execution of a sequence of program statements.
			Example:
			```
			while (sum != 10)
			{
				sum = sum + 2;
			}
			```
		5) Priority of operations
		| :---:		| :---:			|
		| Priority	| Operations	|
		| 1			| (, )			|
		| 2			| ^				|
		| 3			| !, –a			|
		| 4			| *, /, %		|
		| 5			| a + b, a – b	|
		| 6			| <, >, <=, =>	|
		| 7			| ==, !=		|
		| 8			| &&			|
		| 9			| ||			|
		| 10		| print, =		|
	4. Usage example
	// Declarations
	bool finish = false;
	float sum = 5.0;
	const float FACTOR = 2.;
	string str = "Abcdef";
	int count;
	count = 0;
	int d = 5;

	if (d % 2 == 0)
	{
	}
	else
	{
		sum=0.0;
	}

	/*
	Loop
	*/
	while (finish == false)
	{
		sum = 5.0+(sum+10.0) * -FACTOR^1.5;
		count = count + 1;
		finish = !(count > 100 || sum <= 1.0 && sum >= 0.0);	
	}

	{
	print "Result";
	print sum;
	}

## License

This project is under MIT License.

Copyright (c) 2025 3lyrion

> Permission is hereby granted, free of charge, to any person obtaining a copy  
> of this software and associated documentation files (the "Software"), to deal  
> in the Software without restriction, including without limitation the rights  
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
> copies of the Software, and to permit persons to whom the Software is  
> furnished to do so, subject to the following conditions:  
> 
> 
> The above copyright notice and this permission notice shall be included in all  
> copies or substantial portions of the Software.  
> 
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
> SOFTWARE.
