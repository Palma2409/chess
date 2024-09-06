#include "xchess.h"

static cell **box;

//получение ссылки на одноименный массив структур из дисплейного модуля
int relink(cell **boxing){
    box = boxing;
    return 0;
}

//функция атаки одиночной фигуры
int attack(int start_x, int start_y, int end_x, int end_y){
    if ((start_x == end_x) || (start_y == end_y) || (start_x + start_y == end_x + end_y) || ((start_x - start_y) == (end_x - end_y)))
        return UNDERRATACK;
    return NOTUNDERRATACK;
}

//установка полей атаки для отдельной фигуры
int set_attack(int y, int x){
    int ny, nx;
    for (ny = y, nx = x+1; nx < COLS; nx++)			/* просмотр клеток справа */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y+1, nx = x; ny < ROWS; ny++)			/* просмотр клеток снизу */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y, nx = x-1; nx >= 0; nx--)			/* просмотр клеток слева */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y-1, nx = x; ny >= 0; ny--)			/* просмотр клеток сверху */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));

    for (ny = y+1, nx = x+1; ny < ROWS && nx < COLS; ny++, nx++)	/* просмотр клеток по диагонали справа снизу */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y-1, nx = x-1; ny >= 0 && nx >= 0; ny--, nx--)	/* просмотр клеток по диагонали слева сверху */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y+1, nx = x-1; ny < ROWS && nx >= 0; ny++, nx--)	/* просмотр клеток по диагонали слева снизу */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));
    for (ny = y-1, nx = x+1; ny >= 0 && nx < COLS; ny--, nx++)	/* просмотр клеток по диагонали справа сверху */
        box[ny][nx].type = UNDERRATACK+(2*((nx + ny) % 2));

    return 0;
}

//перерисовка полей атаки фигур
int reattack(){
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            box[i][j].type = NOTUNDERRATACK+(2*((i + j) % 2));

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (box[i][j].figure == 1)
                set_attack(i,j);

    return 0;
}

//проверка правил изменения позиции фигуры
int CheckRules(int start_x, int start_y, int end_x, int end_y){
    if (box[end_y][end_x].figure == 1)
        return 0;
    if (attack(start_x, start_y, end_x, end_y)) { 
        box[end_y][end_x].figure = 1;
        box[start_y][start_x].figure = 0;
        reattack();
    }
    return 0;
}

//задание начального расположение фигур
int desk_setter(){
    static short position[8][8] = {{1, 0, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 1, 0},
                                   {0, 0, 0, 0, 1, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 1},
                                   {0, 1, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 1, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 1, 0, 0},
                                   {0, 0, 1, 0, 0, 0, 0, 0}};

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            box[i][j].figure = position[i][j];

    reattack();
    return 0;
}

