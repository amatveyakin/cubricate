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


void SimpleWorldMap::set (Vec3i pos, BlockType newBlockType) {
  set (pos, WorldBlock (newBlockType));
}

void SimpleWorldMap::set (Vec3i pos, WorldBlock newWorldBlock) {
  if (newWorldBlock.type == BT_WATER && m_blocks (pos).type != BT_WATER)
    waterEngine.addWaterCube (pos);
  else if (newWorldBlock.type != BT_WATER && m_blocks (pos).type == BT_WATER)
    waterEngine.removeWaterCube (pos);

  m_blocks (pos) = newWorldBlock;
  if (m_nRepaintLocks == 0) {
    lockRepaint ();
    cubeOctree.set (pos.x (), pos.y (), pos.z (), newWorldBlock.type, true);
    doUnlockRepaint (false);
  }
  else
    cubeOctree.set (pos.x (), pos.y (), pos.z (), newWorldBlock.type, false);
}

void SimpleWorldMap::set (int x, int y, int z, BlockType newBlockType) {
  set (Vec3i (x, y, z), newBlockType);
}

void SimpleWorldMap::set (int x, int y, int z, WorldBlock newWorldBlock) {
  set (Vec3i (x, y, z), newWorldBlock);
}


void SimpleWorldMap::swapCubes (Vec3i firstPos, Vec3i secondPos) {
  WorldBlock tmp = m_blocks (firstPos);
  set (firstPos, m_blocks (secondPos));
  set (secondPos, tmp);
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
