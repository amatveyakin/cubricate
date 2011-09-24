// TODO: delete
#include <iostream>



#include <cassert>

#include "common/utils.hpp"
#include "common/debug.hpp"

#include "client/octree.hpp"


// Returns the difference (myNumber - siblingNumber) for every direction, 0 if there is not a sibling there
const int siblingShiftTable[8][7] = {
  /* Z-  Y-  X-      X+  Y+  Z+ */
  {  0,  0,  0,  0,  1,  2,  4  },  /* 0 */
  {  0,  0, -1,  0,  0,  2,  4  },  /* 1 */
  {  0, -2,  0,  0,  1,  0,  4  },  /* 2 */
  {  0, -2, -1,  0,  0,  0,  4  },  /* 3 */
  { -4,  0,  0,  0,  1,  2,  0  },  /* 4 */
  { -4,  0, -1,  0,  0,  2,  0  },  /* 5 */
  { -4, -2,  0,  0,  1,  0,  0  },  /* 6 */
  { -4, -2, -1,  0,  0,  0,  0  }   /* 7 */
};

enum SiblingTableDirection {
  SIBLING_SHIFT_TABLE_X_PLUS  = 4,
  SIBLING_SHIFT_TABLE_Y_PLUS  = 5,
  SIBLING_SHIFT_TABLE_Z_PLUS  = 6,
  SIBLING_SHIFT_TABLE_X_MINUS = 2,
  SIBLING_SHIFT_TABLE_Y_MINUS = 1,
  SIBLING_SHIFT_TABLE_Z_MINUS = 0
};

// const int directionToSiblingShiftTable = {
//   4, /* X_PLUS  */
//   5, /* Y_PLUS  */
//   6, /* Z_PLUS  */
//   2, /* X_MINUS */
//   1, /* Y_MINUS */
//   0, /* Z_MINUS */
// };


Octree::Octree (int height, int heightOffset) {
  m_height = height;
  m_nNodes = 0;
  for (int i = 0; i <= height; ++i)
    m_nNodes += 1 << (i * 3);
  m_size = 1 << height;
  m_nLeaves = 1 << (height * 3);
  m_nodes = new TreeNodeT [m_nNodes];
  assert (m_nodes);
  m_nodes[0].type () = 0;
  int nodeBegin = 0;
  int nodeEnd   = 0;
  for (int i = 0; i <= height; ++i) {
    nodeEnd += 1 << (i * 3);
    for (int j = nodeBegin; j < nodeEnd; ++j)
      m_nodes[j].height () = i + heightOffset;
    nodeBegin = nodeEnd;
  }
  m_nodesArrayIsOriginal = true;
}

Octree::~Octree () {
  if (m_nodesArrayIsOriginal)
    delete[] m_nodes;
}


void Octree::setPointer (TreeDataT* newPointer) {
  if (m_nodesArrayIsOriginal) {
    delete[] m_nodes;
    m_nodesArrayIsOriginal = false;
  }
  m_nodes = reinterpret_cast<TreeNodeT*>(newPointer);
}


const TreeDataT* Octree::nodes () const {
  return reinterpret_cast<TreeDataT*>(m_nodes);
}

int Octree::height () const {
  return m_height;
}

int Octree::size () const {
  return m_size;
}

int Octree::nNodes () const {
  return m_nNodes;
}

int Octree::nLeaves () const {
  return m_nLeaves;
}



TreeDataT Octree::get (int x, int y, int z) const {
  checkCoordinates (x, y, z);
  return m_nodes [getDeepestNode (x, y, z)].type ();
}

void Octree::set (int x, int y, int z, WorldBlock block, bool updateNeighboursFlag) {
  TreeDataT treeTypeRecord = blockTypeToTreeDataT (block.type);
  checkCoordinates (x, y, z);
  int tmp;
  Vec3i nodeCorner;
  int nodeSize;
  int newX = x, newY = y, newZ = z;
  int curNode = getDeepestNode (newX, newY, newZ, nodeSize, tmp, XYZ_LIST (nodeCorner));
  int newNodeSize = nodeSize;
  int newNode = curNode;
  if  (m_nodes [curNode].type () != treeTypeRecord) {
    if  (newNodeSize > 1) {
      while  (newNodeSize > 1) {
        splitNode (newNode);
        stepDownOneLevel (newX, newY, newZ, newNode, newNodeSize);
      }
      m_nodes [newNode].type () = treeTypeRecord;
    }
    else {
      m_nodes [curNode].type () = treeTypeRecord;
      newNode = uniteNodesRecursively (curNode);
    }
  }

  if (updateNeighboursFlag && newNode != curNode) {
    Vec3i newNodeCorner;
    newX = x;
    newY = y;
    newZ = z;
    Box3i mapBoundingBox (Vec3i::replicated (0), Vec3i::replicated (MAP_SIZE));
    getDeepestNode (newX, newY, newZ, newNodeSize, tmp, XYZ_LIST (newNodeCorner));
    Box3i oldNodeBox (nodeCorner - Vec3i::replicated (1), nodeCorner + Vec3i::replicated (nodeSize + 1));
    Box3i newNodeBox (newNodeCorner - Vec3i::replicated (1), newNodeCorner + Vec3i::replicated (newNodeSize + 1));
    computeNeighboursLocal (intersectBox (enclosingBox (oldNodeBox, newNodeBox), mapBoundingBox));
  }

  // TODO: Fix: That's wrong! We unite node on the basis of types and than spread one block's parameters to the whole big cube.
  if (BlockInfo::isLiquid (block.type))
    m_nodes [newNode].parameter () = block.fluidSaturation * MAX_FLUID_SATURATION;
  else
    m_nodes [newNode].parameter () = block.parameters;
}


bool Octree::hasChildren (int node) const {
  return m_nodes [node].type () == MIXED_TYPE;
}


void Octree::computeNeighbours () {
  BEGIN_TIME_MEASUREMENT

  for (int i = 0; i < m_nNodes; ++i)
    for (int j = 0; j < 3; ++j)
      m_nodes[i].neighbour (j) = -1;

  doComputeNeighboursRecursively (0, -1, m_size, 0, 0, 0);

  END_TIME_MEASUREMENT (0, "computeNeighbours");
}

void Octree::computeNeighboursLocal (Box3i region) {
  BEGIN_TIME_MEASUREMENT

  Vec3i& corner1 = region.corner1;
  Vec3i& corner2 = region.corner2;

  for (int x = corner1.x(); x < corner2.x(); ++x) {
    for (int y = corner1.y(); y < corner2.y(); ++y) {
      for (int z = corner1.z(); z < corner2.z(); ++z) {
        int xTmp = x, yTmp = y, zTmp = z;
        int node = getDeepestNode (xTmp, yTmp, zTmp);

        for (int i = 0; i < 3; ++i)
          m_nodes[node].neighbour (i) = -1;
      }
    }
  }

  for (int x = corner1.x(); x < corner2.x(); ++x) {
    for (int y = corner1.y(); y < corner2.y(); ++y) {
      for (int z = corner1.z(); z < corner2.z(); ++z) {
        int nodeSize, iChild;
        int xTmp = x, yTmp = y, zTmp = z;
        int cornerX, cornerY, cornerZ;
        int node = getDeepestNode (xTmp, yTmp, zTmp, nodeSize, iChild, cornerX, cornerY, cornerZ);

        if (m_nodes[node].neighbour (0) == -1 && m_nodes[node].neighbour (1) == -1 && m_nodes[node].neighbour (2) == -1)
          doComputeNodeNeighbours (node, iChild, nodeSize, cornerX, cornerY, cornerZ);
      }
    }
  }

  END_TIME_MEASUREMENT (0, "computeNeighboursLocal");
}






TreeDataT Octree::blockTypeToTreeDataT (BlockType blockType) const {
  return BlockInfo::isSubobject (blockType) ? (-nNodes() /* TODO:  + subobject_offset */) : static_cast <TreeDataT> (blockType);
}

BlockType Octree::TreeDataTToBlockType (TreeDataT treeDataT) const {
  return (treeDataT < 0) ? BT_TEST_SUBOBJECT /* TODO: fix */ : static_cast <BlockType> (treeDataT);
}


void Octree::checkCoordinates (int x, int y, int z) const {
  FIX_UNUSED (x);
  FIX_UNUSED (y);
  FIX_UNUSED (z);
  assert (x >= 0);
  assert (y >= 0);
  assert (z >= 0);
  assert (x < m_size);
  assert (y < m_size);
  assert (z < m_size);
}

bool Octree::coordinatesValid (int x, int y, int z) const {
  return   (x >= 0) && (x < m_size)
        && (y >= 0) && (y < m_size)
        && (z >= 0) && (z < m_size);
}


int Octree::getParent (int node) {
  return (node - 1) / 8;
}

int Octree::getChild (int node, int iChild) {
  return node * 8 + 1 + iChild;
}


void Octree::stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize, int& iChild, int& cornerX, int& cornerY, int& cornerZ) const {
  nodeSize /= 2;
  iChild =    (z / nodeSize) * 4
            + (y / nodeSize) * 2
            + (x / nodeSize);
  assert (iChild < 8);
  z %= nodeSize;
  y %= nodeSize;
  x %= nodeSize;
  cornerX += ( iChild      % 2) * nodeSize;
  cornerY += ((iChild / 2) % 2) * nodeSize;
  cornerZ += ((iChild / 4) % 2) * nodeSize;
  node = getChild (node, iChild);
}

void Octree::stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize, int& iChild) const {
  nodeSize /= 2;
  iChild =    (z / nodeSize) * 4
            + (y / nodeSize) * 2
            + (x / nodeSize);
  assert (iChild < 8);
  z %= nodeSize;
  y %= nodeSize;
  x %= nodeSize;
  node = getChild (node, iChild);
}

void Octree::stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize) const {
  int tmp;
  stepDownOneLevel (x, y, z, node, nodeSize, tmp);
}


int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize, int& iChild, int& cornerX, int& cornerY, int& cornerZ) const {
  int curNode = 0;
  nodeSize = m_size;
  cornerX = 0;
  cornerY = 0;
  cornerZ = 0;
  while (hasChildren (curNode))
    stepDownOneLevel (x, y, z, curNode, nodeSize, iChild, cornerX, cornerY, cornerZ);
  assert (nodeSize > 0);
  return curNode;
}

int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize, int& iChild) const {
  int curNode = 0;
  nodeSize = m_size;
  while (hasChildren (curNode))
    stepDownOneLevel (x, y, z, curNode, nodeSize, iChild);
  assert (nodeSize > 0);
  return curNode;
}

int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize) const {
  int tmp;
  return getDeepestNode (x, y, z, nodeSize, tmp);
}

int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z) const {
  int tmp;
  return getDeepestNode (x, y, z, tmp);
}


void Octree::splitNode (int node) {
  for (int i = 0; i < N_NODE_CHILDREN; ++i)
    m_nodes [getChild (node, i)].type () = m_nodes [node].type ();
  m_nodes [node].type () = MIXED_TYPE;
}

int Octree::uniteNodesRecursively (int node) {
  if (!blockShouldBeUnited (TreeDataTToBlockType (m_nodes [node].type ())))
    return node;
  while  (node > 0) {
    int parent = getParent (node);
    TreeDataT type = m_nodes [node].type ();
    if  (   type != MIXED_TYPE
         && m_nodes [getChild (parent, 0)].type () == type && m_nodes [getChild (parent, 1)].type () == type
         && m_nodes [getChild (parent, 2)].type () == type && m_nodes [getChild (parent, 3)].type () == type
         && m_nodes [getChild (parent, 4)].type () == type && m_nodes [getChild (parent, 5)].type () == type
         && m_nodes [getChild (parent, 6)].type () == type && m_nodes [getChild (parent, 7)].type () == type) {
      m_nodes [parent].type () = type;
      node = parent;
    }
    else
      return node;
  }
  return node;
}


void Octree::tryToAddNeighbour (int node, int nodeSize, int iNeighbour, int neighbourX, int neighbourY, int neighbourZ) {
  if (!coordinatesValid (neighbourX, neighbourY, neighbourZ))
    return;
  int neighbourSize;
  int neighbour = getDeepestNode (neighbourX, neighbourY, neighbourZ, neighbourSize);
  assert (neighbourSize > 0);
  while (neighbourSize < nodeSize) {
    neighbour = getParent (neighbour);
    neighbourSize *= 2;
  }
  m_nodes[node].neighbour (iNeighbour) = neighbour;
}

void Octree::doComputeNeighboursRecursively (int node, int indexInParent, int nodeSize, int cornerX, int cornerY, int cornerZ) {
  if (m_nodes [node].type() == MIXED_TYPE) {
    for (int i = 0; i < N_NODE_CHILDREN; ++i) {
      int childSize = nodeSize / 2;
      doComputeNeighboursRecursively (getChild (node, i), i, childSize,
                                      cornerX + ( i      % 2) * childSize,
                                      cornerY + ((i / 2) % 2) * childSize,
                                      cornerZ + ((i / 4) % 2) * childSize);
    }
  }
  else if (indexInParent >= 0)
    doComputeNodeNeighbours (node, indexInParent, nodeSize, cornerX, cornerY, cornerZ);
}

void Octree::doComputeNodeNeighbours (int node, int indexInParent, int nodeSize, int cornerX, int cornerY, int cornerZ) {
  if (siblingShiftTable [indexInParent][SIBLING_SHIFT_TABLE_X_MINUS] == 0)
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_X, cornerX - 1, cornerY, cornerZ);
  else
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_X, cornerX + nodeSize, cornerY, cornerZ);

  if (siblingShiftTable [indexInParent][SIBLING_SHIFT_TABLE_Y_MINUS] == 0)
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_Y, cornerX, cornerY - 1, cornerZ);
  else
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_Y, cornerX, cornerY + nodeSize, cornerZ);

  if (siblingShiftTable [indexInParent][SIBLING_SHIFT_TABLE_Z_MINUS] == 0)
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_Z, cornerX, cornerY, cornerZ - 1);
  else
    tryToAddNeighbour (node, nodeSize, TreeNodeT::NEIGHBOUR_Z, cornerX, cornerY, cornerZ + nodeSize);
}



bool Octree::blockShouldBeUnited (BlockType type) {
  return BlockInfo::isFluid (type);
}
