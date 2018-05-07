#version 450

out layout(location = 0) vec2 textureCoordinate;

in layout(location = 0) vec2 position;
in layout(location = 1) vec2 texCoords;

void main()
{
  gl_Position = vec4(position, 0.0, 1.0);
  textureCoordinate = texCoords;
}
