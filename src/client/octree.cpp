// TODO: delete
#include <iostream>



#include <cassert>

#include "client/octree.hpp"


// Returns the difference (myNumber - siblingNumber) for every direction, 0 if there is not sibling there
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

const int SIBLING_SHIFT_TABLE_X_PLUS  = 4;
const int SIBLING_SHIFT_TABLE_Y_PLUS  = 5;
const int SIBLING_SHIFT_TABLE_Z_PLUS  = 6;
const int SIBLING_SHIFT_TABLE_X_MINUS = 2;
const int SIBLING_SHIFT_TABLE_Y_MINUS = 1;
const int SIBLING_SHIFT_TABLE_Z_MINUS = 0;

// const int directionToSiblingShiftTable = {
//   4, /* X_PLUS  */
//   5, /* Y_PLUS  */
//   6, /* Z_PLUS  */
//   2, /* X_MINUS */
//   1, /* Y_MINUS */
//   0, /* Z_MINUS */
// };


Octree::Octree (int height) {
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
      m_nodes[j].height () = i;
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


const TreeDataT* Octree::nodes() const {
  return reinterpret_cast<TreeDataT*>(m_nodes);
}

int Octree::height() const {
  return m_height;
}

int Octree::size() const {
  return m_size;
}

int Octree::nNodes() const {
  return m_nNodes;
}

int Octree::nLeaves() const {
  return m_nLeaves;
}



TreeDataT Octree::get (int x, int y, int z) const {
  checkCoordinates (x, y, z);
  return m_nodes [getDeepestNode (x, y, z)].type ();
}

// TODO: speed up local neighbour update
void Octree::set (int x, int y, int z, TreeDataT type, bool updateNeighboursFlag) {
  checkCoordinates (x, y, z);
//   std::cout << "set (" << x << ", " << y << ", " << z << ")" << std::endl;
  int nodeSize;
  int curNode = getDeepestNode (x, y, z, nodeSize);
  if  (m_nodes [curNode].type () != type) {
    if  (nodeSize > 1) {
      while  (nodeSize > 1) {
        splitNode (curNode);
        stepDownOneLevel (x, y, z, curNode, nodeSize);
      }
      m_nodes [curNode].type () = type;
      if (updateNeighboursFlag)
        computeNeighbours ();
    }
    else {
      m_nodes [curNode].type () = type;
      int newNode = uniteNodesRecursively (curNode);
      if (updateNeighboursFlag && newNode != curNode)
        computeNeighbours ();
    }
  }
}


bool Octree::hasChildren (int node) const {
  return m_nodes [node].type () == MIXED_TYPE;
}


void Octree::computeNeighbours () {
//   doComputeNeighboursRecursively (0);

  for (int i = 0; i < m_nNodes; ++i)
    for (int j = 0; j < 3; ++j)
      m_nodes[i].neighbour (j) = -1;

  for (int x = 0; x < m_size; ++x) {
    for (int y = 0; y < m_size; ++y) {
      for (int z = 0; z < m_size; ++z) {
        int nodeSize, iChild;
        int xTmp = x, yTmp = y, zTmp = z;
        int node = getDeepestNode (xTmp, yTmp, zTmp, nodeSize, iChild);

//         std::cout << "(" << x << ", " << y << ", " << z << "): node = " << node << ", nodeSize = " << nodeSize << std::endl;

        if (m_nodes[node].neighbour (0) >= 0 || m_nodes[node].neighbour (1) >= 0 || m_nodes[node].neighbour (2) >= 0)
          continue;

        if (siblingShiftTable [iChild][SIBLING_SHIFT_TABLE_X_PLUS] == 0) {
          for (int neighbourX = x + 1; neighbourX < m_size; ++neighbourX)
            if (tryToAddNeighbour (node, nodeSize, 0, neighbourX, y, z))
              break;
        }
        else {
          for (int neighbourX = x - 1; neighbourX >= 0; --neighbourX)
            if (tryToAddNeighbour (node, nodeSize, 0, neighbourX, y, z))
              break;
        }

        if (siblingShiftTable [iChild][SIBLING_SHIFT_TABLE_Y_PLUS] == 0) {
          for (int neighbourY = y + 1; neighbourY < m_size; ++neighbourY)
            if (tryToAddNeighbour (node, nodeSize, 1, x, neighbourY, z))
              break;
        }
        else {
          for (int neighbourY = y - 1; neighbourY >= 0; --neighbourY)
            if (tryToAddNeighbour (node, nodeSize, 1, x, neighbourY, z))
              break;
        }

        if (siblingShiftTable [iChild][SIBLING_SHIFT_TABLE_Z_PLUS] == 0) {
          for (int neighbourZ = z + 1; neighbourZ < m_size; ++neighbourZ)
            if (tryToAddNeighbour (node, nodeSize, 2, x, y, neighbourZ))
              break;
        }
        else {
          for (int neighbourZ = z - 1; neighbourZ >= 0; --neighbourZ)
            if (tryToAddNeighbour (node, nodeSize, 2, x, y, neighbourZ))
              break;
        }

//         for (int neighbourZ = z - 1; neighbourZ >= 0; --neighbourZ)
//           if (tryToAddNeighbour (node, nodeSize, 0, x, y, neighbourZ))
//             break;
//         for (int neighbourY = y - 1; neighbourY >= 0; --neighbourY)
//           if (tryToAddNeighbour (node, nodeSize, 1, x, neighbourY, z))
//             break;
//         for (int neighbourX = x - 1; neighbourX >= 0; --neighbourX)
//           if (tryToAddNeighbour (node, nodeSize, 2, neighbourX, y, z))
//             break;
//
//         for (int neighbourX = x + 1; neighbourX < m_size; ++neighbourX)
//           if (tryToAddNeighbour (node, nodeSize, 3, neighbourX, y, z))
//             break;
//         for (int neighbourY = y + 1; neighbourY < m_size; ++neighbourY)
//           if (tryToAddNeighbour (node, nodeSize, 4, x, neighbourY, z))
//             break;
//         for (int neighbourZ = z + 1; neighbourZ < m_size; ++neighbourZ)
//           if (tryToAddNeighbour (node, nodeSize, 5, x, y, neighbourZ))
//             break;
      }
    }
  }
}



void Octree::checkCoordinates (int x, int y, int z) const {
  assert (x >= 0);
  assert (y >= 0);
  assert (z >= 0);
  assert (x < m_size);
  assert (y < m_size);
  assert (z < m_size);
}


int Octree::getParent (int node) {
  return (node - 1) / 8;
}

int Octree::getChild (int node, int iChild) {
  assert (iChild >= 0);
  assert (iChild < 8);
  return node * 8 + 1 + iChild;
}


void Octree::stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize, int& iChild) const {
  nodeSize /= 2;
//   std::cout << "x = " << x << ", y = " << y << ", z = " << z << ", nodeSize = " << nodeSize << std::endl;
  iChild =   (z / nodeSize) * 4
            + (y / nodeSize) * 2
            + (x / nodeSize);
//   std::cout << "iChild = " << iChild << std::endl;
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
//   std::cout << "splitNode (" << node << ")" << std::endl;
  for (int i = 0; i < N_NODE_CHILDREN; ++i)
    m_nodes [getChild (node, i)].type () = m_nodes [node].type ();
  m_nodes [node].type () = MIXED_TYPE;
}

int Octree::uniteNodesRecursively (int node) {
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


bool Octree::tryToAddNeighbour (int node, int nodeSize, int iNeighbour, int neighbourX, int neighbourY, int neighbourZ) {
  int neighbourSize;
  int neighbour = getDeepestNode (neighbourX, neighbourY, neighbourZ, neighbourSize);
  assert (neighbourSize > 0);
  if (neighbour != node) {
    while (neighbourSize < nodeSize) {
      neighbour = getParent (neighbour);
      neighbourSize *= 2;
    }
    m_nodes[node].neighbour (iNeighbour) = neighbour;
    return true;
  }
  return false;
}

// void Octree::doComputeNeighboursRecursively (int node, int x, int y, int z, int nodeSize) {
//   if (m_nodes [node] == MIXED_TYPE) {
//     for (int i = 0; i < N_NODE_CHILDREN; ++i)
//       doComputeNeighboursRecursively (getChild (node, i));
//   }
//   else {
//
//   }
// }
