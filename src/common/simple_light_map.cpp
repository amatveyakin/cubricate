#include "simple_light_map.hpp"


SimpleLightMap::SimpleLightMap (int sizeX, int sizeY, int sizeZ) :
  m_luminosity (sizeX, sizeY, sizeZ)
{
}

SimpleLightMap::~SimpleLightMap() { }
