#version 130
struct 
uniform samplerBuffer octTree;
uniform vec3 origin;

in  vec3 fDirection;

out vec4 vFragColor;

const vec3 powerVector = vec3 (1., 2., 4.);

void main(void)
{
  int   currCubePointer = 0;
  byte  currCubeType;
  vec3  currCubeMidpoint;
  float currCubeSize;
  
  vec3  ray = fDirection;
  float currT, nextT, delta;
  vec3  deltaVector;
  vec3  currPoint, nextPoint;

  currT = 0; 
  currPoint = origin;
  vFragColor = vec4(0., 0., 0., 0.);
  while (vFragColor.w < 0.95) {    
    currCubePointer = 0;                   // <\
    currCubeSize = 128.;                   // <-kd-restart algorithm, must be optimized
    currCubeMidpoint = vec3 (0., 0., 0.);  // </
    currCubeType = texelFetch (octTree, currCubePointer);
    while (currCubeType = 255) {  // that means "no-leaf node"
      currCubeSize /= 2.;
      uint3 s = step (currPoint, currCubeMidpoint);
      currCubeMidpoint += s * currCubeSize;
      currCubePointer = 8 * currCubePointer + dot (s, powerVector);
      currCubeType = texelFetch (octTree, currCubePointer);
    }
    vFragColor.xyz = vFragColor.xyz * vFragColor.w + currCubeType * vec3 ( 0. 0.7, 0.); //  <-Change to color sampling
    vFragColor.w  += currCubeType;                                                       //  </ 
    
    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = (nextPoint - currPoint) / ray;
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);
    currPoint += ray * (delta + 0.0001); //make epsilon constant
   }
}
