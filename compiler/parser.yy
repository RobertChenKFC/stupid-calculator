%skeleton "lalr1.cc"
%require "3.8.1"
%header

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>
    class Driver;
}

%param { Driver &driver };

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
    #include "driver.hh" 

    using namespace std::string_literals;
}

%define api.token.prefix {TOK_}
%token 
    CONST "const" 
    VAR "var" 
    FUNCTION "function"
    FOR "for"
    IF "if"
    ELIF "elif"
    ELSE "else"
    WHILE "while"
    RETURN "return"
    TRUE "true"
    FALSE "false"
    LPAREN "("
    RPAREN ")"
    LBRACK "["
    RBRACK "]"
    LBRACE "{"
    RBRACE "}"
    COMMA ","
    SEMICOL ";"
    ASSIGN "="
    EQ "=="
    NEQ "!="
    LT "<"
    LEQ "<="
    GT ">"
    GEQ ">="
    ADD "+"
    SUB "-"
    AND "&"
    LOGIC_AND "&&"
    OR "|"
    LOGIC_OR "||"
    NOT "~"
    LOGIC_NOT "!"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%nterm <std::string> expr exprList call nonEmptyExprList assignStmt

%left "&&" "||"
%right "!"
%nonassoc "<" "<=" ">" ">=" "==" "!="
%left "+" "-"
%left "&" "|"
%right "~"

%%

%start code;

code:
      { driver.addLine("class Main {"); driver.indent(); }
      decls defs
      { driver.unindent(); driver.addLine("}"); };

decls:
      %empty {}
    | decl decls {};

decl:
      "const" constList ";" {}
    | "var" varList ";" {};

constList:
      constList "," const {}
    | const {};

const:
      "identifier" "=" "number" { driver.addConstant($1, $3); };

varList:
      varList "," var {}
    | var {};

var:
      "identifier" { driver.addVar($1); };

defs:
      defs def {}
    | %empty {}

def:
      "function" "identifier" "("
      { driver.addLine("function int " + $2 + "(", false); }
      paramList ")" "{"
      { driver.addLine(") {", true, false); driver.enterFunction($2); }
      body "}"
      { driver.addLine("return 0;"); driver.exitFunction(); driver.addLine("}"); };

paramList:
      nonEmptyParamList {}
    | %empty {};

nonEmptyParamList:
      nonEmptyParamList "," "identifier"
      { driver.addLine(", int " + $3, false, false); }
    | "identifier" { driver.addLine("int " + $1, false, false); };

body:
      decls { driver.initConstants(); } stmts {};

stmts:
      stmts stmt {}
    | %empty {};

stmt:
      forStmt {}
    | whileStmt {}
    | ifStmt {}
    | returnStmt ";" {}
    | assignStmt ";" { driver.addLine($1 + ";"); };
    | call ";" { driver.addLine("do " + $1 + ";"); };

forStmt:
      "for" "(" assignStmt ";" expr ";" assignStmt ")" "{" 
      { driver.addLine($3 + ";");
        driver.addLine("while (" + $5 + ") {");
        driver.indent(); }
      stmts "}"
      { driver.addLine($7 + ";"); driver.unindent(); driver.addLine("}"); };

whileStmt:
      "while" "(" expr ")" "{"
      { driver.addLine("while (" + $3 + ") {"); driver.indent(); }
      stmts "}"
      { driver.unindent(); driver.addLine("}"); };

ifStmt:
      "if" "(" expr ")" "{"
      { driver.addLine("if (" + $3 + ") {"); driver.indent(); }
      stmts "}"
      { driver.unindent(); driver.addLine("}"); }
      elifStmts elseStmt {};

elifStmts:
      "elif" "(" expr ")" "{"
      { driver.addLine("else {");
        driver.indent();
        driver.addLine("if (" + $3 + ")" "{");
        driver.indent(); }
      stmts "}"
      { driver.unindent(); driver.addLine("}"); }
      elifStmts 
      { driver.unindent(); driver.addLine("}"); }
    | %empty {};

elseStmt:
      "else" "{"
      { driver.addLine("else {"); driver.indent(); }
      stmts "}"
      { driver.unindent(); driver.addLine("}"); }
    | %empty {};

returnStmt:
      "return" { driver.addLine("return 0;"); }
    | "return" expr { driver.addLine("return " + $2 + ";"); };

assignStmt:
      "identifier" "=" expr { $$ = "let " + $1 + " = " + $3; }
    | "identifier" "[" expr "]" "=" expr
      { $$ = "let " + $1 + "[" + $3 + "] = " + $6; }
    | %empty { $$ = ""s; };

expr:
      expr "&&" expr { $$ = "(" + $1 + " & " + $3 + ")"; }
    | expr "||" expr { $$ = "(" + $1 + " | " + $3 + ")"; }
    | expr "<" expr { $$ = "(" + $1 + " < " + $3 + ")"; }
    | expr "<=" expr { $$ = "(~(" + $1 + " > " + $3 + "))"; }
    | expr ">" expr { $$ = "(" + $1 + " > " + $3 + ")"; }
    | expr ">=" expr { $$ = "(~(" + $1 + " < " + $3 + "))"; }
    | expr "==" expr { $$ = "(" + $1 + " = " + $3 + ")"; }
    | expr "!=" expr { $$ = "(~(" + $1 + " = " + $3 + "))"; }
    | expr "+" expr { $$ = "(" + $1 + " + " + $3 + ")"; }
    | expr "-" expr { $$ = "(" + $1 + " - " + $3 + ")"; }
    | expr "&" expr { $$ = "(" + $1 + " & " + $3 + ")"; }
    | expr "|" expr { $$ = "(" + $1 + " | " + $3 + ")"; }
    | "~" expr { $$ = "(~" + $2 + ")"; }
    | "-" expr { $$ = "(0 - " + $2 + ")"; }
    | "!" expr { $$ = "(~" + $2 + ")"; }
    | "(" expr ")" { $$ = "(" + $2 + ")"; }
    | "identifier" { $$ = $1; }
    | "number" { $$ = std::to_string($1); }
    | "true" { $$ = "true"s; }
    | "false" { $$ = "false"s; }
    | call { $$ = $1; }
    | "identifier" "[" expr "]" { $$ = $1 + "[" + $3  + "]"; };

call:
      "identifier" "(" exprList ")" { $$ = "Main." + $1 + "(" + $3 + ")"; };

exprList:
      nonEmptyExprList { $$ = $1; }
    | %empty { $$ = ""s; };

nonEmptyExprList:
      nonEmptyExprList "," expr { $$ = $1 + ", " + $3; }
    | expr { $$ = $1; };
%%

void yy::parser::error(
        const location_type &location, const std::string &message) {
    std::cerr << location << ": " << message << "\n";
}
