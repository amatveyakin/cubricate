#ifndef WORLD_MAP_HPP
#define WORLD_MAP_HPP


#include <map>

#include "common/world_chunk.hpp"
#include "common/linear_algebra.hpp"


enum class ChunkNecessity {
  USE,
  KEEP,
  UNLOAD
};


class WorldMap {
public:
  WorldMap();
  ~WorldMap();

  static ChunkNecessity chunkNecessityForPlayer (Vec2i playerChunk, Vec2i targetChunk);

protected:
  std::map <Vec2i, WorldChunk, LexicographicCompareVec2i> m_chunks;
};


#endif
