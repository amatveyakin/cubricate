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
  const WorldChunk& chunkRef = it->second;
  if (   it == m_chunks.end ()
      || chunkRef.state () != ChunkState::ACTIVE)
    return BlockType::EMPTY;
  return chunkRef.cube (cubeInChunk);
}


void WorldMap::onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos) {
  for (auto it = m_chunks.begin (); it != m_chunks.end (); ) {
    WorldChunk& chunkRef = it->second;
    ++it;  // We can now remove current object from the set
    chunkRef.onPlayerMove (playerId, oldPos, newPos);
    if (chunkRef.state () == ChunkState::REDUNDANT) {
      chunkRef.saveToDisk ();
      m_chunks.erase (it);
    }
  }
}
