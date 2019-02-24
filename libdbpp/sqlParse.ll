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
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wnull-conversion"
#endif
%}

space			[ \t\n\r\f]
linespace	[ \t]
non_newline [^\r\n]
mcomment_start ("/*"{space}*)
mcomment_stop ({space}*"*/")
lcomment_start	({space}*"--"{linespace}*)
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

%x MCOMMENT
%x LCOMMENT
%x STATEMENT
%x QUOTE
%x DOLLARQUOTE

%%

<STATEMENT>{mcomment_start} {
	yy_push_state(MCOMMENT);
}

{mcomment_start} {
	yy_push_state(MCOMMENT);
}

<STATEMENT>{lcomment_start} {
	yy_push_state(LCOMMENT);
}

{lcomment_start} {
	yy_push_state(LCOMMENT);
}

<MCOMMENT>{mcomment_stop} {
	Comment(comment);
	comment.clear();
	yy_pop_state();
}

<MCOMMENT>{any} {
	comment += YYText();
}

<MCOMMENT><<EOF>> {
	throw SqlParseException("Unterminated comment", yylineno);
}

<LCOMMENT>{non_newline}* {
	Comment(YYText());
  yy_pop_state();
}

<LCOMMENT>[\r\n]+ {
	Comment(comment);
  yy_pop_state();
}

<INITIAL>{term} {
	// Random terminator
}

{space} { }
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
	statement += scriptDir;
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

