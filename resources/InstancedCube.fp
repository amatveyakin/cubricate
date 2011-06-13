// Fragment Shader

#version 400

uniform vec4 color;
uniform sampler2DArray squareTexture;

in vec4 f_normal;
in vec4 f_position;
in vec2 f_tex_coord;
flat in int  f_type;

out vec4 vFragColor;



void main(void)
{
  vec4 base_color;
  float light_coeff = 0;

  //if ( f_type == 0 ) discard;

  base_color  = texture (squareTexture, vec3 (f_tex_coord, f_type));
  light_coeff = 0.5 * max (dot (f_normal, normalize (f_position - vec4 (1.0, 0.0, 0., 0.))), 0);
  vFragColor  = (0.1 + light_coeff) * base_color;
}
