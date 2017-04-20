#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
 
int main()
{
 char mesg[]=">";		/* message to be appeared on the screen */
 char str[80];
 int row,col;				/* to store the number of rows and *
					 * the number of colums of the screen */
 initscr();				/* start the curses mode */
 getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

 mvprintw(5,5,"Registers",mesg);
 mvprintw(5,30,"Memory",mesg);
                     		/* print the message at the center of the screen */
 getstr(str);
 mvprintw(LINES - 2, 0, "You Entered: %s", str);
 getch();
 endwin();

 return 0;
}