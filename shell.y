/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE ABBEND PIPE INPUT BACKGROUND ERROR
/*
GREAT = ">" 
NEWLINE = "\n"
ABBEND = ">>" 
PIPE = "|"
INPUT = "<"
BACKGROUND = "&"
*/

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command command
	| simple_command
    ;

simple_command:	
	command_and_args iomodifier_opt PIPE {
		printf("   Yacc: Execute command\n");
		
	}
	|command_and_args iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
		;
	}
	| NEWLINE
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	ABBEND WORD iomodifier_opt{
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1; 

	}
	|GREAT WORD iomodifier_opt{
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	|BACKGROUND iomodifier_opt{
		printf("   Yacc: insert background \"1\"\n");
		Command::_currentCommand._background = 1;
	}
	|INPUT WORD iomodifier_opt{
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	|ERROR WORD iomodifier_opt{
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	} 
	|
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
