#version 460
const float   PI = 3.1415926;

layout (location=0) in vec2 uv;

layout (location = 0) out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gSpecular;

uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform mat4 WorldView;
uniform mat4 WorldInverse;
uniform mat4 lightSpaceMatrix;

uniform vec2 resolution;
uniform float bias;
uniform int gammaCorrection;
uniform int castShadow;
uniform int PCF;

//uniform vec3 diffuse;    // Kd
//uniform vec3 specular;   // Ks
uniform float shininess; // alpha
uniform vec3 Light;      // Ii
uniform vec3 Ambient;    // Ia

float HardShadow(vec4 FragPosLightSpace)
{
  vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5; // map [-1,1] to [0,1]

  if (projCoords.z > 1.0)
    return 0.0;

  float shadow = 0.0;
  float currentDepth = projCoords.z;
  if (PCF == 1.0)
  {
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
      {
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
      }    
    }
    shadow /= 9.0;

    return shadow;
  }
  float closetDepth = texture(shadowMap, projCoords.xy).r;
  shadow = currentDepth - bias > closetDepth ? 1.0 : 0.0;

  return shadow;
}

void main()
{
  //vec2 uv = gl_FragCoord.xy / resolution; // use magic number for now
  vec3 FragPos = texture(gPosition, uv).rgb;
  vec3 normalVec = texture(gNormal, uv).rgb;

  vec3 lightPos_ViewSpace = (WorldView*vec4(lightPos,1.0)).xyz;
  vec4 FragPos_LightSpace = lightSpaceMatrix*WorldInverse*vec4(FragPos,1.0);

  vec3 albedoMap;
  if (gammaCorrection == 1.0)
  {
    // apply gamma correction
    albedoMap = pow(texture(gAlbedo, uv).rgb, vec3(2.2));
  }
  else
  {
    albedoMap = texture(gAlbedo, uv).rgb;
  }

  vec3 Kd = albedoMap;
  vec3 Ks = texture(gSpecular, uv).rgb;
  float alpha = shininess;
  vec3 Ii = Light;
  vec3 Ia = Ambient;
  
  vec3 V = normalize(-FragPos);
  vec3 N = normalize((normalVec * 2.0) - vec3(1.0)); // convert from [0,1] to [-1,1]
  vec3 L = normalize(lightPos_ViewSpace - FragPos);

  // calculate half vector
  vec3 H = normalize(L+V);
  
  // The Schlick approximation to the Fresnel term F
  float LH = max(dot(L,H),0.0);
  vec3 F = Ks + (vec3(1.0,1.0,1.0) - Ks) * pow((1.0 - LH),5);
  
  // The masking term G
  float G = 1.0 / pow(LH,2);
  
  // The micro-facet normal distribution term D
  float NH = max(dot(N,H),0.0);
  float D = ((alpha + 2.0) / (2.0 * PI)) * pow(NH, alpha);
  
  // Calculate BRDF portion
  vec3 BRDF = (Kd / PI) + (F * G * D) / 4.0;
  
  // micro-facet lighting
  float NL = max(dot(N,L),0.0);

  float shadowFactor = HardShadow(FragPos_LightSpace);

  vec3 I = Ia * Kd + (1.0 - shadowFactor) * Ii * NL * BRDF;

  FragColor = vec4(I,1.0);
}