#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"

// image:
#include "images/bckg.h"
#include "images/gameover.h"
#include "images/win.h"
#include "images/finish.h"
#include "images/pika.h"


enum gba_state {
  START_SCREEN_INIT,
  START_TEXT_INIT,
  START_AWAIT,
  PLAY_INIT,
  PLAY,
  END_INIT,
  END
};

struct square player;
int hasWon;
struct square enemyColumn[ENEMY_COUNT];

int main(void) {
  REG_DISPCNT = MODE3 | BG2_ENABLE;

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = START_SCREEN_INIT;

  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    switch (state) {
      case START_SCREEN_INIT:
        waitForVBlank();
        drawFullScreenImageDMA(background);
        state = START_TEXT_INIT;
        break;

      case START_TEXT_INIT:
        waitForVBlank();
        drawCenteredString(0,0, WIDTH, HEIGHT, "World's Hardest GBA Game", BLACK);
        drawImageIgnore(HEIGHT / 2 + 20, WIDTH / 2 - PIKA_WIDTH, PIKA_WIDTH, PIKA_HEIGHT, pika, WHITE);
        drawString(120, 75, "Press A to Play", BLACK);
        state = START_AWAIT;
        break;

      case START_AWAIT:
        if(KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons)) {
          state = PLAY_INIT;
        }
        break;

      case PLAY_INIT:
        hasWon = 0;
        init_player(&player, 
                    HEIGHT / 2 - PLAYER_SIZE, 
                    PLAYER_START_COL, 
                    PLAYER_SIZE, PLAYER_SPEED, PLAYER_COLOR);
        
        init_column(enemyColumn, ENEMY_COUNT, ENEMY_SPEED, 20, 190);

        waitForVBlank();
        // background
        fillScreenDMA(GAME_BACKGROUND_COLOR);
        // victory / end zone
        drawRectDMA(0, VICTORY_ZONE_START, WIDTH - VICTORY_ZONE_START, HEIGHT, CYAN);
        drawImageDMA(5, 205, FINISH_WIDTH, FINISH_HEIGHT, finish);
        // player
        drawImageDMA(player.row, player.col, PIKA_WIDTH, PIKA_HEIGHT, pika);

        for (int i = 0; i < ENEMY_COUNT; i++) {
          drawRectDMA(enemyColumn[i].row, enemyColumn[i].col, enemyColumn[i].size, enemyColumn[i].size, enemyColumn[i].color);
        }

        // this is to reset "frame counter" so that the score is based on how quick you start
        vBlankCounter = 0;
        state = PLAY;
        break;

      case PLAY: ;
        struct square previousP;
        struct square previousE[ENEMY_COUNT];
        if (PLAYER_WALL_COLLIDED(player, HEIGHT) || player_collided_enemy(&player, enemyColumn, ENEMY_COUNT)) {
          hasWon = 0;
          state = END_INIT;
          break;
        }

        if (PLAYER_VICTORY(player, VICTORY_ZONE_START)) {
          hasWon = 1;
          state = END_INIT;
          break;
        }

        /* player movement */
        previousP = player;
        if (KEY_DOWN(BUTTON_LEFT, currentButtons)) {
          move_player_left(&player);
        }
        if (KEY_DOWN(BUTTON_RIGHT, currentButtons)) {
          move_player_right(&player);
        }
        if (KEY_DOWN(BUTTON_UP, currentButtons)) {
          move_player_up(&player);
        }
        if (KEY_DOWN(BUTTON_DOWN, currentButtons)) {
          move_player_down(&player);
        }


        /* enemy movement */
        for (int i = 0; i < ENEMY_COUNT; i++) {
          previousE[i] = enemyColumn[i];
          enemyColumn[i].col += enemyColumn[i].speed;
          if (enemyColumn[i].col > 190 || enemyColumn[i].col < 20) {
            enemyColumn[i].speed = -enemyColumn[i].speed;
          }
        }

        /* debug if you want to have only 1 column move */
        /*for (int i = 0; i < ENEMY_COUNT; i++) {
          previousE[i] = enemyColumn[i];
          if (i % 2) { // change this to either == 0 or == 1
            enemyColumn[i].col += enemyColumn[i].speed;
            if (enemyColumn[i].col > 190 || enemyColumn[i].col < 20) {
             enemyColumn[i].speed = -enemyColumn[i].speed;
            }
          }
        }  */

        if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
          state = START_SCREEN_INIT;
        }

        waitForVBlank();
        // draw player
        drawRectDMA(previousP.row, previousP.col, previousP.size, previousP.size, GAME_BACKGROUND_COLOR);
        drawImageDMA(player.row, player.col, PIKA_WIDTH, PIKA_HEIGHT, pika);

        // draw enemy
        for (int i = 0; i < ENEMY_COUNT; i++) {
          drawRectDMA(previousE[i].row, previousE[i].col, 
                      previousE[i].size, previousE[i].size, GAME_BACKGROUND_COLOR);
          
          drawRectDMA(enemyColumn[i].row, enemyColumn[i].col, 
                      enemyColumn[i].size, enemyColumn[i].size, enemyColumn[i].color);
        }

        // draw score
        char buffer[6];
        sprintf(buffer, "%d", vBlankCounter);
        // wipe old score
        drawRectDMA(0,0,28,8, GAME_BACKGROUND_COLOR);
        drawString(0,0, buffer, BLACK);

        break;

      case END_INIT:
        if (hasWon) {
          char buffer[80];
          sprintf(buffer, "Score: %d", vBlankCounter);
          drawFullScreenImageDMA(win);
          drawCenteredString(-20, 0, WIDTH, HEIGHT, buffer, WHITE);
        } else {
          drawFullScreenImageDMA(gameover);
        }
        drawCenteredString(30, 0, WIDTH, HEIGHT, "Press Select to Reset", WHITE);
        state = END;
        break;
      case END:
        if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
          state = START_SCREEN_INIT;
        }
        break;
     
    }

    previousButtons = currentButtons; // Store the current state of the buttons
  }

  return 0;
}


void init_player(struct square* player, int row, int col, int size, int speed, u16 color) {
    player->row = row;
    player->col = col;
    player->size = size;
    player->speed = speed;
    player->color = color;
}

void init_column(struct square* column, int count, int speed, int start_col1, int start_col2) {
  for (int i = 0; i < count; i+=2) {
    column[i].col = start_col1;
    column[i].row = i * 10;
    column[i].speed = speed;
    column[i].size = 8;
    column[i].color = BLUE;

    
    column[i + 1].col = start_col2;
    column[i + 1].row = i * 10 + 10;
    column[i + 1].speed = speed;
    column[i + 1].size = 8;
    column[i + 1].color = BLUE;
  }
}

void move_player_left(struct square* player) {
  player->col -= player->speed;
}

void move_player_right(struct square* player) {
  player->col += player->speed;
}

void move_player_up(struct square* player) {
  player->row -= player->speed;
}

void move_player_down(struct square* player) {
  player->row += player->speed;
}

int player_collided_enemy(struct square* player, struct square* column, int length) {
  int index = player->row / 10;
  // player is too the right of
  if (player->col + player->size > column[index].col) {
    if (column[index].col + column[index].size > player->col) {
      return 1;
    }
  }
  // need to check the lower as well so row checking needs to happen here too
  // never need to check above with how this algorithm is designed.
  if (index < length - 1) {
    if (player->col + player->size > column[index + 1].col) {
      if (column[index + 1].col + column[index + 1].size >=player->col) {
        if (player->row + player->size > column[index + 1].row) {
          return 1;
        }
      }
    }
  }

  return 0;
}