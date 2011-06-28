#ifndef CLIENT_WORLD_HPP
#define CLIENT_WORLD_HPP


#include "common/world_map.hpp"
#include "common/player.hpp"

// TODO: delete
#include "client/visible_cube_set.hpp"
#include "client/octree.hpp"



extern  WorldMap        worldMap;
extern  ChunksForRender chunksForRender;
extern  Player          player;

// TODO: delete
// extern  VisibleCubeSet  cubeArray;
extern  Octree          cubeOctree;


#endif
