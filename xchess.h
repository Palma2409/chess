#ifndef XCHESS
#define XCHESS

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define ROWS 8
#define COLS 8
static unsigned CELLSIZE;
static unsigned FIGURESIZE;

#define UNDERRATACK 1
#define NOTUNDERRATACK 0

typedef struct cell{
    Window window;
    short figure;
    unsigned short type;
} cell;


/*xpat1.c*/
int xcustom();
int do_step(XEvent *);
int dispatch();
int redraw();
int key_analiz(XEvent*);
int main();


/*xpat2.c*/
int relink(cell **);
int set_attack(int, int );
int reattack();
int CheckRules(int , int , int , int );
int desk_setter();



#endif 
