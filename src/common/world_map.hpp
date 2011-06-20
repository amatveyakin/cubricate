#ifndef WORLD_MAP_HPP
#define WORLD_MAP_HPP


#include <map>

#include "common/world_chunk.hpp"
#include "common/linear_algebra.hpp"


class WorldMap {
public:
  WorldMap();
  ~WorldMap();

  BlockType cube (Vec3i cubePos) const;

  void onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos);

protected:
  std::map <Vec2i, WorldChunk, LexicographicCompareVec2i> m_chunks;
};


#endif
