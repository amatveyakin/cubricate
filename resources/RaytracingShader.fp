#version 140
#extension GL_ARB_texture_cube_map_array : enable


#define EXIT_IF(condition__, r__, g__, b__)   if (bool (condition__)) { vFragColor = vec4 ((r__), (g__), (b__), 1.); return; }

struct CubeProperties {
  float transparency;
  float refractionIndex;
};

uniform isamplerBuffer   octTree;
uniform  samplerBuffer   cubeProperties;
uniform isamplerBuffer   siblingShiftTable;
uniform sampler2D        depthTexture;
uniform samplerCubeArray cubeTexture;

uniform vec3             origin;

in  vec3    fDirection;
in  vec2    fPosition;

out vec4    vFragColor;

const vec3  vec111 = vec3 (1., 1., 1.);
const vec3  vec123 = vec3 (1., 2., 3.);
const vec3  vec124 = vec3 (1., 2., 4.);

const int   TREE_HEIGHT = 6;
const float CHUNK_SIZE = (1 << TREE_HEIGHT) / 2.;
const int   MAX_ITER_OUTER = 100;
const int   MAX_ITER_INNER = 100;

const int   CUBE_TYPE_AIR     = 0;
const int   CUBE_TYPE_DIRT    = 1;
const int   CUBE_TYPE_WATER   = 2;

// Node structure: Type | NeigbourX | NeigbourY | NeigbourZ | Size
const int   NODE_STRUCT_SIZE       = 5;
const int   NODE_OFFSET_TYPE       = 0;
const int   NODE_OFFSET_HEIGHT     = 4;
const int   NODE_OFFSET_NEIGHBOURS = 0;

const int     RAY_PACKET_WIDTH  = 4;
const int     RAY_PACKET_HEIGHT = 4;

const int     SCREEN_WIDTH  = 1024;
const int     SCREEN_HEIGHT = 1024;


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
  return CHUNK_SIZE / (1 << texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_HEIGHT).r);
}

vec3 getNodeMidpoint (vec3 pointInNode, float nodeSize) {
  return 2 * nodeSize * (floor (pointInNode / (2 * nodeSize)) + 0.5 * vec111);
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

CubeProperties getCubeProperties (int cubeType) {
  CubeProperties result;
  vec4 fetch = texelFetch (cubeProperties, cubeType);
  result.transparency    = fetch.r;
  result.refractionIndex = fetch.g;
  return result;
}



bool pointInCube (vec3 point, vec3 cubeMidpoint, float cubeSize) {
  return all (lessThan (abs(point - cubeMidpoint), vec3(cubeSize, cubeSize, cubeSize)));
}

void main(void)
{
  int   currCubePointer = 0;
  int   currCubeType;
  int   prevCubeType = 0;
  CubeProperties currCubeProperties, prevCubeProperties;
  vec3  currCubeMidpoint;
  float currCubeSize;

  vec3  ray = normalize (fDirection);
  float currT, nextT, delta;
  vec3  deltaVector;
  vec3  currPoint, nextPoint;
  vec3  normal = ray;

  //vec2 samplingPosition = floor (fPosition * SCREEN_WIDTH / RAY_PACKET_WIDTH) * RAY_PACKET_WIDTH  * (1. / SCREEN_WIDTH);
  vec2 samplingPosition = fPosition;
  float samplingShift = 0.5 * RAY_PACKET_WIDTH / SCREEN_WIDTH;
   float minDepth = min (min (texture (depthTexture, samplingPosition + vec2(samplingShift,  samplingShift)).g, texture (depthTexture, samplingPosition + vec2(-samplingShift,  samplingShift)).g),
                         min (texture (depthTexture, samplingPosition + vec2(samplingShift, -samplingShift)).g, texture (depthTexture, samplingPosition + vec2(-samplingShift, -samplingShift)).g));
  //float minDepth = texture (depthTexture, samplingPosition).g;
  //minDepth = max( minDepth, 0);
  currT = minDepth * 4 * CHUNK_SIZE - 3;
  currT = max (currT, 0);
  //currT = 0;
  currPoint = origin + ray * currT;

  vFragColor = vec4 (0., 0., 0., 1.); // fourth component is transparency, not opacity!
  //int iterOuter = int (texture (cubeTexture, vec3(1, 0.5, -0.5)).r);
  int iterOuter = 0;
  currCubePointer = 0;
  currCubeSize = CHUNK_SIZE;
  currCubeMidpoint = vec3 (0., 0., 0.);
  currCubeType = getNodeType (currCubePointer);
  //prevCubeType = currCubeType;
  while (iterOuter < MAX_ITER_OUTER && (vFragColor.w > 0.05) && (pointInCube(currPoint, vec3(0, 0, 0), CHUNK_SIZE))) {
//     EXIT_IF (currCubePointer < 0,  1., 1., 0.);
//     EXIT_IF (currCubePointer > 8,  0., 1., 1.);

    int iterInner = 0;
    while (/*iterInner < MAX_ITER_INNER &&*/ currCubeType == 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currCubeMidpoint, currPoint);
      currCubeMidpoint += (2 * s - vec111) * currCubeSize;
      currCubePointer = getNodeChild (currCubePointer, int (dot (s, vec124)));
      currCubeType = getNodeType (currCubePointer);
      iterInner++;
    }
   currCubeProperties = getCubeProperties (currCubeType);
   if ((currCubeType != prevCubeType) && (iterOuter > 0))
      ray = normalize (refract (ray, normal, prevCubeProperties.refractionIndex / currCubeProperties.refractionIndex));

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    deltaVector = mix (deltaVector, 128 * vec111, isinf(deltaVector));
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);

    EXIT_IF (delta < 0, 0.5, 0., 1.);

    vec3  baseColor = texture (cubeTexture, vec4((currPoint - currCubeMidpoint) / currCubeSize, currCubeType)).rgb;
    float transparency = pow (currCubeProperties.transparency, delta);
    float lightCoef;
    if (currCubeProperties.transparency == 0)
      lightCoef = dot (normal, vec3 (3, -1, 7)) / 12 + 0.3;
    else
      lightCoef = 1.0;

//     switch (currCubeType) {
//       case CUBE_TYPE_AIR: {
//         //baseColor = vec3 (1., 1., 1.);
//         transparency = pow (0.995, delta);
//         break;
//       }
//       case CUBE_TYPE_DIRT: {
//         //baseColor = texture (cubeTexture, vec4((currPoint - currCubeMidpoint) / currCubeSize, 4)).rgb;
//         //baseColor = texture (depthTexture, (vec2(1,1) + ((currPoint - currCubeMidpoint) / currCubeSize).xz) / 2).rgb;
//         //baseColor = texture (depthTexture,((currPoint - currCubeMidpoint) / currCubeSize).xz).rgb;
//         //baseColor = texture (depthTexture, fPosition).rgb;
//         //baseColor = vec3(0, 1, 0);
//         // moistening effect
//         if (prevCubeType == CUBE_TYPE_WATER)
//           baseColor *= 0.3;
//         lightCoef = dot (normal, vec3 (3, -1, 7)) / 12 + 0.3;
//         break;
//       }
//       case CUBE_TYPE_WATER: {
//         baseColor = vec3 (0., 0.1, 0.7);
//         transparency = pow (0.93, delta);
//         break;
//       }
//     }

    vFragColor.xyz   += baseColor * vFragColor.w * lightCoef * (1 - transparency);
    vFragColor.w     *= transparency;

    currPoint        += ray * (delta + 0.001);
    normal            = -trunc((currPoint - currCubeMidpoint) / currCubeSize);

//     EXIT_IF (length (normal) > 2.01,  1., 0., 1.);

    currCubePointer   = getNodeNeighbour (currCubePointer, -normal);
    //if (currCubePointer != -1) {
    currCubeSize      = getNodeSize (currCubePointer);
    currCubeMidpoint  = getNodeMidpoint (currPoint, currCubeSize);

//     EXIT_IF (!pointInCube(currPoint, currCubeMidpoint, currCubeSize),  0., 1., 0.);

    prevCubeType       = currCubeType;
    prevCubeProperties = currCubeProperties;
    currCubeType      = getNodeType (currCubePointer);
  //}

    // make epsilon constant
//     EXIT_IF (delta < 0.,  0., 0., 1.);

    iterOuter++;
  }
  if (iterOuter == MAX_ITER_OUTER) vFragColor = vec4 (1., 0., 0., 1.);
}

