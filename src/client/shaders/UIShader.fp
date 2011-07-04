#version 140
uniform sampler2D UITexture;

in vec2 fPosition;

out vec4 vFragColor;

void main (void)
{
  vec4 baseColor  = texture (UITexture, fPosition);

  if (baseColor.a < 0.01) discard;
//   if (baseColor.b == 0) discard;
  //if (fPosition.x < 0.5 && fPosition.y < 0.5) discard;

  vFragColor  = baseColor;
}
