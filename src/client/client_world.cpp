#include "common/game_parameters.hpp"  // TODO: delete (?)
#include "client/client_world.hpp"


WorldMap        worldMap;
ChunksForRender chunksForRender;
Player          player;

VisibleCubeSet  cubeArray (MAP_SIZE, MAP_SIZE, MAP_SIZE);
Octree          cubeOctree (TREE_HEIGHT);
