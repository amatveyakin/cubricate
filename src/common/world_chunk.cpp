#include "common/utils.hpp"
#include "common/game_parameters.hpp"
#include "common/math_utils.hpp"
#include "common/string_utils.hpp"
#include "common/world_chunk.hpp"


const int MAX_CHUNK_COORDINATE_STRING_LENGTH = 8;

const int CHUNK_ACTIVE_RADIUS  = 8;
const int CHUNK_PRESERVE_RADIUS = 10;

const int CHUNK_ACTIVE_RADIUS_SQR  = xSqr (CHUNK_ACTIVE_RADIUS);
const int CHUNK_PRESERVE_RADIUS_SQR = xSqr (CHUNK_PRESERVE_RADIUS);



WorldChunk::WorldChunk () {
  // ...
}

WorldChunk::~WorldChunk () {
  // ...
}


// TODO: precompute state and return it immediately
ChunkState WorldChunk::state () const {
  if (!playersUsingChunk.empty ())
    return ChunkState::ACTIVE;
  else if (!playersKeepingChunk.empty ())
    return ChunkState::PRESERVED;
  else
    return ChunkState::REDUNDANT;
}


void WorldChunk::onPlayerMove (int playerId, Vec3d oldPos, Vec3d newPos) {
  FIX_UNUSED (oldPos);
  playersUsingChunk.erase   (playerId);
  playersKeepingChunk.erase (playerId);
  ChunkState necessity = necessityForPlayer (Vec2i::fromVectorConverted (newPos.xy ()), m_pos);
  switch (necessity) {
    case ChunkState::ACTIVE:
      playersUsingChunk.insert   (playerId);
      break;
    case ChunkState::PRESERVED:
      playersKeepingChunk.insert (playerId);
      break;
    case ChunkState::REDUNDANT:
      break;
  }
}


// int WorldChunk::packToBuffer (DataBuffer& buffer) const {
//   // ...
// }
//
// int WorldChunk::unpackFromBuffer (DataBuffer& buffer) {
//   // ...
// }


int WorldChunk::loadFromDisk () {
  // ...
  return 0;
}

int WorldChunk::saveToDisk () const {
  // ...
  return 0;
}


static std::string coordinateToFileNamePart (int coord) {
  return   ((coord < 0) ? "m" : "p")
         + intToStr (xAbs (coord), MAX_CHUNK_COORDINATE_STRING_LENGTH);
}

std::string WorldChunk::correspondingFileName () {
  return coordinateToFileNamePart (m_pos.x) + '-' + coordinateToFileNamePart (m_pos.y);
}



ChunkState WorldChunk::necessityForPlayer (Vec2i playerChunk, Vec2i targetChunk) {
  int distSqr = L2::distanceSqr (playerChunk, targetChunk);
  if (distSqr <= CHUNK_ACTIVE_RADIUS_SQR)
    return ChunkState::ACTIVE;
  else if (distSqr <= CHUNK_PRESERVE_RADIUS_SQR)
    return ChunkState::PRESERVED;
  else
    return ChunkState::REDUNDANT;
}
