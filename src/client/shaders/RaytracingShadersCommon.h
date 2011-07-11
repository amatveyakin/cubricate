include(`../shaders_common.h')


#define EXIT_IF(condition__, r__, g__, b__)   if (bool (condition__)) { vFragColor = vec4 ((r__), (g__), (b__), 1.); return; }


const vec3  vec111 = vec3 (1., 1., 1.);
const vec3  vec123 = vec3 (1., 2., 3.);
const vec3  vec124 = vec3 (1., 2., 4.);

const float RENDER_WORLD_SIZE = MAP_SIZE / 2.;
const int   MAX_ITER_OUTER = 30;
const int   MAX_ITER_INNER = 10;

const int   CUBE_TYPE_AIR     = 0;
const int   CUBE_TYPE_WATER   = 1;

// const int   NODE_OFFSET_BY_HEIGHT[] = int[](
//   /* 0 */  0,
//   /* 1 */  1,
//   /* 2 */  9,
//   /* 3 */  73,
//   /* 4 */  585,
//   /* 5 */  4681,
//   /* 6 */  37449,
//   /* 7 */  299593,
//   /* 8 */  2396745,
//   /* 9 */  19173961
// );
//
// const int siblingShiftTable[] = int[](
// /* Z-  Y-  X-      X+  Y+  Z+ */
//    0,  0,  0,  0,  1,  2,  4,  /* 0 */
//    0,  0, -1,  0,  0,  2,  4,  /* 1 */
//    0, -2,  0,  0,  1,  0,  4,  /* 2 */
//    0, -2, -1,  0,  0,  0,  4,  /* 3 */
//   -4,  0,  0,  0,  1,  2,  0,  /* 4 */
//   -4,  0, -1,  0,  0,  2,  0,  /* 5 */
//   -4, -2,  0,  0,  1,  0,  0,  /* 6 */
//   -4, -2, -1,  0,  0,  0,  0   /* 7 */
// );

// const float refractionIndices[] = float[](1., 1., 1.333, 1, 1, 1, 1, 1);
// const float transparencyArray[] = float[](0.995, 0.95, 0, 0, 0, 0, 0);


int getNodeParent (int nodePointer) {
  return (nodePointer - 1) / 8;
}

int getNodeChild (int nodePointer, int iChild) {
  return nodePointer * 8 + 1 + iChild;
}

int getNodeType (int nodePointer) {
  return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_TYPE).r;
}

float getNodeSize (int nodePointer) {
  return RENDER_WORLD_SIZE / (1 << texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_HEIGHT).r);
}

vec3 getNodeMidpoint (vec3 pointInNode, float nodeSize) {
  return 2 * nodeSize * (floor (pointInNode / (2 * nodeSize)) + 0.5 * vec111);
}

int getNodeParameter (int nodePointer) {
  return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_PARAMETER).r;
}

int getNodeNeighbour (int nodePointer, vec3 direction) {
  int iChild = (nodePointer - 1) % 8;
  int directionIndex123 = int (round (dot (direction, vec123)));
  int directionIndex124 = int (round (dot (direction, vec124)));
//   int shift = texelFetch (siblingShiftTable, 7 * iChild + 3 + directionIndex123).r;
//   if (shift != 0)
//     return nodePointer + shift;
//   else
//     return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_NEIGHBOURS + abs (directionIndex123)).r;

  if (bool (  int ((iChild & abs (directionIndex124)) != 0)   // We can move is negative direction
            ^ int (directionIndex124 > 0)                  )) // Current direction is positive
    return nodePointer + (iChild ^ abs (directionIndex124)) - iChild;
  else
    return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_NEIGHBOURS + abs (directionIndex123)).r;
}

bool pointInCube (vec3 point, vec3 cubeMidpoint, float cubeSize) {
  return all (lessThan (abs(point - cubeMidpoint), vec3(cubeSize, cubeSize, cubeSize)));
}

vec3 colorToVector (vec3 color) {
  return 2 * color - vec111;
}

vec3 getNormalizedCubemapCoordinates (vec3 centralVector, float cubeSize, float texturingCoeff) {
  return fract (texturingCoeff * (centralVector + vec111 * cubeSize)) - vec111 / 2;
}