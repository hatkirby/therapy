#ifndef CONSTS_H
#define CONSTS_H

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT - 1;
const int WALL_GAP = 5;
const int TILESET_COLS = 8;
const int FONT_COLS = 16;

const int FRAMES_PER_SECOND = 60;
const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;

#define CALC_VELOCITY(h, l) (-2 * (h) / (l))
#define CALC_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

const double NORMAL_GRAVITY = CALC_GRAVITY(TILE_HEIGHT*3.5, 0.233);
const double JUMP_GRAVITY = CALC_GRAVITY(TILE_HEIGHT*4.5, 0.3);
const double JUMP_VELOCITY = CALC_VELOCITY(TILE_HEIGHT*4.5, 0.3);

const double WALK_SPEED = 90;

#endif
