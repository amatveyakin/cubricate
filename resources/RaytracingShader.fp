#version 140
uniform isamplerBuffer octTree;
uniform vec3 origin;

in  vec3 fDirection;

out vec4 vFragColor;

const vec3  powerVector = vec3 (1., 2., 4.);
const float chunkSize = 128;
int getNodeData(int nodePointer) {
  if (nodePointer > 30000)
    return 0;
  else
    return texelFetch (octTree, nodePointer).r;
}

bool pointInChunk (vec3 point) {
  return all (lessThan ( abs(point), vec3(chunkSize, chunkSize, chunkSize)));
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
  vFragColor = vec4(0., 0., 0., 0.1);
  //vFragColor.xyz = (ray + vec3 (1, 1, 0)) / 2;
  while ((vFragColor.w < 0.95) && (pointInChunk(currPoint))) {
    currCubePointer = 0;
    currCubeSize = chunkSize;
    currCubeMidpoint = vec3 (0., 0., 0.);
    currCubeType = getNodeData (currCubePointer);
    while (currCubeType > 0) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      vec3 s = step (currPoint, currCubeMidpoint);
      currCubeMidpoint += s * currCubeSize;
      currCubePointer = 8 * currCubePointer + int (dot (s, powerVector));
      currCubeType = getNodeData (currCubePointer);
      vFragColor = vec4(1., 1., 0,  1.);
    }
    //vFragColor.xyz += vec3(1., 1., 1.) / 10;//vFragColor.xyz * vFragColor.w + currCubeType * vec3 ( 0., 0.7, 0.); //  <-Change to color sampling
    //vFragColor.w  += 0.05;                                                       //  </

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);
    currPoint += ray * (delta + 0.0001); //make epsilon constant
  }
}

