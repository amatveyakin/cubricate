#version 140
uniform samplerBuffer octTree;
uniform samplerCube   cubeTexture;
uniform vec3 origin;

in  vec3 fDirection;

out vec4 vFragColor;

const vec3  powerVector = vec3 (1., 2., 4.);
const vec3  onesVector  = vec3 (1., 1., 1.);
const float chunkSize = 128;
const int   MAX_ITER_OUTER = 100;
const int   MAX_ITER_INNER = 100;

const int   CUBE_TYPE_AIR     = 0;
const int   CUBE_TYPE_DIRT    = 1;
const int   CUBE_TYPE_WATER   = 2;

const float refractionIndices[3] = float[3](1., 1., 1.333);


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
  //vFragColor.xyz = (ray + vec3 (1, 1, 0)) / 2;
  //vFragColor = vec4 (texelFetch (octTree, 1).xyz / 2, 1);
  //return;
  vFragColor = vec4 (0., 0., 0., 1.); //fourth component is transparency, not opacity!
  int iterOuter = 0;
  while (iterOuter < MAX_ITER_OUTER && (vFragColor.w > 0.05) && (pointInChunk(currPoint))) {
    currCubePointer = 0;
    currCubeSize = chunkSize;
    currCubeMidpoint = vec3 (0., 0., 0.);
    currCubeType = getNodeData (currCubePointer);
    int iterInner = 0;
    while (/*iterInner < MAX_ITER_INNER &&*/ currCubeType == 255) {  // that means "no-leaf node"
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
    if (currCubeType != prevCubeType)
      ray = normalize (refract (ray, normal, 1/ (refractionIndices[currCubeType] / refractionIndices[prevCubeType])));

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    //deltaVector = mix (deltaVector, 128 * onesVector, isinf(deltaVector));
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);

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

    vFragColor.xyz += baseColor * vFragColor.w * lightCoef * (1 - transparency);
    vFragColor.w   *= transparency;

    currPoint += ray * (delta + 0.001);
    normal = -trunc((currPoint - currCubeMidpoint) / currCubeSize);
    //make epsilon constant
//     if (delta < 0) {
//       vFragColor = vec4(0, 0, 1, 1);
//       return;
//     }
    prevCubeType = currCubeType;
    iterOuter++;
  }
  if (iterOuter == MAX_ITER_OUTER) vFragColor = vec4(1, 0, 0, 1);
}

