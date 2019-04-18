  
%code requires 
{
#include <string>
#include <string.h> 
#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "simpleCommand.hh"
#include "command.hh"
#include "shell.hh"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <dirent.h> // wildcard expansion directory stuff
#include <fcntl.h> // open() arguments
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strcmp
#include <unistd.h>
#include <algorithm>   // sort


#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE 
%token PIPE GREATGREAT LESS TWOGREAT GREATAMPERSAND GREATGREATAMPERSAND AMPERSAND

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"


void yyerror(const char * s);

int yylex();

void expandWildcard(char * prefix, char * suffix);


void addArg(std::string arg);
int compare (const void * a, const void * b);
void sortArgs();


std::vector<std::string> argArray;
%}

%%

goal:
	command_list
	;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

argument_word:
	WORD {
		// expand wildcards
		char * word = strdup(($1)->c_str());
		expandWildcard(NULL, word);

		// sort the arguments
		sortArgs();
	
		for (int i = 0; i < argArray.size(); i++) {
			//add all the sorted arguments
			std::string* entry = new std::string(argArray[i]);
			Command::_currentSimpleCommand->insertArgument(entry);
		}
		
		argArray.clear();
	
	}
	;
  ;

arg_list:
	arg_list argument_word
	| /*empty*/
	;

cmd_and_args:
	command_word arg_list
	{
		Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

pipe_list:
	pipe_list PIPE cmd_and_args
	| cmd_and_args
	;

//*****
io_modifier:
	GREATGREAT WORD
		{
			//printf("   Yacc: append output to the existing _outFile  \"%s\"\n",$2->c_str());
     	Shell::_currentCommand._outFile = $2;
     	Shell::_currentCommand._append = true;
		}
	| GREAT WORD
		{
			//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
			if (!Shell::_currentCommand._outFile){
				Shell::_currentCommand._outFile = $2;
			}else{
				printf("Ambiguous output redirect.\n");
				exit(1);
			}
		}
	| GREATGREATAMPERSAND WORD
		{
			//printf("   Yacc: append both output and errfile to existing files \"%s\"\n", $2->c_str());
     	Shell::_currentCommand._outFile = $2;
     	Shell::_currentCommand._errFile = $2;
     	Shell::_currentCommand._append = true;
		}
	| GREATAMPERSAND WORD
		{
			//printf("   Yacc: insert both output and errfile \"%s\"\n", $2->c_str());
    	Shell::_currentCommand._outFile = $2;
    	Shell::_currentCommand._errFile = $2;
		}
	| LESS WORD
		{
			//printf("   Yacc: insert input \"%s\"\n", $2->c_str());
     	Shell::_currentCommand._inFile = $2;
		}
	| TWOGREAT WORD
		{
			//printf("   Yacc: insert errfile \"%s\"\n", $2->c_str());
    	Shell::_currentCommand._errFile = $2;
		}
	;
		


io_modifier_list:
	io_modifier_list io_modifier
	| /*empty*/
	;

//******
background_opt:
	AMPERSAND{
		Shell::_currentCommand._background = true;
	}
	| /*empty*/
	;

command:
	pipe_list io_modifier_list background_opt NEWLINE 
		{	
			//printf("   Yacc: Execute command\n");
    	Shell::_currentCommand.execute();
		}
	| NEWLINE { if ( isatty(0) ) Shell::prompt();} /*accept empty cmd line*/ 
	| error NEWLINE { yyerrok; } 
	;


command_list:
	command
	| command_list command
	;

%%


void expandWildcard(char * prefix, char * suffix) {
	if (suffix[0] == 0) {
		// suffix is empty, put prefix in argument
		addArg(std::string(prefix));
		return;
	}

	//1. start_with_slash is true when suffix[0] == '/'
	//2. suffix[0] == '/' only can happen at initial call to expandWildcard
	//since later on, every suffix is set not to start with slash
	bool start_with_slash;

	/******* set suffix, component START ********/
	// obtain the next component in the suffix && advance suffix

	// if the first character is a '/', try to get the position of the 2nd
	char * s = NULL;
	if (suffix[0] == '/') { //if this is '/' then find next '/' after this char
		start_with_slash = true;
		s = strchr( (char*)(suffix+1), '/');
	} else { 
		s = strchr(suffix, '/');
	}
	

	char component[MAXFILENAME] = ""; // must initialize this

	//if '/' is found, now s points to next '/'
	if ( s != NULL ) { // copy up to the next '/'
		strncpy(component, suffix, s - suffix);
		suffix = s + 1;  //char after next '/'
	} else { 
		//else: no future '/' 
		//last part of path, copy the whole thing
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}
	/******* set suffix, component START ********/


	/********** TRY to expand the component START **********/
	char newPrefix[MAXFILENAME];


	//Case1: component has no wildcards 
	//directly append the component to the prefix
 	if ( strchr(component, '*') == NULL && strchr(component, '?') == NULL ) {
		// if prefix is empty
		if ( prefix == NULL || prefix[0] == 0 ) {
			sprintf(newPrefix, "%s", component);
		} else {
			sprintf(newPrefix, "%s/%s", prefix, component);
		}
		expandWildcard(newPrefix, suffix);
		return;
	}

	//Case2: component has wildcards

	/******* expand the component *******/
	
	/** First, convert it to regex **/
	char * reg = (char*)malloc(2*strlen(component)+10);	
	char * a = component;
	char * r = reg;

	//copy over all characters, converting to regex representation
	*(r++) = '^';
	while (*a) {
			if (*a == '/'){a++;}
			if (*a == '*') { *(r++) = '.'; *(r++) = '*';}
			else if (*a == '?') {*(r++) = '.';}
			else if (*a == '.') {*(r++) = '\\'; *(r++) = '.';}
			else {*(r++) = *a;}
			a++;
	}
	*(r++) = '$'; 
	*r = '\0';

	/** Second, compile regex **/
	regex_t re;
	if ( regcomp(&re, reg, REG_EXTENDED|REG_NOSUB) != 0 ) {
		perror("regcomp");
		exit(1);
	}

	// if prefix is empty, then open default directory
	char * dir_name;
	bool add_slash_as_head;

	
	// There are 2 default directories:
	//1. if prefix == NULL && start_with_slash, then open directory '/'
	//2. else: list current directory '.'
	if ( prefix == NULL ) {
		if(start_with_slash){
			dir_name = "/";
			add_slash_as_head = true;
		}else{
			const char * dot_char = ".";
			dir_name = strdup(dot_char);
		}
	} else {
		if(prefix[0] != '/'){
			//sprintf(dir_name, "/%s", prefix);
			
			char temp[MAXFILENAME];
			sprintf(temp, "/%s", prefix);
			dir_name = temp;

		}else{
			dir_name = prefix;
		}
	}

	// open the directory
	DIR * dir = opendir(dir_name);
	if (dir == NULL) {
		return;
	}

	struct dirent * ent;
	bool match = false;

	/**** recursively try to match regex in directories ****/
	while ( (ent = readdir(dir)) != NULL ) {
		//check if name matches
		if (regexec(&re, ent->d_name, (size_t)0, NULL, 0) == 0) {
			match = true;
			if (prefix == NULL || prefix[0] == 0) {
				if(add_slash_as_head){
					sprintf(newPrefix, "/%s", ent->d_name);
				}else{
					sprintf(newPrefix, "%s", ent->d_name);
				}
			} else {
				sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
			}

			if (ent->d_name[0] == '.') { // only add items beginning with . if regex also begins with .
				if (component[0] == '.') {
					expandWildcard(newPrefix, suffix);
				}
			} else {
				expandWildcard(newPrefix, suffix);
			}
		}
	}
	closedir(dir);
	regfree(&re); //important
	
	/*although there is wildcard in component, but failed to expand due to no match
		Thus, deal with it like component has no wildcard */
	if(!match){
		if ( prefix == NULL || prefix[0] == 0 ) {
			sprintf(newPrefix, "%s", component);
		} else {
			sprintf(newPrefix, "%s/%s", prefix, component);
		}
		expandWildcard(newPrefix, suffix);
	}
	/********** TRY to expand the component END **********/
}

// add argument to resizable array
void addArg(std::string arg){
	argArray.push_back(arg);
}

int compare (const void * a, const void * b) {
	return strcmp( *(const char**)a, *(const char**)b);
}

// sort the arguments
void sortArgs(){
	std::sort(argArray.begin(), argArray.end());
}

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
