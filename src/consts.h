#ifndef CONSTS_H
#define CONSTS_H

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT - 1;
const int WALL_GAP = 6;
const int TILESET_COLS = 8;
const int FONT_COLS = 16;

const int FRAMES_PER_SECOND = 60;
const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;

#define JUMP_VELOCITY(h, l) (-2 * (h) / (l))
#define JUMP_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

#endif
