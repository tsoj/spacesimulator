#version 450

#define MAX_NUM_LIGHTS 10
#define NUM_CASCADES 3

in VsOut
{
  vec3 fragPosition_TangentSpace;
  vec2 textureCoordinate;
  vec3 cameraPosition_TangentSpace;
  vec3 lightPosition_TangentSpace[MAX_NUM_LIGHTS];
  vec4 fragPosition_LightSpace[NUM_CASCADES][MAX_NUM_LIGHTS];
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
uniform sampler2D depthMap[NUM_CASCADES][MAX_NUM_LIGHTS];

uniform float cascadeDistances[NUM_CASCADES];

float shadowCalculation(int lightIndex, int cascadeIndex)
{
  vec3 projCoords = vsOut.fragPosition_LightSpace[cascadeIndex][lightIndex].xyz / vsOut.fragPosition_LightSpace[cascadeIndex][lightIndex].w;
  projCoords = projCoords * 0.5 + 0.5;
  float currentDepth = projCoords.z;
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(depthMap[cascadeIndex][lightIndex], 0);
  float bias = 0.0;
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(depthMap[cascadeIndex][lightIndex], projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  return clamp(shadow, 0.0, 1.0);
}

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
  vec3 lightColor[MAX_NUM_LIGHTS],
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
  vec3 lightColor[MAX_NUM_LIGHTS],
  float localLightPower[MAX_NUM_LIGHTS],
  vec3 toCamera,
  vec3 normal
)
{
  vec3 specularLight = vec3(0.0, 0.0, 0.0);
  for(int i = 0; i < numLights; i++)
  {
    vec3 halfDir = normalize(toLight[i] + toCamera);
    float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
    specularLight += lightColor[i] * pow(specAngle, shininess) * localLightPower[i];
  }
  return specularColor * specularLight;
}

void main()
{
  vec4 diffuseTextureColor = texture2D(diffuseTexture, vsOut.textureCoordinate);

  vec3 fragNormal_TangentSpace = normalize(texture2D(normalMap, vsOut.textureCoordinate).rgb * 2.0 - vec3(1.0, 1.0, 1.0));

  int cascadeIndex;
  float currentCascadeDistance = distance(vsOut.fragPosition_TangentSpace, vsOut.cameraPosition_TangentSpace);
  for(int i = 0; i<NUM_CASCADES; i++)
  {
    cascadeIndex = i;
    if(currentCascadeDistance<cascadeDistances[i])
    {
      break;
    }
  }


  vec3 toLight_TangentSpace[MAX_NUM_LIGHTS];
  float localLightPower[MAX_NUM_LIGHTS];
  for(int i = 0; i < numLights; i++)
  {
    toLight_TangentSpace[i] = normalize(vsOut.lightPosition_TangentSpace[i] - vsOut.fragPosition_TangentSpace);
    float lightDistance = distance(vsOut.lightPosition_TangentSpace[i], vsOut.fragPosition_TangentSpace);
    localLightPower[i] =
      calculateLocalLightPower(lightDistance, lightPower[i])*(1.0 - shadowCalculation(i, cascadeIndex));
  }

  vec3 toCamera = normalize(vsOut.cameraPosition_TangentSpace - vsOut.fragPosition_TangentSpace);

  vec3 ambientLight = ambientLight(ambientColor);
  vec3 diffuseLight = diffuseLight(diffuseColor, toLight_TangentSpace, lightColor, localLightPower, fragNormal_TangentSpace);
  vec3 specularLight = specularLight(specularColor, toLight_TangentSpace, lightColor, localLightPower, toCamera, fragNormal_TangentSpace);

  vec3 finalLight = clamp(ambientLight +
    (diffuseLight + specularLight)
    , 0.0, 1.0);

  outColor = diffuseTextureColor * vec4(finalLight,  transparency);
}
