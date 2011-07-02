#version 140

uniform isamplerBuffer octTree;
uniform vec3 origin;

in  vec3 fDirection;

const vec3  vec111 = vec3 (1., 1., 1.);
const vec3  vec123 = vec3 (1., 2., 3.);
const vec3  vec124 = vec3 (1., 2., 4.);

const int   TREE_HEIGHT = 6;
const float CHUNK_SIZE = 128;
const int   MAX_ITER_OUTER = 100;
const int   MAX_ITER_INNER = 100;

const int   CUBE_TYPE_AIR     = 0;
const int   CUBE_TYPE_DIRT    = 1;
const int   CUBE_TYPE_WATER   = 2;

// Node structure: Type | NeigbourX | NeigbourY | NeigbourZ | Size |
const int   NODE_STRUCT_SIZE       = 5;
const int   NODE_OFFSET_TYPE       = 0;
const int   NODE_OFFSET_HEIGHT     = 4;
const int   NODE_OFFSET_NEIGHBOURS = 0;

const int   NODE_OFFSET_BY_HEIGHT[] = int[](
  /* 0 */  0,
  /* 1 */  1,
  /* 2 */  9,
  /* 3 */  73,
  /* 4 */  585,
  /* 5 */  4681,
  /* 6 */  37449,
  /* 7 */  299593,
  /* 8 */  2396745,
  /* 9 */  19173961
);

const int siblingShiftTable[] = int[](
/* Z-  Y-  X-      X+  Y+  Z+ */
   0,  0,  0,  0,  1,  2,  4,  /* 0 */
   0,  0, -1,  0,  0,  2,  4,  /* 1 */
   0, -2,  0,  0,  1,  0,  4,  /* 2 */
   0, -2, -1,  0,  0,  0,  4,  /* 3 */
  -4,  0,  0,  0,  1,  2,  0,  /* 4 */
  -4,  0, -1,  0,  0,  2,  0,  /* 5 */
  -4, -2,  0,  0,  1,  0,  0,  /* 6 */
  -4, -2, -1,  0,  0,  0,  0   /* 7 */
);

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
  return CHUNK_SIZE / (1 << texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_HEIGHT).r);
}

vec3 getNodeMidpoint (vec3 pointInNode, float nodeSize) {
  return 2 * nodeSize * (floor (pointInNode / (2 * nodeSize)) + 0.5 * vec111);
}

// nodePointer can be -3, -2, -1, 1, 2, 3
int getNodeNeighbour (int nodePointer, vec3 direction) {
  int iChild = (nodePointer - 1) % 8;
  int directionIndex123 = int (round (dot (direction, vec123)));
  int directionIndex124 = int (round (dot (direction, vec124)));
//   int shift = siblingShiftTable[7 * iChild + 3 + directionIndex];
//   if (shift != 0)
//     return nodePointer + shift;
//   else
//     return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_NEIGHBOURS + abs (directionIndex)).r;

  if (bool (  int ((iChild & abs (directionIndex124)) != 0)   // We can move is negative direction
            ^ int (directionIndex124 > 0)                  )) // Current direction is positive
    return nodePointer + (iChild ^ abs (directionIndex124)) - iChild;
  else
    return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_NEIGHBOURS + abs (directionIndex123)).r;
}

bool pointInCube (vec3 point, vec3 cubeMidpoint, float cubeSize) {
  return all (lessThan (abs(point - cubeMidpoint), vec3(cubeSize, cubeSize, cubeSize)));
}

void main(void)
{
  int   currCubePointer = 0;
  int   currCubeType;
  int   prevCubeType = 0;
  vec3  currCubeMidpoint;
  float currCubeSize;

  vec3  ray = normalize (fDirection);
  float currT, nextT, delta;
  vec3  deltaVector;
  vec3  currPoint, nextPoint;
  vec3  normal = ray;
  currT = 0;
  currPoint = origin;

  int iterOuter = 0;

  currCubePointer = 0;
  currCubeSize = CHUNK_SIZE;
  currCubeMidpoint = vec3 (0., 0., 0.);
  currCubeType = getNodeType (currCubePointer);

  while (iterOuter < MAX_ITER_OUTER && (currCubeType == CUBE_TYPE_AIR) && (pointInCube(currPoint, vec3(0, 0, 0), CHUNK_SIZE))) {

    int iterInner = 0;
    while (/*iterInner < MAX_ITER_INNER &&*/ currCubeType == 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currCubeMidpoint, currPoint);
      currCubeMidpoint += (2 * s - vec111) * currCubeSize;
      currCubePointer = getNodeChild (currCubePointer, int (dot (s, vec124)));
      currCubeType = getNodeType (currCubePointer);
      iterInner++;
    }

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    deltaVector = mix (deltaVector, 128 * vec111, isinf(deltaVector));
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);

    currPoint        += ray * (delta + 0.001);
    normal            = -trunc((currPoint - currCubeMidpoint) / currCubeSize);
    currCubePointer   = getNodeNeighbour (currCubePointer, -normal);
    currCubeSize      = getNodeSize (currCubePointer);
    currCubeMidpoint  = getNodeMidpoint (currPoint, currCubeSize);
    currCubeType      = getNodeType (currCubePointer);

    iterOuter++;
  }
  gl_FragData[0] = length(currPoint - origin) / (4 * CHUNK_SIZE);
}

