#ifndef CLIENT_WORLD_HPP
#define CLIENT_WORLD_HPP


#include "common/world_map.hpp"
#include "common/player.hpp"
#include "common/physics/water_engine.hpp"
#include "common/simple_world_map.hpp"
#include "common/simple_light_map.hpp"

#include "client/octree.hpp"


class GLWidget;


// extern  WorldMap        worldMap;
extern  SimpleWorldMap  simpleWorldMap; // TODO: delete and use worldMap
extern  SimpleLightMap  simpleLightMap; // TODO: delete and use lightMap
extern  Player          player;
extern  Octree          cubeOctree;
extern  WaterEngine     waterEngine;
extern  GLWidget*       renderingEngine; // TODO: delete (?)


#endif
