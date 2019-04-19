
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>  //back-up and resume orinal terminal settings

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];


// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [150];

int history_length = sizeof(history)/sizeof(char *);

void read_line_print_usage()
{
	char * usage = "\n"
		" ctrl-?       Print usage\n"
		" Backspace    Deletes last character\n"
		" up arrow     See last command in the history\n";

	write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {
	// save terminal settings
	struct termios orig_attr;
	tcgetattr(0, &orig_attr);

	// Set terminal in raw mode
	tty_raw_mode();

	line_buffer[0] = '\0'; 
	line_length = 0;
	int line_pos = line_length;   
	int add = 0;
	int sub = 0;
	// Read one line until enter is typed
	while (1) {
		//printf("%d\n", track);
		// Read one character in raw mode.
		char ch;
		read(0, &ch, 1);
		/********* ch>=32 START *********/
		if (ch>=32) {
			// Delete 
			if (ch == 127){
				if (line_length > 0 && line_pos > 0){
					// Move the cursor to the very left
					for (int i = 0; i < line_pos; i++){
						ch = 8;
						write(1, &ch, 1);
					}
					line_pos --;

					// Refresh the message
					char buffer[sizeof(line_buffer)];
					int j = 0;
					for (int i = 0; i < line_length; i ++) {
						if (i != line_pos) {
							buffer[j] = line_buffer[i];
							write(1, &buffer[j], 1);
							j++;
						}
					}
					ch = ' ';
					write(1, &ch, 1);

					// Move the cursor to the current position
					for (int i = 0; i < (line_length-line_pos); i++){
						ch = 27;
						write(1, &ch, 1);
						ch = 91;
						write(1, &ch, 1);
						ch = 68;
						write(1, &ch, 1);
					}

					//update the line buffer
					line_length--;
					strncpy(line_buffer, buffer, line_length);
				}
				continue;
			} // end of ch=127

			// If max number of character reached, then return.
			if (line_length==MAX_BUFFER_LINE-2) break; 

			// Do echo
			write(1,&ch,1);
			if (line_pos < line_length){
				char buffer[sizeof(line_buffer)];
				int j = 0;
				for (int i = 0; i < line_length; i++){
					if (i == line_pos){
						buffer[j] = ch;
						j+=1;
					} 
					if (i > line_pos-1){
						write(1, &line_buffer[i], 1);
					}
					buffer[j] = line_buffer[i];
					j++;
				}   

				
				// add char to buffer.

				for (int i = 0; i < (line_length-line_pos); i++){
					ch = 27;
					write(1, &ch, 1);
					ch = 91;
					write(1, &ch, 1);
					ch = 68;
					write(1, &ch, 1);
				}

				line_length++;
				strncpy(line_buffer, buffer, line_length);

			}
			else {
				line_length++;
				line_buffer[line_pos] = ch;	
			}
			line_pos++;
		} 
		/*********  ch>=32 END  *********/


		/**** ENTER START ****/
		else if (ch==10) {
			write(1,&ch,1);
			break;
		} 
		/**** ENTER END ****/

		/**** Ctrl E = END START ****/
		else if(ch == 5){
			while (line_pos != line_length){
				ch = 27;
				write(1, &ch, 1);
				ch = 91;
				write(1, &ch, 1);
				ch = 67;
				write(1, &ch, 1);
				line_pos++;
			}
		} 
		/**** Ctrl E = END END ****/

		/**** Ctrl A = Home START ****/
		else if (ch == 1){
			while (line_pos > 0){
				ch = 27;
				write(1, &ch, 1);
				ch = 91;
				write(1, &ch, 1);
				ch = 68;
				write(1, &ch, 1);
				line_pos--;
			}
		}
		/**** Ctrl A = Home END ****/




		/**** Ctrl H = Backspace START ****/
		else if (ch == 8) {
			if (line_length > 0 && line_pos > 0){
				// Move the cursor to the very left
				for (int i = 0; i < line_pos; i++){
					ch = 8;
					write(1, &ch, 1);
				}
				line_pos --;

				// Refresh the message
				char buffer[sizeof(line_buffer)];
				int j = 0;
				for (int i = 0; i < line_length; i ++) {
					if (i != line_pos) {
						buffer[j] = line_buffer[i];
						write(1, &buffer[j], 1);
						j++;
					}
				}
				ch = ' ';
				write(1, &ch, 1);

				// Move the cursor to the current position
				for (int i = 0; i < (line_length-line_pos); i++){
					ch = 27;
					write(1, &ch, 1);
					ch = 91;
					write(1, &ch, 1);
					ch = 68;
					write(1, &ch, 1);
				}

				//update the line buffer
				line_length--;
				strncpy(line_buffer, buffer, line_length);
			}
		}
		/**** Ctrl H = Backspace END ****/

		/**** Ctrl D = Delete START ****/
		else if (ch == 4) {
			if (line_length > 0 && line_pos < line_length){
				// Move the cursor to the very left
				for (int i = 0; i < line_pos; i++){
					ch = 8;
					write(1, &ch, 1);
				}
				//line_pos --;

				// Refresh the message
				char buffer[sizeof(line_buffer)];
				int j = 0;
				for (int i = 0; i < line_length; i ++) {
					if (i != line_pos) {
						buffer[j] = line_buffer[i];
						write(1, &buffer[j], 1);
						j++;
					}
				}
				ch = ' ';
				write(1, &ch, 1);

				// Move the cursor to the current position
				for (int i = 0; i < (line_length-line_pos); i++){
					ch = 27;
					write(1, &ch, 1);
					ch = 91;
					write(1, &ch, 1);
					ch = 68;
					write(1, &ch, 1);
				}

				//update the line buffer
				line_length--;
				strncpy(line_buffer, buffer, line_length);
			} 
		}
		/**** Ctrl D = Delete END ****/

		else if (ch==27) {
			// Escape sequence. Read two chars more
			//
			// HINT: Use the program "keyboard-example" to
			// see the ascii code for the different chars typed.
			//
			char ch1; 
			char ch2;
			read(0, &ch1, 1);
			read(0, &ch2, 1);
			if (ch1==91 && ch2==65) {
				// Up arrow. Print last line in history.

				// Erase old line
				// Print backspaces
				//printf("lc: %d; ll: %d",line_pos, line_length);
				while (line_pos > 0){
					ch = 8;
					write(1, &ch, 1);
					ch = ' ';
					write(1, &ch, 1);
					ch = 8;
					write(1, &ch, 1);
					line_pos--;
				}
				
				
				// Copy line from history
				if (history_length > 0 && history_index-add+sub-1 >= 0){
					strcpy(line_buffer, history[history_index-add+sub-1]);
					add++;
					line_length = strlen(line_buffer);
				} else {
					strcpy(line_buffer, history[history_index-add+sub]);
				}
				// echo line
				write(1, line_buffer, line_length);
				line_pos = line_length;

			} // end of 91 65
			// down arrow
			else if (ch1 == 91 && ch2 == 66){
				// print backspaces
				
				while (line_pos > 0){
                    ch = 8;
                    write(1, &ch, 1);
                    ch = ' ';
                    write(1, &ch, 1);
                    ch = 8;
                    write(1, &ch, 1);
                    line_pos--;
                }
				
				// Copy line from history
				if (history_length > 0 && history_index-add+sub+1 < history_index){
					strcpy(line_buffer, history[history_index-add+sub+1]);
					sub++;
					line_length = strlen(line_buffer);
				} else {
                    strcpy(line_buffer, history[history_index-add+sub]);
                }
                // echo line
                write(1, line_buffer, line_length);
                line_pos = line_length;
			} // end of 91 66
			// left arrow
			else if (ch1 == 91 && ch2 == 68) {
				if (line_pos > 0){
					ch = 27;
					write(1, &ch, 1);
					ch = 91;
					write(1, &ch, 1);
					ch = 68;
					write(1, &ch, 1);
					line_pos--;
				}
			} // end of 91 68
			// right arrow
			else if (ch1 == 91 && ch2 == 67){
				if (line_pos < line_length){
					ch = 27;
					write(1, &ch, 1);
					ch = 91;
					write(1, &ch, 1);
					ch = 67;
					write(1, &ch, 1);
					line_pos++;
				}
			} // end of 91 67
		} // end of ch=27
	}
	
	// Add eol and null char at the end of string
	line_buffer[line_length]=10;
	line_length++;
	line_buffer[line_length] = 0;

	history[history_index] = (char*)malloc(strlen(line_buffer)*sizeof(char) + 1);
	strcpy(history[history_index], line_buffer);
	history[history_index][strlen(line_buffer)-1] = '\0';
	history_index += 1;
	add = 0;
	sub = 0;

	// set the terminal back to orig settings
	tcsetattr(0, TCSANOW, &orig_attr);

	return line_buffer;
}

