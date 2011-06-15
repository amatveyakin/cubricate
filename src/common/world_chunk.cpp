#include "common/game_parameters.hpp"
#include "common/math_utils.hpp"
#include "common/string_utils.hpp"
#include "common/world_chunk.hpp"


const int MAX_COORDINATE_VALUE_LENGTH = 8;


WorldChunk::WorldChunk () {
  // ...
}

WorldChunk::~WorldChunk () {
  // ...
}


static std::string coordinateToFileNamePart (int coord) {
  return   ((coord < 0) ? "m" : "p")
         + intToStr (xAbs (coord), MAX_COORDINATE_VALUE_LENGTH);
}

std::string WorldChunk::getCorrespondingFileName () {
  return coordinateToFileNamePart (m_posX) + '-' + coordinateToFileNamePart (m_posY);
}
