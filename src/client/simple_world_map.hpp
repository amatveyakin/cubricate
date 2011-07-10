#ifndef SIMPLE_WORLD_MAP_HPP
#define SIMPLE_WORLD_MAP_HPP


#include "common/multidim_array.hpp"
#include "common/world_block.hpp"


class SimpleWorldMap {
public:
  SimpleWorldMap (int sizeX, int sizeY, int sizeZ);
  ~SimpleWorldMap ();

  void lockRepaint ();
  void unlockRepaint ();

  // TODO: replace with operator()  (?)

  WorldBlock get (Vec3i pos) const                          { return m_blocks (pos); }
  WorldBlock get (int x, int y, int z) const                { return m_blocks (x, y, z); }

  void set (Vec3i pos, BlockType newBlockType);
  void set (Vec3i pos, WorldBlock newWorldBlock);
  void set (int x, int y, int z, BlockType newBlockType);
  void set (int x, int y, int z, WorldBlock newWorldBlock);

  void swapCubes (Vec3i firstPos, Vec3i secondPos);

  void saveToFile () const;
  void loadFromFile ();

  static const char* WORLD_MAP_FILE_NAME;

protected:
  Array3D <WorldBlock> m_blocks;
  int m_nRepaintLocks;

  void doUnlockRepaint (bool octreeUpdateNeighboursFlag);
};


#endif
