%option batch
%option c++
%option noyywrap
%option 8bit
%option stack
%option yylineno
%option yyclass="DB::SqlParse"
%option prefix="sqlBase"

%{
#include "sqlParse.h"
#pragma GCC diagnostic ignored "-Wsign-compare"
%}

space			[ \t\n\r\f]
non_newline [^\r\n]
mcomment_start "/*"
mcomment_stop "*/"
comment			("--"{non_newline}*)
other .
term ;
any ({other}|{space})
quote ('|\")
quote_apos ''
quote_esc \\.
dolq_start [A-Za-z\200-\377_]
dolq_cont [A-Za-z\200-\377_0-9]
dollarquote \$({dolq_start}{dolq_cont}*)?\$
scriptdir "$SCRIPTDIR"

%x COMMENT
%x STATEMENT
%x QUOTE
%x DOLLARQUOTE

%%

{mcomment_start} {
	comment += YYText();
	yy_push_state(COMMENT);
}

<COMMENT>{mcomment_stop} {
	comment += YYText();
	Comment(comment);
	comment.clear();
	yy_pop_state();
}

<COMMENT>{any} {
	comment += YYText();
}

<COMMENT><<EOF>> {
	throw SqlParseException("Unterminated comment", yylineno);
}

{comment} {
	Comment(YYText());
}

<INITIAL>{term} {
	// Random terminator
}

{other} {
	statement += YYText();
	yy_push_state(STATEMENT);
}

<STATEMENT>{quote} {
	statement += YYText();
	yy_push_state(QUOTE);
}

<STATEMENT>{dollarquote} {
	statement += YYText();
	yy_push_state(DOLLARQUOTE);
}

<QUOTE>{quote} {
	statement += YYText();
	yy_pop_state();
}

<QUOTE>{scriptdir} {
	statement += scriptDir.string();
}

<QUOTE>{quote_apos} {
	statement += YYText();
}

<QUOTE>{quote_esc} {
	statement += YYText();
}

<DOLLARQUOTE>{any} {
	statement += YYText();
}

<DOLLARQUOTE>{dollarquote} {
	statement += YYText();
	yy_pop_state();
}

<DOLLARQUOTE><<EOF>> {
	throw SqlParseException("Unterminated dollar quoted string", yylineno);
}

<QUOTE>{any} {
	statement += YYText();
}

<QUOTE><<EOF>> {
	throw SqlParseException("Unterminated quoted string", yylineno);
}

<STATEMENT>{term} {
	Statement(statement);
	statement.clear();
	yy_pop_state();
}

<STATEMENT>{any} {
	statement += YYText();
}

<*>[ \t\r\n\f] {
}

