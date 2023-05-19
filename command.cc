/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "command.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include  < limits.h>
#include  < bits / stdc++.h>
#include  < ctime>
#include  < string>
//#include <errno.h>

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
	// Print contents of Command data structure
	print();
	// Exit commands
	if (!strcasecmp(_currentSimpleCommand->_arguments[0], "exit"))
	{
		kill(getpid(), SIGQUIT);
	}
	// Cd command
	if (!strcasecmp(_currentSimpleCommand->_arguments[0], "cd"))
	{
		int cd;
		char *dir;
		if (_currentSimpleCommand->_arguments[1] == NULL)
		{
			struct passwd *pw = getpwuid(getuid());
			const char *homeDirectory = pw->pw_dir;
			strcpy(dir, homeDirectory);
			cd = chdir(homeDirectory);
		}
		else
		{
			strcpy(dir, _currentSimpleCommand->_arguments[1]);
			cd = chdir(_currentSimpleCommand->_arguments[1]);
		}

		if (cd != 0)
		{
			perror("chdir");
		}
		// Clear to prepare for next command
		clear();
		// Print new prompt
		prompt();
		if (cd == 0)
		{
			printf("%s> ", dir);
		}
		return;
	}
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	int infd;
	int outfd;
	int errfd;
	int appfd;
	int fdpipe[2];

	if (pipe(fdpipe) == -1){
		perror("Error Whlie Piping");
		exit(2);
	}
	
	for(int i = 0; i < _numberOfSimpleCommands; i++){
		int n;
	    char *command = _simpleCommands[i]->_arguments[0];
	    
	 	if(i == 0){
			if(_inputFile != 0){
				infd = open(_inputFile, O_RDONLY);
				if(infd < 0){
					perror("Error while creating input file\n");
					exit(2);
				}
		// Redirect output to the created utfile instead off printing to stdout 
 			dup2(infd, 0);
			close(infd);
			}
			if(_inputFile != 0){
				dup2(infd, 0);
			}else{
				dup2(defaultin, 0);
			}
		}
		if(i != 0){
			dup2(fdpipe[0], 0);
			close(fdpipe[0]);
		 	close(fdpipe[1]);
		 	if(pipe(fdpipe) == - 1){
				perror("Error While Piping , Error code : 0P");
			 	exit(2);
			}
		}
	 	if(i == _numberOfSimpleCommands - 1){
			if(_appendFile != 0){
				appfd = open(_appendFile, O_WRONLY | O_APPEND);
				if(appfd < 0){
					perror("Error while creating append file\n");
				 	exit(2);
				}
			// Redirect output to the created utfile instead off printing to stdout 
			dup2(appfd, 1);
			close(appfd);                         
			}
		    if(_outFile != 0){
				outfd = creat(_outFile, 0666);
				if  (outfd < 0){
					perror("Error While Directing Output\n");
					exit(2);
				}
			// Redirect output to the created utfile instead off printing to stdout 
            dup2(outfd, 1);
			close(outfd);                         
		 	}
			if(_errFile != 0){
				errfd = open(_appendFile, O_WRONLY | O_APPEND);
			 	if(errfd < 0){
					perror("Error While Directing Output\n");
				    exit(2);
				}
			// Redirect output to the created utfile instead off printing to stdout 
			dup2(errfd, 2);
			close(errfd);
			}
		    if(_appendFile != 0){
				dup2(appfd, 1);
			}else if(_outFile != 0){
				dup2(outfd, 1);
			}else{
				dup2(defaultout, 1);
			}
		}
		if(i != _numberOfSimpleCommands - 1){
			dup2(fdpipe[1], 1);             
	 	}
		// Print contents of Command data structure 
		int pid = fork();
	    if(pid == - 1){
			perror("ls: fork\n");
		    exit(2);
		}
		if(pid == 0){
			if(_outFile != 0){
				close(outfd);
			}
		    if(_inputFile != 0){
				close(infd);
			}
		    if(_appendFile != 0){
				close(appfd);
			}
		    if(_errFile != 0){
               close(errfd);
			}
		    childTerminated();
		    close(defaultin);
		    close(defaultout);
		    close(defaulterr);
		 	// You can use execvp() instead if the arguments are stored in an array 
 			execvp(command, _simpleCommands[i]->_arguments);
			// exec() is not suppose to return, something went wrong 
            perror("Error While execution");
		    exit(2);
		}
		if(_background == 0){
			waitpid(pid, 0, 0);
		}     
	}
	// Restore input, output, and error 
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	// Close file descriptors that are not needed 
	if  (_inputFile != 0){
		close(infd);         
	}
	if(_appendFile != 0){
		close(appfd);         
	}
	if(_outFile != 0){
		close(outfd);
	}
	if(_errFile != 0){
		close(errfd);         
	}
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	// Wait for last process in the pipe line 
	// Clear to prepare for next command 
	clear();
	// Print new prompt 
	prompt();
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("\nmyshell> ");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

// Signals Handelrs
void sigintHandler(int sig_num)
{
	signal(SIGINT, sigintHandler);
	printf("\n Cannot be terminated using Ctrl+C \n");
	Command::_currentCommand.prompt();
}
void sigkillHandler(int sig_num)
{
	printf("\n\tGood bye!!\n\n");
	exit(0);
}

void sigchildHandler(int sig_num)
{
	time_t t;
	time(&t);
	FILE *f = fopen("log.txt", "a");
	fprintf(f, "Child Terminated at %s\n", ctime(&t));
	fclose(f);
}
int main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT, sigintHandler);
	signal(SIGQUIT, sigkillHandler);
	signal(SIGCHLD, sigchildHandler);
	yyparse();
	return 0;
}
