#include "simple_light_map.hpp"


SimpleLightMap::SimpleLightMap (int sizeX, int sizeY, int sizeZ) : Array3D <Vec4uc> (sizeX, sizeY, sizeZ) { }

SimpleLightMap::~SimpleLightMap() { }
