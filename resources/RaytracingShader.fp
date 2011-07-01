#version 140

uniform isamplerBuffer octTree;
uniform samplerCube   cubeTexture;
uniform vec3 origin;

in  vec3 fDirection;

out vec4 vFragColor;

const vec3  vec111 = vec3 (1., 1., 1.);
const vec3  vec123 = vec3 (1., 2., 3.);
const vec3  vec124 = vec3 (1., 2., 4.);

const int   TREE_HEIGHT = 8;
const float CHUNK_SIZE = 128;
const int   MAX_ITER_OUTER = 100;
const int   MAX_ITER_INNER = 100;

const int   CUBE_TYPE_AIR     = 0;
const int   CUBE_TYPE_DIRT    = 1;
const int   CUBE_TYPE_WATER   = 2;

// Node structure: Type | NeigbourZ- | NeigbourY- | NeigbourX- | Size | NeigbourX+ | NeigbourY+ | NeigbourZ+
const int   NODE_STRUCT_SIZE       = 8;
const int   NODE_OFFSET_TYPE       = 0;
const int   NODE_OFFSET_HEIGHT     = 4;
const int   NODE_OFFSET_NEIGHBOURS = 4;

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

const float refractionIndices[] = float[](1., 1., 1.333);


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
int getNodeNeighbour (int nodePointer, int neighbourIndex) {
  return texelFetch (octTree, NODE_STRUCT_SIZE * nodePointer + NODE_OFFSET_NEIGHBOURS + neighbourIndex).r;
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

  vFragColor = vec4 (0., 0., 0., 1.); //fourth component is transparency, not opacity!
  int iterOuter = 0;

  currCubePointer = 0;
  currCubeSize = CHUNK_SIZE;
  currCubeMidpoint = vec3 (0., 0., 0.);
  currCubeType = getNodeType (currCubePointer);

  while (iterOuter < MAX_ITER_OUTER && (vFragColor.w > 0.05) && (pointInCube(currPoint, vec3(0, 0, 0), CHUNK_SIZE))) {
//     if (currCubePointer < 0) {
//       vFragColor = vec4 (1., 1., 0., 1.);
//       return;
//     }
//     else if (currCubePointer > 8) {
//       vFragColor = vec4 (0., 1., 1., 1.);
//       return;
//     }

    int iterInner = 0;
    while (/*iterInner < MAX_ITER_INNER &&*/ currCubeType == 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currCubeMidpoint, currPoint);
      currCubeMidpoint += (2 * s - vec111) * currCubeSize;
      currCubePointer = getNodeChild (currCubePointer, int (dot (s, vec124)));
      currCubeType = getNodeType (currCubePointer);
      iterInner++;
    }
//     if (currCubeType != prevCubeType)
//       ray = normalize (refract (ray, normal, 1/ (refractionIndices[currCubeType] / refractionIndices[prevCubeType])));

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    deltaVector = mix (deltaVector, 128 * vec111, isinf(deltaVector));
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);


    if ( delta < 0) {
      vFragColor = vec4 (0.5, 0., 1., 1.);
      return;
    }

    vec3 baseColor = vec3 (0., 0., 0.);
    float transparency = 0.;
    float lightCoef = 1.;
    switch (currCubeType) {
      case CUBE_TYPE_AIR: {
        baseColor = vec3 (1., 1., 1.);
        transparency = pow (0.998, delta);
        break;
      }
      case CUBE_TYPE_DIRT: {
        baseColor = texture (cubeTexture, (currPoint - currCubeMidpoint) / currCubeSize).rgb;
        // moistening effect
        if (prevCubeType == CUBE_TYPE_WATER)
          baseColor *= 0.3;
        lightCoef = dot (normal, vec3(3, -1, 7))/12 + 0.3;
        break;
      }
      case CUBE_TYPE_WATER: {
        baseColor = vec3 (0., 0.1, 0.7);
        transparency = pow (0.93, delta);
        break;
      }
    }

    vFragColor.xyz   += baseColor * vFragColor.w * lightCoef * (1 - transparency);
    vFragColor.w     *= transparency;

    currPoint        += ray * (delta + 0.001);
    normal            = -trunc((currPoint - currCubeMidpoint) / currCubeSize);

//     if ( length (normal) > 2.01) {
//       vFragColor = vec4 (1., 0., 1., 1.);
//       return;
//     }

    currCubePointer   = getNodeNeighbour (currCubePointer, -int (round (dot (normal, vec123))));
    //if (currCubePointer != -1) {
      currCubeSize      = getNodeSize (currCubePointer);
      currCubeMidpoint  = getNodeMidpoint (currPoint, currCubeSize);
/*
      if (!pointInCube(currPoint, currCubeMidpoint, currCubeSize)) {
        vFragColor =  vec4(0, 1, 0, 0);
        return;
      }*/

    prevCubeType      = currCubeType;
    currCubeType      = getNodeType (currCubePointer);
  //}

    //make epsilon constant
//     if (delta < 0) {
//       vFragColor = vec4(0, 0, 1, 1);
//       return;
//     }

    iterOuter++;
  }
  if (iterOuter == MAX_ITER_OUTER) vFragColor = vec4 (1., 0., 0., 1.);
}

