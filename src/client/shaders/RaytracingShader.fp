#version 140
#extension GL_ARB_texture_cube_map_array : enable


struct CubeProperties {
  float transparency;
  float refractionIndex;
  int   normalMapIndex;
};


uniform isamplerBuffer   octTree;
uniform  samplerBuffer   cubeProperties;
uniform isamplerBuffer   siblingShiftTable;
uniform sampler2D        depthTexture;
uniform samplerCubeArray cubeTexture;
uniform samplerCubeArray cubeNormalMap;

uniform vec3             origin;

in  vec3    fDirection;
in  vec2    fPosition;

out vec4    vFragColor;


include(`RaytracingShadersCommon.h')


CubeProperties getCubeProperties (int cubeType) {
  CubeProperties result;
  vec4 fetch = texelFetch (cubeProperties, cubeType);
  result.transparency    = fetch.r;
  result.refractionIndex = fetch.g;
  result.normalMapIndex  = int (fetch.b);
  return result;
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
  currT = minDepth * 4 * RENDER_WORLD_SIZE - 3;
  currT = max (currT, 0);
  //currT = 0;
  currPoint = origin + ray * currT;

  CubeProperties airProperties = getCubeProperties (CUBE_TYPE_AIR);
  vec3  baseColor = texture (cubeTexture, vec4(1, 0, 0, CUBE_TYPE_AIR)).rgb;
  float transparency = pow (airProperties.transparency, currT);
  vFragColor.xyz   = baseColor * (1 - transparency);
  vFragColor.w     = transparency;

  //vFragColor = vec4 (0., 0., 0., 1.); // fourth component is transparency, not opacity!
  //int iterOuter = int (texture (cubeTexture, vec3(1, 0.5, -0.5)).r);
  int iterOuter = 0;
  currCubePointer = 0;
  currCubeSize = RENDER_WORLD_SIZE;
  currCubeMidpoint = vec3 (0., 0., 0.);
  currCubeType = getNodeType (currCubePointer);
  //prevCubeType = currCubeType;
  while (iterOuter < MAX_ITER_OUTER && (vFragColor.w > 0.05) && (pointInCube(currPoint, vec3(0, 0, 0), RENDER_WORLD_SIZE))) {
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

    normal              = colorToVector (texture (cubeNormalMap, vec4 (currPoint - currCubeMidpoint, currCubeProperties.normalMapIndex)).xyz);
    vec3 oldRay = ray;
    if ((currCubeType != prevCubeType) && (iterOuter > 0))
      ray = refract (oldRay, normal, prevCubeProperties.refractionIndex / currCubeProperties.refractionIndex);
    if (length (ray) < 0.0001)
      ray = reflect (oldRay, normal);

    nextPoint = currCubeMidpoint + currCubeSize * sign (ray);
    deltaVector = abs ((nextPoint - currPoint) / ray);
    //deltaVector = mix (deltaVector, 128 * vec111, isinf(deltaVector));
    delta = min (min (deltaVector.x, deltaVector.y), deltaVector.z);

//     EXIT_IF (delta < 0, 0.5, 0., 1.);

    vec4 baseColor = texture (cubeTexture, vec4 (fract ((currPoint - currCubeMidpoint) + vec111 * currCubeSize) - vec111 / 2, currCubeType));
    float materialTransparency = currCubeProperties.transparency;

//     if (currCubeType == CUBE_TYPE_WATER)
//       baseColor.xyz = vec111 - (vec111 - baseColor.xyz) * getNodeParameter (currCubePointer) / float (MAX_FLUID_SATURATION);
    if (currCubeType == CUBE_TYPE_WATER)
      materialTransparency = 1 - getNodeParameter (currCubePointer) / float (MAX_FLUID_SATURATION) * (1 - materialTransparency);


    float transparency;
    if (materialTransparency == 0.)
      transparency = 0.;
    else
      transparency = pow (materialTransparency, delta);
//     if ((currCubeType != prevCubeType) && (iterOuter > 0))
//       transparency *= baseColor.a;

    float lightCoef;
    if (currCubeProperties.transparency == 0)
      lightCoef = dot (normal, vec3 (3, -1, 7)) / 12 + 0.3;
    else
      lightCoef = 1.0;

    float moisteningCoeff;
    if (currCubeType > CUBE_TYPE_WATER && prevCubeType == CUBE_TYPE_WATER)
      moisteningCoeff = 0.3;
    else
      moisteningCoeff = 1.;


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

    vFragColor.xyz   += baseColor.rgb * vFragColor.w * lightCoef * moisteningCoeff * (1 - transparency);
    vFragColor.w     *= transparency;

    currPoint        += ray * (delta + 0.0001);
    //normal            = -trunc((currPoint - currCubeMidpoint) / currCubeSize);
    vec3 cubeNormal     = colorToVector (texture (cubeNormalMap, vec4 (currPoint - currCubeMidpoint, 0)).xyz);
    //     EXIT_IF (length (normal) > 2.01,  1., 0., 1.);

    currCubePointer   = getNodeNeighbour (currCubePointer, cubeNormal);
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

