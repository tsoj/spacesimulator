#version 450

#define MAX_NUM_LIGHTS 10

out VsOut
{
  vec3 fragPosition_TangentSpace;
  vec2 textureCoordinate;
  vec3 cameraPosition_TangentSpace;
  vec3 lightPosition_TangentSpace[MAX_NUM_LIGHTS];
  vec4 fragPosition_LightSpace[MAX_NUM_LIGHTS];
} vsOut;

layout(location = 0) out vec4 outColor;

uniform float lightPower[MAX_NUM_LIGHTS];
uniform vec3 lightColor[MAX_NUM_LIGHTS];
uniform int numLights;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float transparency;
uniform float shininess;
uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;

float calculateLocalLightPower(float lightDistance, float lightPower)
{
  return clamp(lightPower/pow(lightDistance, 2.0), 0.0, 1.0);
}

vec3 ambientLight(vec3 ambientColor)
{
  return ambientColor * vec3(0.0, 0.0, 0.0);
}

vec3 diffuseLight(
  vec3 diffuseColor,
  vec3 toLight[MAX_NUM_LIGHTS],
  float lightColor[MAX_NUM_LIGHTS],
  float localLightPower[MAX_NUM_LIGHTS],
  vec3 normal
)
{
  vec3 diffuseLight = vec3(0.0, 0.0, 0.0);
  for(int i = 0; i < numLights; i++)
  {
    diffuseLight +=
      lightColor[i] * clamp(dot(toLight[i], normal), 0.0, 1.0) * localLightPower[i];
  }
  return diffuseColor * clamp(diffuseLight, 0.0, 1.0);
}

vec3 specularLight(
  vec3 specularColor,
  vec3 toLight[MAX_NUM_LIGHTS],
  float lightColor[MAX_NUM_LIGHTS],
  float localLightPower[MAX_NUM_LIGHTS],
  vec3 toCamera,
  vec3 normal
)
{
  float specularLight = 0;
  for(int i = 0; i < numLights; i++)
  {
    vec3 halfDir = normalize(toLight[i] + toCamera);
    float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
    specularLight += lightColor[i] * pow(specAngle, 20) * localLightPower[i];//TODO:shininess
  }
  return specularColor * specularLight;
}

void main()
{
  vec4 diffuseTextureColor = texture2D(diffuseTexture, textureCoordinate);

  vec3 fragNormal_TangentSpace = normalize(texture2D(normalMap, textureCoordinate).rgb * 2.0 - vec3(1.0, 1.0, 1.0));

  vec3 toLight_TangentSpace[MAX_NUM_LIGHTS];
  float localLightPower[MAX_NUM_LIGHTS];
  for(int i = 0; i < numLights; i++)
  {
    toLight_TangentSpace[i] = normalize(lightPosition_TangentSpace[i] - fragPosition_TangentSpace);
    float lightDistance = distance(lightPosition_TangentSpace[i], fragPosition_TangentSpace);
    localLightPower[i] = calculateLocalLightPower(lightDistance, lightPower[i]);
  }

  vec3 toCamera = normalize(cameraPosition_TangentSpace - fragPosition_TangentSpace);

  vec3 ambientLight = ambientLigth(ambientColor);
  vec3 diffuseLight = diffuseLight(diffuseColor, toLight_TangentSpace, localLightPower, fragNormal_TangentSpace);
  vec3 specularLight = specularLight(specularColor, toLight_TangentSpace, localLightPower, toCamera, fragNormal_TangentSpace);

  vec3 finalLight = clamp(ambientLight + diffuseLight + specularLight, 0.0, 1.0);

  outColor = textureColor * vec4(finalLight,  transparency);
}
