#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <pwd.h>

#include "simpleCommand.hh"
//#define MAXFILENAME 1024

SimpleCommand::SimpleCommand() {
	_arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
	// iterate over all the arguments and delete them
	for (auto & arg : _arguments) {
		delete arg;
	}
}

void SimpleCommand::insertArgument( std::string * argument ) {
	// expand arguments then insert to _arguments
	envExpansion(argument);
	tildeExpansion(argument);
	_arguments.push_back(argument);
}


// Environment Variable Expansion
void SimpleCommand::envExpansion(std::string * argument){
	char * input = strdup(argument->c_str());

	char * dollar = strchr(input, '$');
	char * bracesf = strchr(input, '{');

	if (dollar && bracesf){
		char * res = (char *)malloc(sizeof(argument) * 500);
		char * pt_res = res;
		//directly copy chars before $
		while (*input != '$'){
			*pt_res = *input;
			pt_res++;
			input ++;
		}
		*pt_res = '\0';  //must do this for future strcat
		//stop outermost while loop when there is no more
		while (dollar){
		//catch every dollar
			if (dollar[1] == '{' && dollar[2] != '}'){
				char * dummy = dollar + 2;
				char * apd = (char *)malloc(sizeof(argument)*50);
                char * copy = apd;

				while(*dummy != '}'){
					*copy = *dummy;
					copy++;
					dummy++;
				}
				*copy = '\0';
				//expand to PID of the shell process
				if(strcmp(apd, "$") == 0){
					char *pid = res;
					sprintf(pid, "%ld", (long)getpid());
				//The path of your shell executable
				} else if (strcmp(apd, "SHELL") == 0){
					char * link = (char *)malloc(50);
					if (readlink("/proc/self/exe", link, 50) != -1){
						char * path = realpath(link, NULL);
						strcat(res, path);
					}
					free(link);	
				//otherwise, find in environment variable
				} else{ 
					strcat(res, getenv(apd));
				}
				free(apd);
				while (*(input-1) != '}') input++;

				char * x = (char *)malloc(sizeof(argument) *20);
				char * pt_x = x;
				
				while (*input != '$' && *input){
					*pt_x = *input;
					pt_x++;
					input++;
				}
				*pt_x = '\0';
				strcat(res, x);
			}
			dollar++;
			dollar = strchr(dollar, '$');
		}
		std::string s(strdup(res));
		free(res);
		*argument = s;
	}
}


void SimpleCommand::tildeExpansion(std::string * argument){
	char * input = strdup(argument->c_str());
	if (input[0] == '~'){
		if (strlen(input) == 1){
			*argument = strdup(getenv("HOME"));
		} else {
			input ++;
			char * userName = (char *)malloc(sizeof(argument) * 500);
			char * name = userName;
			while (*input && *input != '/'){
				*name = *input;
				name ++;
				input ++;
			}
			*name = '\0';
			if (*input){
				*argument = strdup(strcat(getpwnam(userName)->pw_dir, input));
			} else {
				*argument = strdup(getpwnam(userName)->pw_dir);
			}
			free(userName);
		}
	
	}

}


// Print out the simple command
void SimpleCommand::print() {
	for (auto & arg : _arguments) {
		std::cout << "\"" << *arg << "\" \t";
	}
	// effectively the same as printf("\n\n");
	std::cout << std::endl;
}


