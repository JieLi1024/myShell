README

myShell is a shell interpreter which combines behavior from common shells including bash and csh.

------------------ Functions of myShell ---------------------------
Part 1: Parsing and Executing Commands
	1.1 Use Lex and Yacc to preprocess input commands/arguments
	1.2 Executing commands
		● Simple command process creation and execution 
		● File redirection
		● Pipes
		● isatty()

Part 2: Signal Handling, More Parsing, and Subshells 
	2.1: Ctrl­C: exit from the current running process in myShell but not quit from myShell(parent process)
	2.2: Zombie Elimination 
	2.3: Exit: quit from myShell(parent process)
	2.4: Quotes
	2.5: Escaping
	2.6: Builtin Functions: printenv, setenv, unsetenv, source
	2.7: Creating a Default Source File: “.shellrc” 
	2.8: Subshells


Part 3: Expansions, Wildcards, and Line Editing 
	3.1: Environment variable expansion
		3.1.1: ${$} The PID of the shell process
   		3.1.2: ${!} PID of the last process run in the background
   		3.1.3: ${_} The last argument in the fully expanded previous command Note: this excludes redirects
  	3.2: Tilde expansion
	3.3: Wildcarding
	3.4: Edit mode
		● Left arrow key: Move the cursor to the left and allow insertion at that position. If the cursor is at the beginning of the line it does nothing.
		● Right arrow key: Move the cursor to the right and allow insertion at that position. If the cursor is at the end of the line it does nothing.
		● Delete key (ctrl­D): Removes the character at the cursor. The characters in the right side are shifted to the left.
		● Backspace key (ctrl­H): Removes the character at the position before the cursor. The characters in the right side are shifted to the left.
		● Home key (ctrl­A): The cursor moves to the beginning of the line 
		● End key (ctrl­E): The cursor moves to the end of the line
	3.5: History
		● Up arrow key: Shows the previous command in the history list. 
		● Down arrow key: Shows the next command in the history list.
	3.6: Path completion: When the <tab> key is typed, the editor will try to expand the current word to the matching files similar to what csh and bash do
	3.7: Variable prompt
		The shell has a default prompt indicator: myprompt>. If there is an environment variable called PROMPT, my shell can print the value of that variable as the prompt instead.


--------------- How to run myShell ------------------
inside the myShell directory:
Run make
Run shell
type commands allowed according to above specifications
use exit to quit

--------------- examples --------------
see examples in test-shell directory




