#ifndef VISIBLE_CUBE_SET_HPP
#define VISIBLE_CUBE_SET_HPP


#include <cassert>
#include <vector>
#include <set>

#include "common/world_block.hpp"
#include "common/linear_algebra.hpp"


typedef float CubePositionT;  // TODO: (?)
typedef float CubeTypeT;      // 'cause we need to send it to the videocard in one vec4


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

  void addCube (int x, int y, int z, int type);
  void removeCube (int x, int y, int z);

protected:
  int   m_sizeX;
  int   m_sizeY;
  int   m_sizeZ;
  int   m_maxCubes;
  int   m_nCubes;

  CubePositionT*      m_cubePositionArray;
  CubeTypeT*         m_cubeTypesArray;
  std::vector <int>   m_mapPositionToCubeIndex;

  void checkCoordinates (int x, int y, int z) const;

  int xyzToPosition (int x, int y, int z) const   { return  x * m_sizeY * m_sizeZ  +  y * m_sizeZ  +  z; }

  static CubeTypeT emptyCube ()                   { return static_cast <CubeTypeT> (BlockType::EMPTY); }
  static CubeTypeT isEmpty (CubeTypeT cubeType)   { return cubeType == emptyCube (); }
};


// TODO: refactor class structure (inheritance seems irrelevant here)
// TODO: rename (?)
class VisibleCubeSet : public CubeArray {
public:
  VisibleCubeSet (int sizeX, int sizeY, int sizeZ);
  ~VisibleCubeSet ();

  void addCube (int x, int y, int z, int type);
  void removeCube (int x, int y, int z);

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

  void lockCubes ();
  void unlockCubes ();

private:
  std::set <int> m_freeSlices;
};


#endif
