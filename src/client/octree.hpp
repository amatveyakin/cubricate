#ifndef OCTREE_HPP
#define OCTREE_HPP


#include "common/c++0x_workaround.hpp"
#include "common/cube_geometry.hpp"
// #include "common/world_block.hpp"


typedef int TreeNodeT;
const TreeNodeT MIXED_TYPE = 0xff;


// TODO: use Vec3i for indexing
class Octree {
public:
  static const int N_NODE_CHILDREN = 8;

  Octree (int height);
  Octree (const Octree&) = delete;
  ~Octree ();

  void        setPointer (TreeNodeT* newPointer);
  void        restorePointer ();

  const TreeNodeT*  nodes() const;
  int               height () const;
  int               size () const;
  int               nNodes () const;
  int               nLeaves () const;

  TreeNodeT   get (int x, int y, int z) const;
  void        set (int x, int y, int z, TreeNodeT type);

  bool        hasChildren (int node) const;

protected:
  TreeNodeT*  m_nodes;
  TreeNodeT*  m_nodesOriginal;
  int         m_height;
  int         m_size;
  int         m_nNodes;
  int         m_nLeaves;

  void        checkCoordinates (int x, int y, int z) const;

  static int  getParent (int node);
  static int  getChild  (int node, int iChild);   // iChild = 0, 1, ..., 7

  void        stepDownOneLevel (/* i/o */ int& x, int& y, int& z, int& node, int& nodeSize) const;
  int         getDeepestNode (/* i/o */ int& x, int& y, int& z, /* out */ int& nodeSize) const;
  int         getDeepestNode (/* i/o */ int& x, int& y, int& z) const;

  void        splitNode (int node);
  int         uniteNodesRecursively (int node);
};


#endif
