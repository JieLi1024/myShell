README

1. Features specified in the handout that work.

Part 1: Parsing and Executing Commands
	Part 1A: Lex and Yacc ­ Accepting more complex commands 
	Part 1B: Executing commands
		1B.1: Simple command process creation and execution 
		1B.2: File redirection
		1B.3: Pipes
		1B.4: isatty()

Part 2: Signal Handling, More Parsing, and Subshells 
	2.1: Ctrl­C
	2.2: Zombie Elimination 
	2.3: Exit
	2.4: Quotes
	2.5: Escaping
	2.6: Builtin Functions
	2.7: Creating a Default Source File: “.shellrc” 
	2.8: Subshells


Part 3: Expansions, Wildcards, and Line Editing 
	3.1: Environment variable expansion
		3.1.1: ${$} The PID of the shell process
   		3.1.3: ${!} PID of the last process run in the background
   		3.1.4: ${_} The last argument in the fully expanded previous command Note: this excludes redirects
  	3.2: Tilde expansion
	3.3: Wildcarding
	3.4: Edit mode
		● Left arrow key: Move the cursor to the left and allow insertion at that position. If the cursor is at the beginning of the line it does nothing.
		● Right arrow key: Move the cursor to the right and allow insertion at that position. If the cursor is at the end of the line it does nothing.
		● Delete key (ctrl­D): Removes the character at the cursor. The characters in the right side are shifted to the left.
		● Backspace key (ctrl­H): Removes the character at the position before the cursor. The characters in the right side are shifted to the left.
		● Home key (ctrl­A): The cursor moves to the beginning of the line ● End key (ctrl­E): The cursor moves to the end of the line
	3.5: History
		● Up arrow key: Shows the previous command in the history list. ● Down arrow key: Shows the next command in the history list.
	3.6: Path completion: When the <tab> key is typed, the editor will try to expand the current word to the matching files similar to what csh and bash do
	3.7: Variable prompt
		The shell has a default prompt indicator: myprompt>. If there is an environment variable called PROMPT, my shell can print the value of that variable as the prompt instead.


2. Features specified in the handout that do not work. 
(Optional):
	2.9: Process Substitution
(Mandatory):
	3.1.2: ${?} The return code of the last executed simple command (ignoring commands sent to the background).


3. Extra features you have implemented.
	The following are specified in handout but will be given extra points according to grading rubric:

	3.6: Path completion: When the <tab> key is typed, the editor will try to expand the current word to the matching files similar to what csh and bash do
	3.7: Variable prompt
	    	The shell has a default prompt indicator: myprompt>. If there is an environment variable called PROMPT, my shell can print the value of that variable as the prompt instead.



