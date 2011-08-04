#include <fstream>
#include <iostream>   // TODO: delete

#include <QDir>

#include "common/simple_world_map.hpp"

#include "client/glwidget.hpp"
#include "client/client_world.hpp"


const char* SimpleWorldMap::WORLD_MAP_FILE_NAME = "world/map";


SimpleWorldMap::SimpleWorldMap (int sizeX, int sizeY, int sizeZ) :
  m_blocks (sizeX, sizeY, sizeZ),
  m_highestPoint (sizeX, sizeY)
{
  m_blocks.fill (BT_AIR);
  m_highestPoint.fill (-1);
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
  if (pos.z() > m_highestPoint (pos.xy()) && BlockInfo::isFirm (newWorldBlock)) {
    m_highestPoint (pos.xy()) = pos.z();
  }
  else if (pos.z() == m_highestPoint (pos.xy()) && !BlockInfo::isFirm (newWorldBlock)) {
    int z;
    for (z = pos.z() - 1; z >= 0; --z)
      if (BlockInfo::isFirm (m_blocks (pos.x(), pos.y(), z)))
        break;
    m_highestPoint (pos.xy()) = z;
  }

  if (m_nRepaintLocks == 0) {
    lockRepaint ();
    cubeOctree.set (pos.x (), pos.y (), pos.z (), newWorldBlock, true);
    doUnlockRepaint (false);
  }
  else
    cubeOctree.set (pos.x (), pos.y (), pos.z (), newWorldBlock, false);
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


void SimpleWorldMap::loadFromFile () {
  std::ifstream streamIn (WORLD_MAP_FILE_NAME);
  assert (streamIn.is_open ());
  lockRepaint ();
  for (int x = 0; x < MAP_SIZE; ++x) {
    for (int y = 0; y < MAP_SIZE; ++y) {
      for (int z = 0; z < MAP_SIZE; ++z) {
        WorldBlock block;
        streamIn.read (reinterpret_cast <char *> (&block), sizeof (WorldBlock));
        assert (!streamIn.fail ());
        set (x, y, z, block);
      }
    }
  }
  std::cout << "Map loaded." << std::endl;
  unlockRepaint ();
}

void SimpleWorldMap::saveToFile () const {
  QDir curDur;
  bool result = curDur.mkpath ("world");
  FIX_UNUSED (result);
  assert (result);
  std::ofstream streamOut (WORLD_MAP_FILE_NAME);
  streamOut.write (reinterpret_cast <const char *> (m_blocks.data ()), m_blocks.totalElements () * sizeof (WorldBlock));
  assert (!streamOut.fail ());
  std::cout << "Map saved." << std::endl;
}



void SimpleWorldMap::doUnlockRepaint (bool octreeUpdateNeighboursFlag) {
  m_nRepaintLocks--;
  assert (m_nRepaintLocks >= 0);
  if (m_nRepaintLocks == 0) {
    if (octreeUpdateNeighboursFlag)
      cubeOctree.computeNeighbours ();
    renderingEngine->unlockCubes ();
    renderingEngine->updateGL ();
  }
}
