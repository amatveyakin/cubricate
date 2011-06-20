#ifndef WORLD_CHUNK_HPP
#define WORLD_CHUNK_HPP


#include <vector>
#include <set>
#include <string>

#include "common/linear_algebra.hpp"
#include "common/world_block.hpp"

class DataBuffer;


enum class ChunkState {
  ACTIVE,
  PRESERVED,
  REDUNDANT
};


class WorldChunk {
public:
  WorldChunk();
  ~WorldChunk();

  ChunkState state () const;

  BlockType cube (Vec3i cubePos) const;

  void onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos);

  int unpackFromBuffer (DataBuffer& buffer);
  int packToBuffer (DataBuffer& buffer) const;

  int loadFromDisk ();
  int saveToDisk () const;

  std::string correspondingFileName ();

protected:
  Vec2i m_pos;

  std::set <int> playersUsingChunk;
  std::set <int> playersKeepingChunk;
//   Array3d <MapBlock> m_blocks;

  ChunkState necessityForPlayer (Vec2i playerChunk, Vec2i targetChunk);
};


#endif
