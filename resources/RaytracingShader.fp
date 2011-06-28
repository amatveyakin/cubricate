#version 140
uniform samplerBuffer octTree;
uniform vec3 origin;

in  vec3 fDirection;

out vec4 vFragColor;

const vec3  powerVector = vec3 (1., 2., 4.);
const vec3  onesVector  = vec3 (1., 1., 1.);
const float chunkSize = 128;

int getNodeData(int nodePointer) {
  if (nodePointer > 300000)
    return 0;
  else
    return int (texelFetch (octTree, nodePointer).r);
}

bool pointInChunk (vec3 point) {
  return all (lessThan (abs(point), vec3(chunkSize, chunkSize, chunkSize)));
}

void main(void)
{
  int   currCubePointer = 0;
  int   currCubeType;
  vec3  currCubeMidpoint;
  float currCubeSize;

  vec3  ray = fDirection;
  float currT, nextT, delta;
  vec3  deltaVector;
  vec3  currPoint, nextPoint;

  currT = 0;
  currPoint = origin;
  //vFragColor.xyz = (ray + vec3 (1, 1, 0)) / 2;
  //vFragColor = vec4 (texelFetch (octTree, 1).xyz / 2, 1);
  //return;
  vFragColor = vec4 (0., 0., 0., 0.);
  int iterOuter = 0;
  while (iterOuter < 100 && (vFragColor.w < 0.95) && (pointInChunk(currPoint))) {
    currCubePointer = 0;
    currCubeSize = chunkSize;
    currCubeMidpoint = vec3 (0., 0., 0.);
    currCubeType = getNodeData (currCubePointer);
    int iterInner = 0;
    while (/*iterInner < 100 &&*/ currCubeType == 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currCubeMidpoint, currPoint);
      currCubeMidpoint += (2 * s - onesVector) * currCubeSize;
      currCubePointer = 8 * currCubePointer + int (dot (s, powerVector)) + 1;
//       if (currCubePointer >= 9) {
//         vFragColor = vec4(1, 1, 1, 1);
//         return;
//       }
      currCubeType = getNodeData (currCubePointer);
      //vFragColor += vec4(3 / currCubeSize, 0., 0,  0.1);
      iterInner++;
    }
    if (currCubeType > 0) {
      vFragColor = vec4(0, 1, 0, 0) * (256 - length(currPoint - origin)) / 256.;
      vFragColor.w = 1;
    }

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);
    currPoint += ray * (delta + 0.0001); //make epsilon constant
    if (delta < 0) {
      vFragColor = vec4(0, 0, 1, 1);
      return;
    }
    iterOuter++;
  }
  if (iterOuter == 100) vFragColor = vec4(1, 0, 0, 1);
}

