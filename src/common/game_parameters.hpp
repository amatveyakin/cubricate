#ifndef GAME_PARAMETERS_HPP
#define GAME_PARAMETERS_HPP


#include "client/shaders_common.h"


const int     CHUNK_SIZE = 16;
const int     MAP_HEIGHT = 256;
const int     MAP_HEIGHT_IN_CHUNKS = MAP_HEIGHT / CHUNK_SIZE;

static_assert (MAP_HEIGHT % CHUNK_SIZE == 0, "Map height must be divisible by chunk size");


const double  GRAVITY = 20.;

const int     PLAYER_SIGHT_RADIUS = 9; // chunks


#endif
