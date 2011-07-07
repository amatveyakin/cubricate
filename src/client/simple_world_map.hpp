#ifndef SIMPLE_WORLD_MAP_HPP
#define SIMPLE_WORLD_MAP_HPP


#include "common/multidim_array.hpp"


class SimpleWorldMap {
public:
  SimpleWorldMap (int sizeX, int sizeY, int sizeZ);
  ~SimpleWorldMap ();

  void lockRepaint ();
  void unlockRepaint ();

  BlockType get (int x, int y, int z) const                 { return m_blocks (x, y, z); }
  void set (int x, int y, int z, BlockType newBlockType);

protected:
  Array3D <BlockType> m_blocks;
  int m_nRepaintLocks;

  void doUnlockRepaint (bool octreeUpdateNeighboursFlag);
};


#endif
