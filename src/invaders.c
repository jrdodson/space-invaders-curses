#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define CHAR ACS_BTEE
#define FIRE ACS_BULLET
#define BAD ACS_UARROW
pthread_mutex_t mutex;
int delay; 

typedef struct {
	int curx;
	int cury;
} BULLET;
BULLET bullet;

typedef struct {
	int coords[100][100];
	int starty;
	int cury;
	int curx;
	bool status;
} BADGUYS;
BADGUYS bgSet;

void draw_char(int y, int x) {
	pthread_mutex_lock(&mutex);
	mvaddch(y,x,CHAR);
	refresh();
	pthread_mutex_unlock(&mutex);
}
void reset_badguys() {
	int i, j;
	for(i = 0; i < 100; i++) {
		for(j=0; j < 27; j++) {
			if(bgSet.coords[i][j]!=-1) {
				pthread_mutex_lock(&mutex);
				bgSet.coords[i][j]=0;
				pthread_mutex_unlock(&mutex);
			}
		}
	}
}
void draw_badguys() {
	bool not_dead = false;
	int i, j;
	for(i = 0; i < 100; i++) {
		for(j=0; j < 27; j++) {
			if(bgSet.coords[i][j] == 1) {
				not_dead=true;
				pthread_mutex_lock(&mutex);
				mvaddch(j, i, BAD);
				refresh();	
				pthread_mutex_unlock(&mutex);
			}
			else { erase_coords(j, i); }
		}
	}
	bgSet.status = not_dead;
}
void erase_coords (int y, int x) {
	pthread_mutex_lock(&mutex);	
	mvaddch(y, x, ' ');
	pthread_mutex_unlock(&mutex);
}
void init_curses() {
	initscr();
	clear();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);	
}
void set_badguys(int y, int x) {
	int startx = x;	
	int tempx, tempy;

	while(y>bgSet.starty) {	

		while(x!=75) {
			if(bgSet.coords[x][y]!=-1) {
				pthread_mutex_lock(&mutex);
				bgSet.coords[x][y] = 1;
				pthread_mutex_unlock(&mutex);
			}
			x+=5;
		}
		y-=4;
		x = startx;
	}
	bgSet.starty+=1;
	draw_badguys();
}

bool check_hits(int y, int x) {
	//if(bgSet.coords[x][y]==1) {
		pthread_mutex_lock(&mutex);			
		bgSet.coords[x][y]=-1;
		pthread_mutex_unlock(&mutex);
		return true;
	//}
	//return false;
}

void fire(int y, int x) {
	bullet.cury=y+1;
	bullet.curx=x+1;

	mvaddch(bullet.cury, bullet.curx, FIRE);
	while(bullet.cury) {
		usleep(10000);
		erase_coords(bullet.cury, bullet.curx);
		bullet.cury-=1;
		mvaddch(bullet.cury,bullet.curx, FIRE);
		check_hits(bullet.cury, bullet.curx);
		refresh();
	}
	erase_coords(bullet.cury, bullet.curx);
	refresh();
}
void game_loop(int y, int x, int ch) {
	if(ch == 'q'){ return; }
	mvaddch(y, x, CHAR);
	refresh(); 
	while(1){
		ch = getch(); 
		switch(ch) {
			case KEY_LEFT:
				erase_coords(y, x);
				x-=1;
				draw_char(y,x);
				break;
			case KEY_RIGHT:
				erase_coords(y, x);
				x+=1;
				draw_char(y,x);
				break;
			case KEY_UP:
				fire(y, x);
				refresh();
				break;
			case 'q': return;
			default: break;
		}
	}
}

void game_over() {
	clear();
	printw("Game over\n");
	getch();
	endwin();
	exit(1);
}

void* badguy_fn(void* data) {
	bgSet.starty=0;
	bgSet.curx=5;
	bgSet.cury=10;
	bgSet.status=true;

	reset_badguys();
	while(1) {	
		if(bgSet.cury==28 || bgSet.status==false) { game_over(); }
		sleep(delay);
		set_badguys(bgSet.cury, bgSet.curx);
		bgSet.cury+=1;
		reset_badguys();
	}
}
void set_difficulty(int ch) {
	switch(ch) {
		case 'h':
			delay=1;
			break;
		case 'n': 
			delay=2;
			break;
		case 'e':
			delay=3;
			break;
		default:
			delay=3;
	}
}
int main(int argc, char** argv) {
	pthread_t t1;
	const char* m1 = "thread1";
	pthread_mutex_init(&mutex, NULL);
	
	int row = 28, col = 37;

	init_curses();

	printw("Welcome to Space Invaders.\nPlease select your difficulty:\n(h)ard\n(n)ormal\n(e)asy\nTo quit press \'q\'");

	int ch = getch();
	clear();	
	set_difficulty(ch);
	pthread_create(&t1, 0, badguy_fn, (void*)m1);
	game_loop(row, col, ch);

	
	game_over();

	return 0;
}
