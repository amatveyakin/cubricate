#ifndef WORLD_CHUNK_HPP
#define WORLD_CHUNK_HPP


#include <vector>
#include <string>

#include "common/world_block.hpp"

class DataBuffer;


class WorldChunk {
public:
  WorldChunk();
  ~WorldChunk();

  int loadFromDisk ();
  int saveToDisk () const;

  int unpackFromBuffer (DataBuffer& buffer);
  int packToBuffer (DataBuffer& buffer) const;

  std::string getCorrespondingFileName ();

protected:
  int m_posX;
  int m_posY;
//   Array3d <MapBlock> m_blocks;
};


#endif
