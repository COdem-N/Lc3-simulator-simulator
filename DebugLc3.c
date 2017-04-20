#include <ncurses.h>
#define HEIGHT 10
#define WIDTH 10


WINDOW *create_newwin(int height, int width, int starty, int startx);


int main(int argc, char *argv[])
{	initscr();
WINDOW *win = newwin(5, 5, 1, 1);

start_color();
init_pair(1, COLOR_BLACK, COLOR_WHITE);
//init_pair(2, COLOR_BLACK, COLOR_WHITE);

wbkgd(stdscr, COLOR_PAIR(1));
//wbkgd(win, COLOR_PAIR(2));
refresh();
//wrefresh(win);

 int ch[256];
 int i = 0;
 while(ch[i] != '\n') 
 {
    ch[i] = getch();
    i++;
   
   // printw("-biteme-");
 }
  //  sleep(3);
  
     printw("%c",ch[i]);
     
    
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}
