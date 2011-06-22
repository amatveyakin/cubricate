#include "common/game_parameters.hpp"
#include "client/visible_cube_set.hpp"


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>


static_assert (MAP_HEIGHT % CHUNK_SIZE == 0, "Map height must be divisible by chunk size");

const int MAP_HEIGHT_IN_CHUNKS = MAP_HEIGHT / CHUNK_SIZE;
const int MAX_CHUNKS_FOR_RENDER = PLAYER_SIGHT_RADIUS * PLAYER_SIGHT_RADIUS * MAP_HEIGHT_IN_CHUNKS;  // TODO: estimate more precisely



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CubeArray

CubeArray::CubeArray (int sizeX, int sizeY, int sizeZ) :
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

CubeArray::~CubeArray() { }


void CubeArray::setPointers (CubePositionT* cubePositions, CubeTypeT* cubeTypes) {
  m_cubePositionArray = cubePositions;
  m_cubeTypesArray    = cubeTypes;
}


void CubeArray::addCube (int x, int y, int z, int type) {
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

void CubeArray::removeCube (int x, int y, int z) {
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


void CubeArray::checkCoordinates (int x, int y, int z) const {
  assert (x >= 0);
  assert (y >= 0);
  assert (z >= 0);
  assert (x < m_sizeX);
  assert (y < m_sizeY);
  assert (z < m_sizeZ);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VisibleCubeSet

VisibleCubeSet::VisibleCubeSet (int sizeX, int sizeY, int sizeZ) :
    CubeArray (sizeX, sizeY, sizeZ),
    m_mapCubeTypes (m_maxCubes),
    m_mapNCubeNeighbours (m_maxCubes, 0) {
  for (int x = 0; x < sizeX; ++x)
    for (int y = 0; y < sizeY; ++y) {
      m_mapNCubeNeighbours [xyzToPosition (x, y, 0) ]++;
      m_mapNCubeNeighbours [xyzToPosition (x, y, sizeZ - 1) ]++;
    }

  for (int y = 0; y < sizeY; ++y)
    for (int z = 0; z < sizeZ; ++z) {
      m_mapNCubeNeighbours [xyzToPosition (0, y, z) ]++;
      m_mapNCubeNeighbours [xyzToPosition (sizeX - 1, y, z) ]++;
    }

  for (int x = 0; x < sizeX; ++x)
    for (int z = 0; z < sizeZ; ++z) {
      m_mapNCubeNeighbours [xyzToPosition (x, 0, z) ]++;
      m_mapNCubeNeighbours [xyzToPosition (x, sizeY - 1, z) ]++;
    }
}

VisibleCubeSet::~VisibleCubeSet() { }


void VisibleCubeSet::addCube (int x, int y, int z, int type) {
  checkCoordinates (x, y, z);
  int position = xyzToPosition (x, y, z);

  if (m_mapCubeTypes [position] == type)
    return;

  if (isEmpty (type))
    return removeCube (x, y, z);

  CubeTypeT oldType = m_mapCubeTypes [position];
  m_mapCubeTypes [position] = type;
  if (m_mapNCubeNeighbours [position] < 6)
    CubeArray::addCube (x, y, z, type);

  if (!isEmpty (oldType))
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

void VisibleCubeSet::removeCube (int x, int y, int z) {
  checkCoordinates (x, y, z);
  int position = xyzToPosition (x, y, z);
  if (isEmpty (m_mapCubeTypes [position]))
    return;

  m_mapCubeTypes [position] = emptyCube ();
  CubeArray::removeCube (x, y, z);

  int iMin, jMin, kMin, iMax, jMax, kMax;
  getNeighbourLimits (x, y, z, iMin, jMin, kMin, iMax, jMax, kMax);

  for (int i = iMin; i <= iMax; i += 2)
    removeNeighbour (i, y, z);
  for (int j = jMin; j <= jMax; j += 2)
    removeNeighbour (x, j, z);
  for (int k = kMin; k <= kMax; k += 2)
    removeNeighbour (x, y, k);
}


void VisibleCubeSet::getNeighbourLimits (int x, int y, int z, int& xMin, int& yMin, int& zMin, int& xMax, int& yMax, int& zMax) {
  xMin = x - 1;
  yMin = y - 1;
  zMin = z - 1;
  if (xMin < 0)  xMin += 2;
  if (yMin < 0)  yMin += 2;
  if (zMin < 0)  zMin += 2;

  xMax = x + 1;
  yMax = y + 1;
  zMax = z + 1;
  if (xMax > m_sizeX - 1)  xMax -= 2;
  if (yMax > m_sizeY - 1)  yMax -= 2;
  if (zMax > m_sizeZ - 1)  zMax -= 2;
}


void VisibleCubeSet::removeNeighbour (int x, int y, int z) {
  int position = xyzToPosition (x, y, z);
  if (m_mapNCubeNeighbours [position] == 6)
    CubeArray::addCube (x, y, z, m_mapCubeTypes [position]);
  assert (m_mapNCubeNeighbours [position] > 0);
  m_mapNCubeNeighbours [position]--;
}

void VisibleCubeSet::addNeighbour (int x, int y, int z) {
  int position = xyzToPosition (x, y, z);
  assert (m_mapNCubeNeighbours [position] < 6);
  m_mapNCubeNeighbours [position]++;
  if (m_mapNCubeNeighbours [position] == 6)
    CubeArray::removeCube (x, y, z);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ChunksForRender

ChunksForRender::ChunksForRender () {
  for (int i = 0; i < MAX_CHUNKS_FOR_RENDER; ++i)
    m_freeSlices.insert (i);
}

ChunksForRender::~ChunksForRender () {

}


// void ChunksForRender::lockCubes () {
//   GLfloat* bufferPos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_CUBES_INFORMATION_OFFSET,
//                                                      N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
//                                                      GL_MAP_WRITE_BIT);
//   GLfloat* bufferType = (GLfloat *) (bufferPos + 4 * N_MAX_BLOCKS_DRAWN);
//   cubeArray.setPointers (bufferPos, bufferType);
// }
//
// void ChunksForRender::unlockCubes () {
//   glUnmapBuffer (GL_ARRAY_BUFFER);
// }
