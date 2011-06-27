#include "common/cube_geometry.hpp"

#include "client/visible_cube_set.hpp"


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>



const int MAX_RENDER_CHUNKS = PLAYER_SIGHT_RADIUS * PLAYER_SIGHT_RADIUS * MAP_HEIGHT_IN_CHUNKS;  // TODO: estimate more precisely



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


void CubeArray::addCube (int x, int y, int z, CubeTypeT type) {
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


int VisibleCubeSet::addCube (int x, int y, int z, CubeTypeT type) {
  checkCoordinates (x, y, z);
  int position = xyzToPosition (x, y, z);

  if (m_mapCubeTypes [position] == type)
    return 0;

  if (isEmpty (type))
    return removeCube (x, y, z);

  CubeTypeT oldType = m_mapCubeTypes [position];
  m_mapCubeTypes [position] = type;
  if (m_mapNCubeNeighbours [position] < 6)
    CubeArray::addCube (x, y, z, type);

  if (!isEmpty (oldType))
    return 0;

  int iMin, jMin, kMin, iMax, jMax, kMax;
  getNeighbourLimits (x, y, z, iMin, jMin, kMin, iMax, jMax, kMax);

  for (int i = iMin; i <= iMax; i += 2)
    addNeighbour (i, y, z);
  for (int j = jMin; j <= jMax; j += 2)
    addNeighbour (x, j, z);
  for (int k = kMin; k <= kMax; k += 2)
    addNeighbour (x, y, k);

  return 1;
}

int VisibleCubeSet::removeCube (int x, int y, int z) {
  checkCoordinates (x, y, z);
  int position = xyzToPosition (x, y, z);
  if (isEmpty (m_mapCubeTypes [position]))
    return 0;

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

  return -1;
}


void VisibleCubeSet::changeNExternalNeighbours (int x, int y, int z, CubeTypeT type, int neighboursDelta) {
  while (neighboursDelta > 0) {
    addNeighbour (x, y, z);
    neighboursDelta--;
  }
  while (neighboursDelta < 0) {
    removeNeighbour (x, y, z);
    neighboursDelta++;
  }
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

// static inline void cubeToRenderChunk (/* in */ Vec3i cube, /* out */ Vec3i& chunk, Vec3i& cubeInChunk) {
//   chunk       = cube.divFloored (CHUNK_SIZE);
//   cubeInChunk = cube.modFloored (CHUNK_SIZE);
// }

// TODO: What's this?
static const int N_MAX_BLOCKS_DRAWN = N_MAP_BLOCKS;



ChunksForRender::ChunksForRender () :
  m_cubesInformationOffset (-1),
  m_renderChunksCubeSets (MAX_RENDER_CHUNKS, VisibleCubeSet (CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)),
  m_renderChunksStates  (MAX_RENDER_CHUNKS, ChunkState::REDUNDANT)
{
  for (int i = 0; i < MAX_RENDER_CHUNKS; ++i)
    m_freeRenderChunks.insert (i);
}

ChunksForRender::~ChunksForRender () { }


void ChunksForRender::init (int cubesInformationOffset) {
  m_cubesInformationOffset = cubesInformationOffset;
}


void ChunksForRender::addRenderChunk (Vec3i renderChunk, ChunkState state, const Array3D <WorldBlock>& worldChunkCubes) {
  assert (state == ChunkState::ACTIVE || state == ChunkState::PRESERVED);
  assert (!m_freeRenderChunks.empty ());
  int iRenderChunk = *m_freeRenderChunks.begin ();
  m_freeRenderChunks.erase (m_freeRenderChunks.begin ());
  m_renderChunksPosToIndex.insert (std::make_pair (renderChunk, iRenderChunk));
  m_renderChunksStates [iRenderChunk] = state;
  lockRenderChunk (iRenderChunk);
//   m_renderChunksCubeSets [iRenderChunk].clear ();
  for (int x = 0; x < CHUNK_SIZE; ++x)
    for (int y = 0; y < CHUNK_SIZE; ++y)
      for (int z = 0; z < CHUNK_SIZE; ++z)
        setCube (renderChunk, Vec3i (x, y, z), worldChunkCubes (x, y, z % CHUNK_SIZE).type);
  unlockRenderChunks ();
}

void ChunksForRender::removeRenderChunk (Vec3i renderChunk) {
  int iRenderChunk = getRenderChunkIndex (renderChunk);
  m_renderChunksPosToIndex.erase (renderChunk);
  m_renderChunksStates [iRenderChunk] = ChunkState::REDUNDANT;
  m_freeRenderChunks.insert (iRenderChunk);
  // TODO: ...
}

void ChunksForRender::setRenderChunkState (Vec3i renderChunk, ChunkState newState) {
  int iRenderChunk = getRenderChunkIndex (renderChunk);
  ChunkState oldState = m_renderChunksStates [iRenderChunk];
  // TODO: ...
}


void ChunksForRender::setCube (Vec3i renderChunk, Vec3i cubeInChunk, BlockType type) {
  int nCubeDelta = setRenderChunkCube (renderChunk, cubeInChunk.x (), cubeInChunk.y (), cubeInChunk.z (), type);
  if (nCubeDelta != 0) {
    if (cubeInChunk.x () == 0)
      chanegNExternalNeighboursInRenderChunk (renderChunk - Vec3i::e1 (), CHUNK_SIZE - 1, cubeInChunk.y (), cubeInChunk.z (), type, nCubeDelta);
    if (cubeInChunk.y () == 0)
      chanegNExternalNeighboursInRenderChunk (renderChunk - Vec3i::e2 (), cubeInChunk.x (), CHUNK_SIZE - 1, cubeInChunk.z (), type, nCubeDelta);
    if (cubeInChunk.z () == 0)
      chanegNExternalNeighboursInRenderChunk (renderChunk - Vec3i::e3 (), cubeInChunk.x (), cubeInChunk.y (), CHUNK_SIZE - 1, type, nCubeDelta);
    if (cubeInChunk.x () == CHUNK_SIZE - 1)
      chanegNExternalNeighboursInRenderChunk (renderChunk + Vec3i::e1 (), 0, cubeInChunk.y (), cubeInChunk.z (), type, nCubeDelta);
    if (cubeInChunk.y () == CHUNK_SIZE - 1)
      chanegNExternalNeighboursInRenderChunk (renderChunk + Vec3i::e2 (), cubeInChunk.x (), 0, cubeInChunk.z (), type, nCubeDelta);
    if (cubeInChunk.z () == CHUNK_SIZE - 1)
      chanegNExternalNeighboursInRenderChunk (renderChunk + Vec3i::e3 (), cubeInChunk.x (), cubeInChunk.y (), 0, type, nCubeDelta);
  }
}


int ChunksForRender::getRenderChunkIndex (Vec3i renderChunkPos) {
  auto it = m_renderChunksPosToIndex.find (renderChunkPos);
  assert (it != m_renderChunksPosToIndex.end ());
  return it->second;
}


void ChunksForRender::lockRenderChunk (int iRenderChunk) {
  assert (m_cubesInformationOffset >= 0);  // TODO: delete
  GLfloat* bufferPos = (GLfloat *) glMapBufferRange (GL_ARRAY_BUFFER, m_cubesInformationOffset,
                                                     N_MAX_BLOCKS_DRAWN * (4 * sizeof (GLfloat) + sizeof (GLfloat)),
                                                     GL_MAP_WRITE_BIT);
  GLfloat* bufferType = (GLfloat *) (bufferPos + 4 * N_MAX_BLOCKS_DRAWN);
  m_renderChunksCubeSets [iRenderChunk].setPointers (bufferPos, bufferType);
}

void ChunksForRender::unlockRenderChunks () {
  glUnmapBuffer (GL_ARRAY_BUFFER);
}


// Returns cube number delta (-1, 0 or 1)
int ChunksForRender::setRenderChunkCube (Vec3i renderChunk, int x, int y, int z, BlockType type) {
  int iRenderChunk = getRenderChunkIndex (renderChunk);
  lockRenderChunk (iRenderChunk);
  int result = m_renderChunksCubeSets [iRenderChunk].addCube (x, y, z, static_cast <CubeTypeT> (type));
  unlockRenderChunks ();
  return result;
}


void ChunksForRender::chanegNExternalNeighboursInRenderChunk (Vec3i renderChunk, int x, int y, int z, BlockType type, int neighboursDelta) {
  int iRenderChunk = getRenderChunkIndex (renderChunk);
  lockRenderChunk (iRenderChunk);
  m_renderChunksCubeSets [iRenderChunk].changeNExternalNeighbours (x, y, z, static_cast <CubeTypeT> (type), neighboursDelta);
  unlockRenderChunks ();
}
