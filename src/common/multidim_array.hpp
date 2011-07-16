#ifndef MULTIDIM_ARRAY_HPP
#define MULTIDIM_ARRAY_HPP


#include "common/linear_algebra.hpp"


template <typename ElementT>
class Array3D {
public:
  Array3D (int sizeX, int sizeY, int sizeZ) {
    m_sizeX = sizeX;
    m_sizeY = sizeY;
    m_sizeZ = sizeZ;
    m_elements = new ElementT [totalElements ()];
  }
  Array3D (const Array3D& other) = delete;   // Do you really need it? Ok, do implement WUT IT IS? HOW ITS WORKS? O_o
  ~Array3D () {
    delete[] m_elements;
  }

  const ElementT* data () const                             { return m_elements; }
  ElementT* data ()                                         { return m_elements; }

  int sizeX () const                                        { return m_sizeX; }
  int sizeY () const                                        { return m_sizeY; }
  int sizeZ () const                                        { return m_sizeZ; }
  int totalElements () const                                { return m_sizeX * m_sizeY * m_sizeZ; }

  const ElementT& at (int x, int y, int z) const            { return m_elements [x * m_sizeY * m_sizeZ  +  y * m_sizeZ  +  z]; }
  ElementT& at (int x, int y, int z)                        { return m_elements [x * m_sizeY * m_sizeZ  +  y * m_sizeZ  +  z]; }
  const ElementT& at (Vec3i pos) const                      { return at (pos.x (), pos.y (), pos.z ()); }
  ElementT& at (Vec3i pos)                                  { return at (pos.x (), pos.y (), pos.z ()); }
  const ElementT& operator() (int x, int y, int z) const    { return at (x, y, z); }
  ElementT& operator() (int x, int y, int z)                { return at (x, y, z); }
  const ElementT& operator() (Vec3i pos) const              { return at (pos); }
  ElementT& operator() (Vec3i pos)                          { return at (pos); }

private:
  int m_sizeX;
  int m_sizeY;
  int m_sizeZ;
  ElementT* m_elements;
};


#endif
