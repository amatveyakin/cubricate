#ifndef OCTREE_HPP
#define OCTREE_HPP


#include <cassert>

#include "common/c++0x_workaround.hpp"
#include "common/cube_geometry.hpp"


typedef int TreeDataT;

const int NODE_STRUCT_SIZE = 5;

struct TreeNodeT {
  TreeDataT& type ()              { return data [0]; }
  TreeDataT& neighbour (int i)    { assert (i >= 0);  assert (i < 3);  return data [i + 1]; }
  TreeDataT& height ()            { return data [4]; }

  TreeDataT data[NODE_STRUCT_SIZE];
};

const TreeDataT MIXED_TYPE = 0xff;


// TODO: use Vec3i for indexing
class Octree {
public:
  static const int N_NODE_CHILDREN = 8;

  Octree (int height);
  Octree (const Octree&) = delete;
  ~Octree ();

  void        setPointer (TreeDataT* newPointer);

  const TreeDataT*  nodes() const;
  int               height () const;
  int               size () const;
  int               nNodes () const;
  int               nLeaves () const;

  TreeDataT   get (int x, int y, int z) const;
  void        set (int x, int y, int z, TreeDataT type, bool updateNeighboursFlag);

  bool        hasChildren (int node) const;

  void        computeNeighbours ();

protected:
  TreeNodeT*  m_nodes;
  int         m_height;
  int         m_size;
  int         m_nNodes;
  int         m_nLeaves;

  void        checkCoordinates (int x, int y, int z) const;

  static int  getParent (int node);
  static int  getChild  (int node, int iChild);   // iChild = 0, 1, ..., 7

  void        stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize, int& iChild) const;
  void        stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize) const;

  int         getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize, int& iChild) const;
  int         getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize) const;
  int         getDeepestNode (/* i/o */ int& x, int& y, int& z) const;

  void        splitNode (int node);
  int         uniteNodesRecursively (int node);

  bool        tryToAddNeighbour (int node, int nodeSize, int iNeighbour, int neighbourX, int neighbourY, int neighbourZ);
//   void        doComputeNeighboursRecursively (int node, int x, int y, int z, int nodeSize);
};


#endif
