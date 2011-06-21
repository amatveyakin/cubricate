// TODO refactor variableNames and (brackets)

#ifndef CUBE_ARRAY_HPP
#define CUBE_ARRAY_HPP


#include <cassert>
#include <vector>


template <typename CubePositionT, typename CubeTypeT>
class CubeArray {
public:
  CubeArray (int sizeX, int sizeY, int sizeZ) :
    m_sizeX (sizeX),
    m_sizeY (sizeY),
    m_sizeZ (sizeZ),
    m_maxCubes (sizeX * sizeY * sizeZ),
    m_nCubes (0),

    m_cubePositionArray (0),
    m_cubeTypesArray (0),
    m_mapPositionToCubeIndex (m_maxCubes, -1)
  {
  }

  ~CubeArray() { }

  int         sizeX () const            { return m_sizeX; }
  int         sizeY () const            { return m_sizeY; }
  int         sizeZ () const            { return m_sizeZ; }
  int         maxCubes () const         { return m_maxCubes; }
  int         nCubes () const           { return m_nCubes; }

  const CubePositionT*  cubePositions () const { return m_cubePositionArray; }
  const CubeTypeT*      cubeTypes () const     { return m_cubeTypesArray; }

  void setPointers (CubePositionT* cubePositions, CubeTypeT* cubeTypes) {
    m_cubePositionArray = cubePositions;
    m_cubeTypesArray    = cubeTypes;
  }

  void addCube (int x, int y, int z, int type) {
    if (isEmpty (type))
      return removeCube (x, y, z);
    checkCoordinates (x, y, z);
    int position = xyzToPosition (x, y, z);
    int index = m_mapPositionToCubeIndex [position];
    if (index >= 0) {
      m_cubeTypesArray [index] = type;
      return;
    }
    m_mapPositionToCubeIndex [position] = m_nCubes;
    m_cubePositionArray [m_nCubes * 4    ] = x;
    m_cubePositionArray [m_nCubes * 4 + 1] = y;
    m_cubePositionArray [m_nCubes * 4 + 2] = z;
    m_cubePositionArray [m_nCubes * 4 + 3] = 0.5;
    m_cubeTypesArray [m_nCubes] = type;
    m_nCubes++;
  }

  void removeCube (int x, int y, int z) {
    checkCoordinates (x, y, z);
    int position = xyzToPosition (x, y, z);
    int index = m_mapPositionToCubeIndex [position];
    if (index < 0)
      return;
    m_nCubes--;
    int lastCubePosition = xyzToPosition (m_cubePositionArray [m_nCubes * 4], m_cubePositionArray [m_nCubes * 4 + 1], m_cubePositionArray [m_nCubes * 4 + 2]);
    m_mapPositionToCubeIndex [lastCubePosition] = index;
    m_mapPositionToCubeIndex [position] = -1;
    std::copy (m_cubePositionArray + m_nCubes * 4, m_cubePositionArray + (m_nCubes + 1) * 4, m_cubePositionArray + index * 4);
    m_cubeTypesArray [index] = m_cubeTypesArray [m_nCubes];
  }

protected:
  int   m_sizeX;
  int   m_sizeY;
  int   m_sizeZ;
  int   m_maxCubes;
  int   m_nCubes;

  CubePositionT*    m_cubePositionArray;
  CubeTypeT*        m_cubeTypesArray;
  std::vector< int >  m_mapPositionToCubeIndex;

  void checkCoordinates (int x, int y, int z) const {
    assert (x >= 0);
    assert (y >= 0);
    assert (z >= 0);
    assert (x < m_sizeX);
    assert (y < m_sizeY);
    assert (z < m_sizeZ);
  }

  int xyzToPosition (int x, int y, int z) const {
    return  x * m_sizeY * m_sizeZ  +  y * m_sizeZ  +  z;
  }

  static CubeTypeT emptyCube ()                   { return CubeTypeT(); }
  static CubeTypeT isEmpty (CubeTypeT cubeType)   { return cubeType == emptyCube(); }
};


// TODO: rename (?)
template <typename CubePositionT, typename CubeTypeT>
class VisibleCubeSet : public CubeArray <CubePositionT, CubeTypeT> {
private:
  typedef CubeArray <CubePositionT, CubeTypeT> Parent;

public:
  VisibleCubeSet (int sizeX, int sizeY, int sizeZ) :
    Parent (sizeX, sizeY, sizeZ),
    m_mapCubeTypes (Parent::m_maxCubes),
    m_mapNCubeNeighbours (Parent::m_maxCubes, 0)
  {
    for (int x = 0; x < sizeX; ++x)
      for (int y = 0; y < sizeY; ++y) {
        m_mapNCubeNeighbours [Parent::xyzToPosition (x, y, 0)]++;
        m_mapNCubeNeighbours [Parent::xyzToPosition (x, y, sizeZ - 1)]++;
      }

    for (int y = 0; y < sizeY; ++y)
      for (int z = 0; z < sizeZ; ++z) {
        m_mapNCubeNeighbours [Parent::xyzToPosition (0, y, z)]++;
        m_mapNCubeNeighbours [Parent::xyzToPosition (sizeX - 1, y, z)]++;
      }

    for (int x = 0; x < sizeX; ++x)
      for (int z = 0; z < sizeZ; ++z) {
        m_mapNCubeNeighbours [Parent::xyzToPosition (x, 0, z)]++;
        m_mapNCubeNeighbours [Parent::xyzToPosition (x, sizeY - 1, z)]++;
      }
  }

  ~VisibleCubeSet() { }

  void addCube (int x, int y, int z, int type) {
    Parent::checkCoordinates (x, y, z);
    int position = Parent::xyzToPosition (x, y, z);

    if (m_mapCubeTypes [position] == type)
      return;

    if (Parent::isEmpty (type))
      return removeCube (x, y, z);

    CubeTypeT oldType = m_mapCubeTypes [position];
    m_mapCubeTypes [position] = type;
    if (m_mapNCubeNeighbours [position] < 6)
      Parent::addCube (x, y, z, type);

    if (!Parent::isEmpty (oldType))
      return;

    int iMin, jMin, kMin, iMax, jMax, kMax;
    getNeighbourLimits (x, y, z, iMin, jMin, kMin, iMax, jMax, kMax);

    for (int i = iMin; i <= iMax; i += 2)
      addNeighbour (i, y, z);
    for (int j = jMin; j <= jMax; j += 2)
      addNeighbour (x, j, z);
    for (int k = kMin; k <= kMax; k += 2)
      addNeighbour (x, y, k);
  }

  void removeCube (int x, int y, int z) {
    Parent::checkCoordinates (x, y, z);
    int position = Parent::xyzToPosition (x, y, z);
    if (Parent::isEmpty (m_mapCubeTypes [position]))
      return;

    m_mapCubeTypes [position] = Parent::emptyCube();
    Parent::removeCube (x, y, z);

    int iMin, jMin, kMin, iMax, jMax, kMax;
    getNeighbourLimits (x, y, z, iMin, jMin, kMin, iMax, jMax, kMax);

    for (int i = iMin; i <= iMax; i += 2)
      removeNeighbour (i, y, z);
    for (int j = jMin; j <= jMax; j += 2)
      removeNeighbour (x, j, z);
    for (int k = kMin; k <= kMax; k += 2)
      removeNeighbour (x, y, k);
  }

  CubeTypeT cubeType (int x, int y, int z) const {
    Parent::checkCoordinates (x, y, z);
    int position = Parent::xyzToPosition (x, y, z);
    return m_mapCubeTypes [position];
  }

  bool cubePresents (int x, int y, int z) const {
    return !Parent::isEmpty (cubeType (x, y, z));
  }

protected:
  std::vector <CubeTypeT> m_mapCubeTypes;
  std::vector <CubeTypeT> m_mapNCubeNeighbours;

  void getNeighbourLimits (int x, int y, int z, int& xMin, int& yMin, int& zMin, int& xMax, int& yMax, int& zMax) {
    xMin = x - 1;
    yMin = y - 1;
    zMin = z - 1;
    if (xMin < 0)  xMin += 2;
    if (yMin < 0)  yMin += 2;
    if (zMin < 0)  zMin += 2;

    xMax = x + 1;
    yMax = y + 1;
    zMax = z + 1;
    if (xMax > Parent::m_sizeX - 1)  xMax -= 2;
    if (yMax > Parent::m_sizeY - 1)  yMax -= 2;
    if (zMax > Parent::m_sizeZ - 1)  zMax -= 2;
  }

  void removeNeighbour (int x, int y, int z) {
    int position = Parent::xyzToPosition (x, y, z);
    if (m_mapNCubeNeighbours [position] == 6)
      Parent::addCube (x, y, z, m_mapCubeTypes [position]);
    assert (m_mapNCubeNeighbours [position] > 0);
    m_mapNCubeNeighbours [position]--;
  }

  void addNeighbour (int x, int y, int z) {
    int position = Parent::xyzToPosition (x, y, z);
    assert (m_mapNCubeNeighbours [position] < 6);
    m_mapNCubeNeighbours [position]++;
    if (m_mapNCubeNeighbours [position] == 6)
      Parent::removeCube (x, y, z);
  }
};


#endif
