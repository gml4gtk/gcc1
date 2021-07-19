%{
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

int regs[26];
int base;

int val;
int printflag;

/* forward declaration */
void yyerror (char *s);

%}

%start stmt

%token DIGIT LETTER EXIT

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS

%%

stmt	:	expr
			{ val = $1; printflag = 1; }
	|	 LETTER '=' expr
			{ regs[$1] = $3; }
	|	EXIT
			{ exit(0); }
	;

expr	:	'(' expr ')'
			{ $$ = $2; }
	|	expr '+' expr
			{ if ($1 == 69)
			    {
			      $$ = 2;
			      printf ("$1 became %d\n", $1);
			      $1 = 69;
			    }
			  $$ = $1 + $3; }
	|	expr '-' expr
			{ $$ = $1 - $3; }
	|	expr '*' expr
			{ $$ = $1 * $3; }
	|	expr '/' expr
			{ $$ = $1 / $3; }
	|	expr '%' expr
			{ $$ = $1 % $3; }
	|	expr '|' expr
			{ $$ = $1 | $3; }
	|	expr '&' expr
			{ $$ = $1 & $3; }
	|	'-' expr %prec UMINUS
			{ $$ = - $2; }
	|	LETTER
			{ $$ = regs[$1]; }
	|	number
/* 	|	'?'
			{ yydebug = !yydebug; }
*/	;

number	:	DIGIT
			{ $$ = $1;  base = ($1 == 0) ? 8 : 10; }
	|	number DIGIT
			{ $$ = base * $1 + $2; }
	;

%%

static int eol;

int yylex (void)
{
  int c;

  while ( (c=getchar()) == ' ') {}
  if (c == '\n')
    { eol = 1;
      return 0; }
  if (c == 'Q' || c == EOF)
    return(EXIT);
  if (islower(c))
    {
      yylval = c - 'a';
      return (LETTER);
    }
  if (isdigit(c))
    {
      yylval = c - '0';
      return (DIGIT);
    }
  return (c);
}

void yyerror (char *s)
{
  printf("%s\n", s);
}

int main (int argc, char *argv[])
{
  for (;;)
    {
      eol = 0;
      printflag = 0;

      if (yyparse()) printflag = 0;

      if (printflag) printf("%d\n", val);

      while (!eol)
	if (yylex() == EOF)
	   exit(0);
    }
 return (0);
}
