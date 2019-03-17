
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
%token NOTOKEN GREAT NEWLINE (original) 
//adding new tokens:
//PIPE GREATGREAT LESS 2GREAT GREATAMPERSAND GREATGREATAMPERSAND AMPERSAND
 */

%{


#include <string>
#include <unistd.h>
#include <fcntl.h> 

#include <cstring>
#include "y.tab.hh"

////////////  Start added code ///////////

extern "C" char * read_line();


int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }

 ch = *p;
 p++;

 return ch;
}

#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////

static  void yyunput (int c,char *buf_ptr  );
void subshell (char* yytext);
void removeQuotes (char* yytext);
void sourcefunc(FILE *f);

void myunputc(int c) {
  unput(c);
}


%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"|" {
  return PIPE;
}

">>" {
  return GREATGREAT;
}

"<" {
  return LESS;
}

"2>" {
  return TWOGREAT;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"&" {
  return AMPERSAND;
}


\$\([^\n\$]*\)|`[^\n`]*` {
  /* subshell */
  //printf("?");
  subshell(yytext);
}



[^ \t\n|><&]*\\[^ \t\n]* {
	
	/* Allow the escape character. */
	
	int i=0;
	char * escChar;
	escChar = new char[strlen(yytext)*2];
	char * args = yytext;

  while (*args) {
    if (*args == '\\'){
      args++; //skip '\'
    }
    escChar[i] = *args;
    args++;
    i++;
  }
  escChar[i] = '\0';
  
	yylval.cpp_string = new std::string(escChar);
	return WORD;
}

["][^\n]*["] {
  removeQuotes (yytext);
  return WORD;
}

[^ \t\n|><][^ \t\n|><]* {
  /* \| ??? */
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

%% 

/***********          subroutine START         *************/

void subshell ( char * yytext){
  //create subshell
  /* there are 2 forms: $(command) or `command` */

  std::string *command_Pointer = new std::string(yytext);
  std::string command = command_Pointer->c_str(); //need std::string for string manipulation
  //const char *commandD = command_Pointer->c_str();
  //printf("%s", commandD);
  


  // form1: `command` 
  if(command.back() == '`'){
    command = command.substr(1, command.length()-2);   
  }
  

  // form2: $(command) 
  else if(command.back() == ')'){
    command = command.substr(2, command.size()-3);
  }

  char *commandP = new char[command.length() + 1];
  strcpy(commandP, command.c_str());
  
  /*** preprocess in parent process START ***/
  /** set I/O redirection of subshell **/
  int tmpin = dup(0);
  int tmpout = dup(1);

  /* parent shell create 2 pipes */
  int PIN[2];
  int POUT[2];
  /* parent: PIN[1] out-->in PIN[0] :subshell */
  /* subshell: POUT[1] out-->in POUT[0] : parent */

  pipe(PIN);
  pipe(POUT);



  /* parent shell write command out to PIN[1]  */
  write(PIN[1], commandP, command.length());  //note: commandP is of type char*
  write(PIN[1], "\n", 1);
  write(PIN[1], "exit", 4);
  write(PIN[1], "\n", 1);
  close(PIN[1]);

  /** subshell I/O redirecion and command execution START **/
  /* subshell input redirection: read input from PIN[0] */
  dup2(PIN[0], 0);
  close(PIN[0]);
  //subshell output redirection: write output to POUT[1]
  dup2(POUT[1], 1);
  close(POUT[1]);  

  /* execute shell from proc/self/execute */
  int ret = fork();
  /* child process */
  if (ret == 0){
    execvp("/proc/self/exe", NULL);
    _exit(1);
  }else if (ret < 0){
    perror("fork");
    exit(1);
  }

  /** subshell I/O redirecion and command execution END **/
  /* Now, we have the output left in POUT[0] with a dead subshell */

  /* restore I/O defaults for parent shell */
  dup2(tmpin, 0);
  dup2(tmpout, 1);
  close(tmpin);
  close(tmpout);
    /* Hint: It is common for students to redirect the current shell’s 
    stdin and stdout file descriptors to communicate with the subshell process
    , however this is not necessary. 
    The current shell can communicate with the subshell
    by writing to the pipes directly. */

  /** parent shell recieve the output from subshell and lex it to WORD **/
  /* parent shell reads output from POUT[0] and write it to a buffer char by char */
  char buffer [2333];
  char tmp; //temporarily store the char currently being read
  int i = 0;
  
  /* ssize_t read(int fd, void *buf, size_t count), return 0 indicates end of file */
  /* The read command will return -1 when the eof is reached ????  */
  while (read(POUT[0], &tmp, 1) != 0){
    if( tmp == '\n'){
      buffer[i] = ' '; 
    }else{
      buffer[i] = tmp;
    }
    i++; 
  }
  /* i from 0 to length */
  buffer[i] = '\0';

  /* int yy_unput(int c) returns one character of the corresponding int back to lex */
  /* parent shell unput the characters back to lex in reverse order */
  i--; //skip '\0'
  for (i; i >=0; i--){
    unput(buffer[i]);
  }

  //free(buffer);
}

void removeQuotes (char* yytext){
  
  // remove quotes 
  std::string *textPointer = new std::string(yytext);
  std::string text = textPointer->c_str();

  text = text.substr(1, text.length()-2); //removing external quotes

  // then pass the string surrounded by quotes to yaac 
  yylval.cpp_string = new std::string(text);   

  /*
  char noQuotes_string [strlen(yytext)];
  char * tmp = yytext;

  int i = 0;
  while ( *tmp ) {
    if( *tmp == '\"'){
      tmp++; //skip "
      noQuotes_string[i] = *tmp;
    }else {
      noQuotes_string[i] = *tmp;
    }
    i++;
    tmp++;
  }
  noQuotes_string[i] = '\0';
  yylval.cpp_string = new std::string(noQuotes_string); */
}


void sourcefunc(FILE *f){
	YY_BUFFER_STATE file = (yy_create_buffer(f, YY_BUF_SIZE));
	yypush_buffer_state(file);
	yyparse();
	yypop_buffer_state();
}

