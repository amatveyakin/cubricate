#version 140


uniform isamplerBuffer octTree;
uniform isamplerBuffer siblingShiftTable;
uniform vec3 origin;

in  vec2    fPosition;
in  vec3    fDirection;


include(`RaytracingShadersCommon.h')


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
  currCubeSize = RENDER_WORLD_SIZE;
  currCubeMidpoint = vec3 (0., 0., 0.);
  currCubeType = getNodeType (currCubePointer);

  while (iterOuter < MAX_ITER_OUTER && (pointInCube(currPoint, vec3(0, 0, 0), RENDER_WORLD_SIZE))) {

    int iterInner = 0;
    while (/*iterInner < MAX_ITER_INNER &&*/ currCubeType == 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currCubeMidpoint, currPoint);
      currCubeMidpoint += (2 * s - vec111) * currCubeSize;
      currCubePointer = getNodeChild (currCubePointer, int (dot (s, vec124)));
      currCubeType = getNodeType (currCubePointer);
      iterInner++;
    }
    if (currCubeType != CUBE_TYPE_AIR) {
      vFragColor = vec4(1, 1, 1, 1) * length (currPoint - origin) / (4 * RENDER_WORLD_SIZE);
      return;
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
  vFragColor = vec4(1, 1, 1, 1) * length (currPoint - origin) / (4 * RENDER_WORLD_SIZE);

}

