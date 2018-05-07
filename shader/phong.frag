#version 450

#define MAX_NUM_LIGHTS 10

in VsOut
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
uniform sampler2D depthMap;

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture2D(depthMap, projCoords.xy).r;
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // check whether current frag pos is in shadow
  float shadow = currentDepth - 0.0001 > closestDepth  ? 1.0 : 0.0;

  return shadow;
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

  vec3 toLight_TangentSpace[MAX_NUM_LIGHTS];
  float localLightPower[MAX_NUM_LIGHTS];
  for(int i = 0; i < numLights; i++)
  {
    toLight_TangentSpace[i] = normalize(vsOut.lightPosition_TangentSpace[i] - vsOut.fragPosition_TangentSpace);
    float lightDistance = distance(vsOut.lightPosition_TangentSpace[i], vsOut.fragPosition_TangentSpace);
    localLightPower[i] = calculateLocalLightPower(lightDistance, lightPower[i]);
  }

  vec3 toCamera = normalize(vsOut.cameraPosition_TangentSpace - vsOut.fragPosition_TangentSpace);

  vec3 ambientLight = ambientLight(ambientColor);
  vec3 diffuseLight = diffuseLight(diffuseColor, toLight_TangentSpace, lightColor, localLightPower, fragNormal_TangentSpace);
  vec3 specularLight = specularLight(specularColor, toLight_TangentSpace, lightColor, localLightPower, toCamera, fragNormal_TangentSpace);

  vec3 finalLight = clamp(ambientLight +
    (diffuseLight + specularLight)*(1.0 - shadowCalculation(vsOut.fragPosition_LightSpace[0], fragNormal_TangentSpace, normalize(vsOut.lightPosition_TangentSpace[0] - vsOut.fragPosition_TangentSpace)))
    , 0.0, 1.0);

  outColor = diffuseTextureColor * vec4(finalLight,  transparency);
}
