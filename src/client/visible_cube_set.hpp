#ifndef VISIBLE_CUBE_SET_HPP
#define VISIBLE_CUBE_SET_HPP


#include <cassert>
#include <vector>
#include <set>
#include <map>

#include "common/linear_algebra.hpp"
#include "common/game_parameters.hpp"
#include "common/world_block.hpp"
#include "common/world_chunk.hpp"



typedef float CubePositionT;  // TODO: (?)
typedef float CubeTypeT;      // 'cause we need to send it to the videocard in one vec4


// TODO: replace isEmpty with blockIsVisible
// TODO: refactor these classes the make their interface more native with current project structure
class CubeArray {
public:
  CubeArray (int sizeX, int sizeY, int sizeZ);
  ~CubeArray ();

  int         sizeX () const            { return m_sizeX; }
  int         sizeY () const            { return m_sizeY; }
  int         sizeZ () const            { return m_sizeZ; }
  int         maxCubes () const         { return m_maxCubes; }
  int         nCubes () const           { return m_nCubes; }

  const CubePositionT*  cubePositions () const { return m_cubePositionArray; }
  const CubeTypeT*      cubeTypes () const     { return m_cubeTypesArray; }

  void setPointers (CubePositionT* cubePositions, CubeTypeT* cubeTypes) ;

  void addCube (int x, int y, int z, CubeTypeT type);
  void removeCube (int x, int y, int z);

protected:
  int   m_sizeX;
  int   m_sizeY;
  int   m_sizeZ;
  int   m_maxCubes;
  int   m_nCubes;

  CubePositionT*      m_cubePositionArray;
  CubeTypeT*          m_cubeTypesArray;
  std::vector <int>   m_mapPositionToCubeIndex;

  void checkCoordinates (int x, int y, int z) const;

  int xyzToPosition (int x, int y, int z) const   { return  x * m_sizeY * m_sizeZ  +  y * m_sizeZ  +  z; }

  static CubeTypeT emptyCube ()                   { return static_cast <CubeTypeT> (BT_AIR); }
  static CubeTypeT isEmpty (CubeTypeT cubeType)   { return cubeType == emptyCube (); }
};


// TODO: refactor class structure (inheritance seems irrelevant here)
// TODO: rename (?)
class VisibleCubeSet : public CubeArray {
public:
  VisibleCubeSet (int sizeX, int sizeY, int sizeZ);
  ~VisibleCubeSet ();

  int  addCube (int x, int y, int z, CubeTypeT type);
  int  removeCube (int x, int y, int z);

  void changeNExternalNeighbours (int x, int y, int z, CubeTypeT type, int neighboursDelta);

  CubeTypeT cubeType (int x, int y, int z) const {
    checkCoordinates (x, y, z);
    int position = xyzToPosition (x, y, z);
    return m_mapCubeTypes [position];
  }

  bool cubePresents (int x, int y, int z) const {
    return !isEmpty (cubeType (x, y, z));
  }

protected:
  std::vector <CubeTypeT>   m_mapCubeTypes;
  std::vector <CubeTypeT>   m_mapNCubeNeighbours;

  void getNeighbourLimits (int x, int y, int z, int& xMin, int& yMin, int& zMin, int& xMax, int& yMax, int& zMax) ;

  void removeNeighbour (int x, int y, int z) ;
  void addNeighbour (int x, int y, int z) ;
};


class ChunksForRender {
public:
  ChunksForRender ();
  ~ChunksForRender ();

  void init (int cubesInformationOffset);

  void addRenderChunk (Vec3i renderChunk, ChunkState state, const Array3D <WorldBlock>& worldChunkCubes);
  void removeRenderChunk (Vec3i renderChunk);
  void setRenderChunkState (Vec3i renderChunk, ChunkState newState);

  void setCube (Vec3i renderChunk, Vec3i cubeInChunk, BlockType type);

protected:
  int                           m_cubesInformationOffset;
  std::set <int>                m_freeRenderChunks;
  std::map <Vec3i, int, LexicographicCompareVec3i> m_renderChunksPosToIndex;
  std::vector <VisibleCubeSet>  m_renderChunksCubeSets;
  std::vector <ChunkState>      m_renderChunksStates;

  int  getRenderChunkIndex (Vec3i renderChunkPos);

  void lockRenderChunk (int iRenderChunk);
  void unlockRenderChunks ();

  int  setRenderChunkCube (Vec3i renderChunk, int x, int y, int z, BlockType type);

  void chanegNExternalNeighboursInRenderChunk (Vec3i renderChunk, int x, int y, int z, BlockType type, int neighboursDelta);
};


#endif
