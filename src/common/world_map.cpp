#include "common/cube_geometry.hpp"
#include "common/world_map.hpp"


const int CHUNK_USE_RADIUS  = 8;
const int CHUNK_KEEP_RADIUS = 10;


WorldMap::WorldMap() {
  // ...
}

WorldMap::~WorldMap() {
  // ...
}


ChunkNecessity WorldMap::chunkNecessityForPlayer (Vec2i playerChunk, Vec2i targetChunk) {
  int distSqr = euclideanDistanceSqr (playerChunk, targetChunk);
  if (distSqr <= CHUNK_USE_RADIUS)
    return ChunkNecessity::USE;
  else if (distSqr <= CHUNK_KEEP_RADIUS)
    return ChunkNecessity::KEEP;
  else
    return ChunkNecessity::UNLOAD;
}
