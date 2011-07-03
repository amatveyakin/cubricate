#ifndef GAME_PARAMETERS_HPP
#define GAME_PARAMETERS_HPP


// TODO: delete this block
const int     TREE_HEIGHT = 6;
const int     MAP_SIZE = 1 << TREE_HEIGHT;
const int     N_MAP_BLOCKS = MAP_SIZE * MAP_SIZE * MAP_SIZE;



const int     CHUNK_SIZE = 16;
const int     MAP_HEIGHT = 256;
const int     MAP_HEIGHT_IN_CHUNKS = MAP_HEIGHT / CHUNK_SIZE;

static_assert (MAP_HEIGHT % CHUNK_SIZE == 0, "Map height must be divisible by chunk size");


const int     PLAYER_SIGHT_RADIUS = 9; // chunks

const int     SCREEN_WIDTH  = 1024;
const int     SCREEN_HEIGHT = 1024;

const int     RAY_PACKET_WIDTH  = 4;
const int     RAY_PACKET_HEIGHT = 4;

#endif
