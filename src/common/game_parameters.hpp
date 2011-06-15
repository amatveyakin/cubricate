#ifndef GAME_PARAMETERS_HPP
#define GAME_PARAMETERS_HPP


// TODO: delete this block
const int TREE_HEIGHT = 6;
const int MAP_SIZE = 1 << TREE_HEIGHT;
const int N_MAP_BLOCKS = MAP_SIZE * MAP_SIZE * MAP_SIZE;


const int CHUNK_SIZE = 16;
const int MAP_HEIGHT = 256;
// static_assert (MAP_HEIGHT % CHUNK_SIZE == 0, "Map height should be divisible by chunk size");


#endif
