#version 450

in layout(location = 0) vec4 modelPosition;

uniform mat4 modelToWorld;
uniform mat4 worldToProjection;

void main()
{
  vec3 worldPosition = vec3(modelToWorld * modelPosition);
  gl_Position = worldToProjection * vec4(worldPosition, 1.0);
}
