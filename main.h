#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

// TODO: Create any necessary structs

/*
* For example, for a Snake game, one could be:
*
* struct snake {
*   int heading;
*   int length;
*   int row;
*   int col;
* };
*
* Example of a struct to hold state machine data:
*
* struct state {
*   int currentState;
*   int nextState;
* };
*
*/

#define PLAYER_START_COL 10
#define PLAYER_SIZE 5
#define PLAYER_COLOR RED
#define PLAYER_SPEED 2

#define ENEMY_COUNT 16
#define ENEMY_SPEED 2

#define GAME_BACKGROUND_COLOR WHITE
#define VICTORY_ZONE_START 200

#define PLAYER_WALL_COLLIDED(p, h) ((p).col <= 0 || \
         (p).row <= 0 || \
         (p).row + (p).size >= (h))

#define PLAYER_VICTORY(p, vz) ((p).col >= vz)

struct square {
    int row;
    int col;

    int size;
    int speed;
    u16 color;
};

void init_player(struct square* player, int row, int col, int size, int speed, u16 color);
void init_column(struct square* column, int count,  int speed, int start_col1, int start_col2);
int player_collided_enemy(struct square* player, struct square* column, int length);

void move_player_left(struct square* player);
void move_player_right(struct square* player);
void move_player_up(struct square* player);
void move_player_down(struct square* player);

#endif
