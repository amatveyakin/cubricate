#ifndef CLIENT_WORLD_HPP
#define CLIENT_WORLD_HPP


#include "common/world_map.hpp"
#include "common/player.hpp"
#include "client/simple_world_map.hpp"
#include "client/octree.hpp"


class GLWidget;


// extern  WorldMap        worldMap;
extern  SimpleWorldMap  simpleWorldMap; // TODO: delete and use worldMap
extern  Player          player;
extern  Octree          cubeOctree;
extern  GLWidget*       renderingEngine; // TODO: delete (?)


#endif
