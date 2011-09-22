#version 130
uniform vec4 color;
//in  vec4 fColor;
out vec4 vFragColor;



void main(void)
{
  vFragColor  = color;
}
