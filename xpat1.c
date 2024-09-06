#include <X11/Xlib.h>
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdlib.h>
#include "xchess.h"

static  Display  *dpy;  		/*  Адрес  дисплейной  структуры */
static GC gc;
static  Window  desk;  			/*  Окно  игрового  поля  программы */
static cell** box;
	    /* Адрес массива NYxNX клеток */

static unsigned long colors[4];

/*  Настройка  графических  параметров */
int  xcustom()  {
    int  x,  y;  /*  Позиции  окон */
    unsigned  w,  h;  /*  Габариты  окон */
    int  depth  =  DefaultDepth(dpy,  0);  /*  Глубина  экрана  0  */
    Window  root;  /*  Корневое  окно  экрана  */
    XSetWindowAttributes  attr;  /*  Атрибуты  окон */
    unsigned  long  amask;  /*  Маска  оконных  атрибутов */
    XSizeHints  hint;  /*  Геометрия  оконного  менеджмента */
    int  i,  j;  /*  индексы  окон */
    Colormap colormap;  /*карта цветов*/

/*  Настройка  графических  контекстов */

    root  =  DefaultRootWindow(dpy);  /*  Корневое  окно  экрана */
    gc = XCreateGC(dpy, root, 0, NULL);

/*Настройка цветов палитры*/
    colormap = XDefaultColormap(dpy, DefaultScreen(dpy));
    XColor RGBpixel;
    XParseColor(dpy, colormap, "#ffffff", &RGBpixel);       //white
    XAllocColor(dpy, colormap, &RGBpixel);
    colors[0] = RGBpixel.pixel;
    XParseColor(dpy, colormap, "#FA595F", &RGBpixel);       //pink
    XAllocColor(dpy, colormap, &RGBpixel);
    colors[1] = RGBpixel.pixel;
    XParseColor(dpy, colormap, "#aaaaaa", &RGBpixel);       //grey
    XAllocColor(dpy, colormap, &RGBpixel);
    colors[2] = RGBpixel.pixel;
    XParseColor(dpy, colormap, "#dc362d", &RGBpixel);       //dark pink
    XAllocColor(dpy, colormap, &RGBpixel);
    colors[3] = RGBpixel.pixel;

/*  Настройка  игрового  окна  программы */
    attr.override_redirect  =  False;  /*  WM  обрамление  окна */
    attr.background_pixel  =  colors[0];//0xFFFFFF;  /*  white  */
    amask  =  (CWOverrideRedirect  |  CWBackPixel );
    w  = COLS * CELLSIZE;    /*  Габариты */
    h  = ROWS * CELLSIZE;    /*  игрового  окна */
    x  =  0;  y  =  0;  /*  Начальные  координаты  окна  игры */
    desk  =  XCreateWindow(dpy,  root,  x,  y,  w,  h, 1, depth, InputOutput,
                           CopyFromParent, amask, &attr); /* Геометрические  рекомендации  оконного  менеджера */
    hint.flags  =  (PMinSize  |  PMaxSize  |  PPosition);
    hint.min_width  =  hint.max_width  =  w;  /*  ФИКСИРОВАТЬ */
    hint.min_height  =  hint.max_height  =  h;  /*  габариты  и */
    hint.x  =  x;  hint.y  =  y; /*  позицию  окна  игрового  поля */
    XSetNormalHints(dpy,  desk,  &hint);  /*  в  свойстве  WM  */
    XStoreName(dpy,  desk,  "xchess");  /*  Заголовок  окна */


/*  Настройка  окон  клеток  */
    amask  =  CWOverrideRedirect  |  CWBackPixel  |  CWEventMask | CWBackingStore;
    attr.override_redirect = True;  /* Отмена обрамления окна */
    attr.background_pixel = XWhitePixel(dpy, XDefaultScreen(dpy));
    attr.backing_store = Always;        /*сохранение конфигурации окна при заслонении*/
    attr.event_mask  =  (KeyPressMask  |  ExposureMask | ButtonPressMask);
    w = CELLSIZE;
    h = CELLSIZE;  /*  Габариты  окна  клетки */
    x = 0;
    y = 0;
    box = (cell **)calloc(ROWS, sizeof(cell*));
    for(i = 0; i < ROWS; i++) { /*  Цикл  по  рядам  клеток */
        box[i] = (cell *) calloc(COLS, sizeof(cell));
        x = 0;
        for (j = 0; j < COLS; j++) { /* Создать окна клеток */
            box[i][j].window = XCreateWindow(dpy, desk, x, y, w-1, h-1, 1, depth,
                                             InputOutput, CopyFromParent, amask,
                                             &attr);/*  Отображение  всех  окон  на  экране */
            box[i][j].type = NOTUNDERRATACK+(2*((i + j) % 2));    /*Значение по умолчанию не под атакой*/
            box[i][j].figure = 0;
            x += CELLSIZE;
        }
        y += CELLSIZE;
    }

    XMapWindow(dpy,  desk);
    XMapSubwindows(dpy,  desk);
    return(0);}
/*  xcustom  */


int  dispatch()  {  /*  Диспетчер  событий */
    XEvent  event;  /*  Структура  событий */

    int  done  =  0;  /*  Флаг  выхода */
    while(done  ==  0)  {  /*  Цикл  обработки  событий */
        XNextEvent(dpy,  &event);  /*  Чтение  событий */
        switch(event.type)  {
            case  Expose:
                redraw();  /*  Перерисовка */
                break;
            case  ButtonPress:
                done = do_step(&event); /*перерасположение фигур*/
                redraw();
                break;
            case  KeyPress:
                key_analiz(&event); /*анализ событий нажатий на клавишу*/
                break;
            default:  break;
        }  /*  switch  */
    } /* while */
    return(0);
}  /*  dispatch  */


/*  Ход  игры  или  корректировка  позиций */
int  do_step(XEvent*  ev)  {
    int cur_time = CurrentTime;

    Window root, child;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    unsigned mask_return;

/*опрос позиции курсора*/
    XQueryPointer(dpy, desk, &root, &child, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mask_return);
    int start_y = win_y_return / CELLSIZE;
    int start_x = win_x_return / CELLSIZE;
    if (box[start_y][start_x].figure == 0){
        return 1;
    }

    XGrabPointer(dpy, desk, True, 0
            , GrabModeAsync, GrabModeAsync, desk, None, cur_time);

/*создание окна фигуры */
    XSetWindowAttributes attr;
    unsigned long amask  =  CWOverrideRedirect  |  CWBackPixel  |  CWEventMask;
    attr.override_redirect = True;  /* Отмена обрамления окна */
    attr.background_pixel = XBlackPixel(dpy, XDefaultScreen(dpy));
    attr.event_mask  =  (KeyPressMask  |  ExposureMask | ButtonPressMask);
    Window Figure = XCreateWindow(dpy, desk, win_x_return - FIGURESIZE/2, win_y_return - FIGURESIZE/2, FIGURESIZE, FIGURESIZE, 0, DefaultDepth(dpy, DefaultScreen(dpy)),
                  InputOutput, CopyFromParent, amask,
                  &attr);

/*Создание и проведение процедуры drag&drop*/
    XMapWindow(dpy, Figure);
    unsigned long mask_return_past = mask_return;
    while (mask_return == mask_return_past){
        XQueryPointer(dpy, desk, &root, &child, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mask_return);
        XMoveWindow(dpy, Figure, win_x_return - FIGURESIZE/2, win_y_return - FIGURESIZE/2);
    }
    XDestroyWindow(dpy, Figure);

/*проверка правил*/
    CheckRules(start_x, start_y, win_x_return/CELLSIZE, win_y_return/CELLSIZE);

    XUngrabPointer(dpy, cur_time);
    return 0;

}  /*  repaint  */


//перерисовка состояния игры на карте
int  redraw(){
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
        {
            XSetForeground(dpy, gc, colors[box[i][j].type]);
            XFillRectangle(dpy, box[i][j].window, gc, 0, 0, CELLSIZE, CELLSIZE);
            if (box[i][j].figure != 0) {
                XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));
                XFillArc(dpy, box[i][j].window, gc, CELLSIZE/5, CELLSIZE/5, FIGURESIZE, FIGURESIZE, 0, 64 * 360);
            }
        }
    return 0;
}

//анализ нажатых клавиш
int  key_analiz(XEvent*  ev){
    if (ev->xkey.keycode == XKeysymToKeycode(dpy, XK_Escape)) {
        desk_setter();
        redraw();
    }
    if (ev->xkey.keycode == XKeysymToKeycode(dpy, XK_Q)) {
    	XDestroySubwindows(dpy, desk);
    	XDestroyWindow(dpy, desk);

    	XCloseDisplay(dpy);
    	return(0);
    }
    return 0;
}


int main(int argc, char *argv[]) {

    if (argc != 2)
        return 0;
    else {
        CELLSIZE = atoi(argv[1]);
        FIGURESIZE = CELLSIZE / 5 * 3;
    }
    dpy  =  XOpenDisplay(NULL);
    Window root = DefaultRootWindow(dpy);

    xcustom();
    relink(box);
    desk_setter();
    dispatch();

    XDestroySubwindows(dpy, desk);
    XDestroyWindow(dpy, desk);

    XCloseDisplay(dpy);
    return(0);
} /* main */
