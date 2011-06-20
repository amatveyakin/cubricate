#include "common/cube_geometry.hpp"
#include "common/world_map.hpp"


WorldMap::WorldMap() {
  // ...
}

WorldMap::~WorldMap() {
  // ...
}


BlockType WorldMap::cube (Vec3i cubePos) const {
  Vec2i chunk;
  Vec3i cubeInChunk;
  cubeToChunk (cubePos, chunk, cubeInChunk);
  auto it = m_chunks.find (chunk);
  if (   it == m_chunks.end ()
      || it->second.state () != ChunkState::ACTIVE)
    return BlockType::EMPTY;
  return it->second.cube (cubeInChunk);
}


void WorldMap::onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos) {
  for (auto it = m_chunks.begin (); it != m_chunks.end (); ) {
    WorldChunk& curChunk = it->second;
    ++it;  // We can now remove current object from the set
    curChunk.onPlayerMove (playerId, oldPos, newPos);
    if (curChunk.state () == ChunkState::REDUNDANT) {
      curChunk.saveToDisk ();
      m_chunks.erase (it);
    }
  }
}
