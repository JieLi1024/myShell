#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "command.hh"
#include "shell.hh"

#include <string>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>




Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
		_append = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

extern char **environ;



void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }

    // Print contents of Command data structure
    //print();

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec

    //keep the default stdin, stdout, stderr for restore later
    int tmpin = dup(0);
    int tmpout = dup(1);
    int tmperr = dup(2);

    //create variables for input, output, error files
    int fdin;
    int fdout;
    int fderr;
	
	/* deal with input file before the loop 
	   since it's related to the first command */

    //if _inFile is specified in command
    if (_inFile) {
		//open _inFile as read only
		fdin = open(_inFile->c_str(), O_RDONLY);
		/* open(): Upon successful completion,
		the function shall open the file and return a non-negative
		integer representing the lowest numbered unused file descriptor. 
		Otherwise, -1 shall be returned and errno set to indicate the error. 
		No files shall be created or modified if the function returns -1. */
		//error checking
		if (fdin < 0) {
			printf("/bin/sh: 1: cannot open %s: No such file\n", _inFile->c_str());
			exit(1);
		} 
	}else{
		//else, use the default stdin as input
		fdin = dup(tmpin);
	}
		
	/* deal with errfile redirection before loop 
	because all errors are output to the same location */
	//if _errFile is specified in the command
	if (_errFile) {
		//Case1: open and append to existing file
		if (_append) {
			fderr = open(_errFile->c_str(), O_CREAT|O_WRONLY|O_APPEND, 0664);
		}else{
		//Case2: open and overwrite the content of the file (exist/not exist)
			fderr = open(_errFile->c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0664);
		}
		//error checking
		if (fderr < 0){
			exit(1);
		}
	}else{
	//else, use the default stderr
		fderr = dup(tmperr);
	}
	//redirect err
	dup2(fderr, 2);
	close(fderr);
	
	int ret;
	for(int i = 0; i < _simpleCommands.size(); i++) {
		//redirect input
		dup2(fdin, 0);
		close(fdin);
		
		/***************** setup fdout START *********************/
		/*** if this is the last simpleCommand ***/
		if (i == _simpleCommands.size() - 1) {
			//if _outFile is specified in the command
			if (_outFile) {
				//Case1: open and append to existing file
				if (_append){
					fdout = open(_outFile->c_str(), O_CREAT|O_WRONLY|O_APPEND, 0664);
					if (fdout < 0){
						exit(1);
					}
				}else{
				//Case2: open and overwrite the content of the file (exist/not exist)
					fdout = open(_outFile->c_str(), O_CREAT|O_WRONLY|O_TRUNC,0664);
					//error checking
					if (fdout < 0){
						exit(1);
					}
				}
			}else{
			//else, use the default stdout
				fdout = dup(tmpout);
			}
		}else{	
		/**** if this is not the last simpleCommand, then create pipe ***/
		int fdpipe[2];
		pipe(fdpipe);
		//what's written in fdpipe[1] could be read from fdpipe[0];
		fdout = fdpipe[1];
		fdin = fdpipe[0];
		}

		/***************** setup fdout END *********************/
		//after setting the fdout,
		//do output redirection
		dup2(fdout, 1);
		close(fdout);
		

		/*************************** Before fork(), parent process  *******************************/
		/****************** BUILT-IN command: exit, setenv, unsetenv, cd, source START ********************/
		if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "exit") == 0){
			close(tmpin);
  			close(tmpout);
 			close(tmperr);
			close(fderr);
			close(fdin);
			close(fdout);	
			exit(0);
		}

		//setenv
		if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv") == 0){
			//int setenv(const char *envname, const char *envval, int overwrite), overwrite > 0:= overwrite=TRUE;
			if(setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1) != 0){
				//Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned,
				perror("setenv");
			}
			clear();
			if(isatty(0)){
				Shell::prompt();
			}
			return;
		}

		if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv") == 0){
			//int setenv(const char *envname, const char *envval, int overwrite), overwrite > 0:= overwrite=TRUE;
			if(unsetenv(_simpleCommands[i]->_arguments[1]->c_str()) != 0){
				//Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned,
				perror("unsetenv");
			}
			clear();
			if(isatty(0)){
				Shell::prompt();
			}
			return;
		}

		if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd") == 0){
			//int chdir(const char *path);

			//If no directory is specified, default to the home directory
			char *dir_PATH;
			if(_simpleCommands[i]->_arguments.size() == 1){
				dir_PATH = getenv("HOME");
			}else{
				dir_PATH = strdup(_simpleCommands[i]->_arguments[1]->c_str());	
			}
			if(chdir(dir_PATH) != 0){
				//printf("cd: can't cd to %s\n", dir_PATH);
				fprintf(stderr, "cd: can't cd to %s\n", dir_PATH);
			}
			clear();
			if(isatty(0)){
				Shell::prompt();
			}
			//close unused file descripter before return
			close(tmpin);
  			close(tmpout);
 			close(tmperr);
			close(fderr);
			close(fdin);
			close(fdout);
			return;
		}

		if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source") == 0){
			FILE * file = fopen(_simpleCommands[i]->_arguments.at(1)->c_str(), "r");
			if (file  == NULL){
				fprintf(stderr, "Cannot open source file!");
			}
			_simpleCommands.erase(_simpleCommands.begin() + i);
			sourcefunc(file);
		}


		/****************** BUILT-IN command: setenv, unsetenv, cd, source START ********************/
		/*************************** Before fork(), parent process  *******************************/


		
		/*************************** child process  *******************************/
		//create child process 
		ret = fork();
		if(ret == 0){
			/*********** BUILT-IN command: printenv START************/
			//Execute printenv in the child process and exit
			if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv") == 0){  //strcmp return 0 if equal
				char** tmp = environ;
				while(*tmp != NULL){
					printf("%s\n", *tmp);
					tmp++;
				}
				exit(0);
			}
			/*********** BUILT-IN command: printenv END************/

			/*********   execute this simpleCommand START   *********/
			/* Preprocessing: convert _arguments to NULL terminated char* array */
			char* nt_arguments[_simpleCommands[i]->_arguments.size()+1]; //nt_arguments is char**
			size_t j;
			for (j = 0; j < _simpleCommands[i]->_arguments.size(); j++){
				nt_arguments[j] = strdup(_simpleCommands[i]->_arguments[j]->c_str()); //strdup will return char* of a string
			}
			nt_arguments[j] = NULL;

			execvp(_simpleCommands[i]->_arguments[0]->c_str(), nt_arguments);
			perror("execvp");
            _exit(1);

			/********   execute this simpleCommand END   **********/
			
		}else if (ret < 0){
			perror("fork");
			return;
		}else{
			//printf("%d",ret);
		}
	}

	//restore
	dup2(tmpin,0);
	dup2(tmpout,1);
	dup2(tmperr, 2);
	close(tmpin);
  	close(tmpout);
 	close(tmperr);

	int status = 0;
  	if (!_background){
   		waitpid(ret, &status ,0);	

	}else{
		//${!}: return pid of the last process run in the background 
		setenv("!", std::to_string(ret).c_str(), 1);
	}	

	//${_}: The last argument in the fully expanded previous command
	//Note: this excludes redirects
	if (_simpleCommands.size() > 0){
		int size = _simpleCommands[_simpleCommands.size() - 1]->_arguments.size();
		Command::lastCommand = strdup(_simpleCommands[_simpleCommands.size()-1]->_arguments[size-1]->c_str());
	}
	setenv("_", Command::lastCommand, 1);

    // Clear to prepare for next command
  	clear();

    // Print new prompt
	if ( isatty(0) ) {
		Shell::prompt();
	}
}

SimpleCommand * Command::_currentSimpleCommand;
