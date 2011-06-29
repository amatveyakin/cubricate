// TODO: delete
// #include <iostream>



#include <cassert>

#include "client/octree.hpp"


Octree::Octree (int height) {
  m_height = height;
  m_nNodes = 0;
  for (int i = 0; i <= height; ++i)
    m_nNodes += 1 << (i * 3);
  m_size = 1 << height;
  m_nLeaves = 1 << (height * 3);
  m_nodes = new TreeNodeT [m_nNodes];
  m_nodesOriginal = m_nodes;
  m_nodes[0] = 0;
}

Octree::~Octree () {
  delete[] m_nodes;
}


void Octree::setPointer (TreeNodeT* newPointer) {
  m_nodes = newPointer;
}

void Octree::restorePointer () {
  m_nodes = m_nodesOriginal;
}


const TreeNodeT* Octree::nodes() const {
  return m_nodes;
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



TreeNodeT Octree::get (int x, int y, int z) const {
  checkCoordinates (x, y, z);
  return m_nodes [getDeepestNode (x, y, z)];
}

void Octree::set (int x, int y, int z, TreeNodeT type) {
  checkCoordinates (x, y, z);
//   std::cout << "set (" << x << ", " << y << ", " << z << ")" << std::endl;
  int nodeSize;
  int curNode = getDeepestNode (x, y, z, nodeSize);
  if  (m_nodes [curNode] != type) {
    if  (nodeSize > 1) {
      while  (nodeSize > 1) {
        splitNode (curNode);
        stepDownOneLevel (x, y, z, curNode, nodeSize);
      }
      m_nodes [curNode] = type;
    }
    else {
      m_nodes [curNode] = type;
      uniteNodesRecursively (curNode);
    }
  }
}


bool Octree::hasChildren (int node) const {
  return m_nodes [node] == MIXED_TYPE;
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


void Octree::stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize) const {
  nodeSize /= 2;
//   std::cout << "x = " << x << ", y = " << y << ", z = " << z << ", nodeSize = " << nodeSize << std::endl;
  int iChild =   (z / nodeSize) * 4
               + (y / nodeSize) * 2
               + (x / nodeSize);
//   std::cout << "iChild = " << iChild << std::endl;
  assert (iChild < 8);
  z %= nodeSize;
  y %= nodeSize;
  x %= nodeSize;
  node = getChild (node, iChild);
}

int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize) const {
  int curNode = 0;
  nodeSize = m_size;
  while  (hasChildren (curNode))
    stepDownOneLevel (x, y, z, curNode, nodeSize);
  assert (nodeSize > 0);
  return curNode;
}

int Octree::getDeepestNode (/* i/o */ int& x, int& y, int& z) const {
  int tmp;
  return getDeepestNode (x, y, z, tmp);
}


void Octree::splitNode (int node) {
//   std::cout << "splitNode (" << node << ")" << std::endl;
  for (int i = 0; i < N_NODE_CHILDREN; ++i)
    m_nodes [getChild (node, i)] = m_nodes [node];
  m_nodes [node] = MIXED_TYPE;
}

int Octree::uniteNodesRecursively (int node) {
  while  (node > 0) {
    int parent = getParent (node);
    TreeNodeT type = m_nodes [node];
    if  (   type != MIXED_TYPE
         && m_nodes [getChild (parent, 0)] == type && m_nodes [getChild (parent, 1)] == type && m_nodes [getChild (parent, 2)] == type && m_nodes [getChild (parent, 3)] == type
         && m_nodes [getChild (parent, 4)] == type && m_nodes [getChild (parent, 5)] == type && m_nodes [getChild (parent, 6)] == type && m_nodes [getChild (parent, 7)] == type) {
      m_nodes [parent] = type;
      node = parent;
    }
    else
      return node;
  }
  return node;
}

