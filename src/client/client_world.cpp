#include "common/game_parameters.hpp"  // TODO: delete (?)

#include "client/glwidget.hpp"
#include "client/client_world.hpp"


// WorldMap        worldMap;
SimpleWorldMap  simpleWorldMap (MAP_SIZE, MAP_SIZE, MAP_SIZE);
Player          player;
Octree          cubeOctree (TREE_HEIGHT);
WaterEngine     waterEngine;
GLWidget*       renderingEngine;
