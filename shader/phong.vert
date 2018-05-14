#version 450

#define MAX_NUM_LIGHTS 10

in layout(location = 0) vec4 vertPosition_ModelSpace;
in layout(location = 1) vec3 vertNormal_ModelSpace;
in layout(location = 2) vec3 vertTangent_ModelSpace;
in layout(location = 3) vec2 textureCoordinate;

uniform mat4 modelToWorld;
uniform mat4 worldToProjection;
uniform mat4 worldToLight[MAX_NUM_LIGHTS];
uniform mat4 worldToLight1[MAX_NUM_LIGHTS];

uniform vec3 cameraPosition_WorldSpace;
uniform vec3 lightPosition_WorldSpace[MAX_NUM_LIGHTS];
uniform int numLights;

out VsOut
{
  vec3 fragPosition_TangentSpace;
  vec2 textureCoordinate;
  vec3 cameraPosition_TangentSpace;
  vec3 lightPosition_TangentSpace[MAX_NUM_LIGHTS];
  vec4 fragPosition_LightSpace[MAX_NUM_LIGHTS];
  vec4 fragPosition_LightSpace1[MAX_NUM_LIGHTS];
} vsOut;

void main()
{
  vec3 vertNormal_WorldSpace = normalize(vec3(modelToWorld * vec4(vertNormal_ModelSpace, 0.0)));
  vec3 vertTangent_WorldSpace = normalize(vec3(modelToWorld * vec4(vertTangent_ModelSpace, 0.0)));
  vec3 vertBiTangent_WorldSpace = normalize(cross(vertNormal_WorldSpace, vertTangent_WorldSpace));
  mat3 worldToTangentSpace = transpose(mat3(
        vertTangent_WorldSpace,
        vertBiTangent_WorldSpace,
        vertNormal_WorldSpace
    ));

  vec3 vertPosition_WorldSpace = vec3(modelToWorld * vertPosition_ModelSpace);
  gl_Position = worldToProjection * vec4(vertPosition_WorldSpace, 1.0);

  vsOut.fragPosition_TangentSpace = worldToTangentSpace * vertPosition_WorldSpace;
  vsOut.cameraPosition_TangentSpace = worldToTangentSpace * cameraPosition_WorldSpace;
  for(int i = 0; i < numLights; i++)
  {
    vsOut.lightPosition_TangentSpace[i] = worldToTangentSpace * lightPosition_WorldSpace[i];
    vsOut.fragPosition_LightSpace[i] = worldToLight[i] * vec4(vertPosition_WorldSpace, 1.0);
    vsOut.fragPosition_LightSpace1[i] = worldToLight1[i] * vec4(vertPosition_WorldSpace, 1.0);
  }
  vsOut.textureCoordinate = textureCoordinate;
}
