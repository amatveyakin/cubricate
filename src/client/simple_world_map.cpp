#include "client/glwidget.hpp"
#include "client/client_world.hpp"
#include "client/simple_world_map.hpp"


SimpleWorldMap::SimpleWorldMap (int sizeX, int sizeY, int sizeZ) :
  m_blocks (sizeX, sizeY, sizeZ)
{
  std::fill (m_blocks.data (), m_blocks.data () + sizeX * sizeY * sizeZ, BT_AIR);
  m_nRepaintLocks = 0;
}

SimpleWorldMap::~SimpleWorldMap () { }


void SimpleWorldMap::lockRepaint () {
  m_nRepaintLocks++;
  renderingEngine->lockCubes ();
}

void SimpleWorldMap::unlockRepaint () {
  doUnlockRepaint (true);
}


void SimpleWorldMap::set (int x, int y, int z, BlockType newBlockType) {
  m_blocks (x, y, z) = newBlockType;
  if (m_nRepaintLocks == 0) {
    lockRepaint ();
    cubeOctree.set (x, y, z, newBlockType, true);
    doUnlockRepaint (false);
  }
  else
    cubeOctree.set (x, y, z, newBlockType, false);
}



void SimpleWorldMap::doUnlockRepaint (bool octreeUpdateNeighboursFlag) {
  m_nRepaintLocks--;
  assert (m_nRepaintLocks >= 0);
  if (m_nRepaintLocks == 0) {
    if (octreeUpdateNeighboursFlag)
      cubeOctree.computeNeighbours ();
    renderingEngine->unlockCubes ();
    renderingEngine->paintGL ();
  }
}
