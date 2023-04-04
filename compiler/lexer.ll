%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "driver.hh"
#include "parser.hh"
%}

%option noyywrap nounput noinput batch

id [a-zA-Z][_a-zA-Z0-9]*
int [0-9]+
blank [ \t\r]

%{
#define YY_USER_ACTION location.columns(yyleng);
%}

%%

%{
    yy::location &location = driver.location;
    location.step();
%}

{blank}+ location.step();
\n+ location.lines(yyleng); location.step();
"//".*\n location.lines(1); location.step();

"const" return yy::parser::make_CONST(location);
"var" return yy::parser::make_VAR(location);
"function" return yy::parser::make_FUNCTION(location);
"for" return yy::parser::make_FOR(location);
"if" return yy::parser::make_IF(location);
"elif" return yy::parser::make_ELIF(location);
"else" return yy::parser::make_ELSE(location);
"while" return yy::parser::make_WHILE(location);
"return" return yy::parser::make_RETURN(location);
"true" return yy::parser::make_TRUE(location);
"false" return yy::parser::make_FALSE(location);
"(" return yy::parser::make_LPAREN(location);
")" return yy::parser::make_RPAREN(location);
"[" return yy::parser::make_LBRACK(location);
"]" return yy::parser::make_RBRACK(location);
"{" return yy::parser::make_LBRACE(location);
"}" return yy::parser::make_RBRACE(location);
";" return yy::parser::make_SEMICOL(location);
"," return yy::parser::make_COMMA(location);
"=" return yy::parser::make_ASSIGN(location);
"==" return yy::parser::make_EQ(location);
"!=" return yy::parser::make_NEQ(location);
"<" return yy::parser::make_LT(location);
"<=" return yy::parser::make_LEQ(location);
">" return yy::parser::make_GT(location);
">=" return yy::parser::make_GEQ(location);
"+" return yy::parser::make_ADD(location);
"-" return yy::parser::make_SUB(location);
"&" return yy::parser::make_AND(location);
"&&" return yy::parser::make_LOGIC_AND(location);
"|" return yy::parser::make_OR(location);
"||" return yy::parser::make_LOGIC_OR(location);
"~" return yy::parser::make_NOT(location);
"!" return yy::parser::make_LOGIC_NOT(location);

{int} return yy::parser::make_NUMBER(std::atoi(yytext), location);
{id} return yy::parser::make_IDENTIFIER(yytext, location);
. { throw yy::parser::syntax_error(location, std::string("invalid character: ") + yytext); }
<<EOF>> return yy::parser::make_YYEOF(location);

%%

void Driver::scanBegin() {
    yyin = fopen(filename_.c_str(), "r");
}

void Driver::scanEnd() {
    fclose(yyin);
}
