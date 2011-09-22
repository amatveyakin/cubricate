#ifndef WORLD_CHUNK_HPP
#define WORLD_CHUNK_HPP


#include <vector>
#include <set>
#include <string>

#include "common/utils.hpp"
#include "common/linear_algebra.hpp"
#include "common/multidim_array.hpp"
#include "common/world_block.hpp"


class DataBuffer;
class ChunksForRender;


#if CPP11_ENUM_ENCHANTMENTS
enum class ChunkState {
#else // !CPP11_ENUM_ENCHANTMENTS
enum ChunkState {
#endif // !CPP11_ENUM_ENCHANTMENTS
  ACTIVE,
  PRESERVED,
  REDUNDANT
};


class WorldChunk {
public:
  WorldChunk (Vec2i pos);
  ~WorldChunk ();

  ChunkState state () const;

  WorldBlock cube (Vec3i cubeInChunk) const;
  void setCube (Vec3i cubeInChunk, WorldBlock newBlock);

  void onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos);

  int unpackFromBuffer (DataBuffer& buffer);
  int packToBuffer (DataBuffer& buffer) const;

  int loadFromDisk ();
  int saveToDisk () const;

  std::string correspondingFileName ();

protected:
  Vec2i           m_pos;

  std::set <int>  m_chunkIsActiveFor;
  std::set <int>  m_chunkIsReservedFor;

  Array3D <WorldBlock>  m_cubes;

  ChunkState necessityForPlayer (Vec2i playerChunk, Vec2i targetChunk);
};


#endif
