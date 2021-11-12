#version 460

layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec3 normalVec;
layout (location = 2) in vec2 texVec;
layout (location = 3) in vec3 tanVec;
layout (location = 4) in vec3 biTanVec;

layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gPosition;
layout (location = 3) out vec4 gDepth;
layout (location = 4) out vec4 gSpecular;

uniform vec3 diffuseColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;

uniform int isTextureSupported;
uniform int isModel;

float near = 1;
float far = 20000.0;

float LinearizeDepth(float depth)
{
  depth = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - depth * (far - near));
}

void main()
{
  vec2 uv;
  vec3 T = normalize(tanVec);
  vec3 B;
  // model
  if (isModel == 1)
  {
    uv = texVec;
    gSpecular = texture(texture_specular1,uv);
    B = normalize(biTanVec);
  }
  else
  {
    uv = texVec * 10.0;
    gSpecular = vec4(vec3(0.3),1.0);
    B = normalize(cross(T,normalVec));
  }
  
  // not model
  if (isTextureSupported == 1)
  {
    gAlbedo = vec4(texture(texture_diffuse1,uv).rgb,1.0);//vec4(diffuseColor, 1.0);
    // normal mapping
    vec3 delta = texture(texture_normal1, uv).xyz;
    // convert [0,1] to [-1,1]
    delta = delta * 2.0 - vec3(1.0);
    // adjust normal base on normal map
    vec3 norm = delta.x*T + delta.y*B + delta.z*normalVec;
    // map [-1,1] to [0,1]
    vec3 FinalNormal = norm * 0.5 + 0.5;
    gNormal = vec4(FinalNormal, 1.0);
  }
  else
  {
    gAlbedo = vec4(diffuseColor,1.0);
    gNormal = vec4(normalVec * 0.5 + 0.5,1.0);
  }
  
  // Frag position
  gPosition = vec4(FragPos, 1.0); 
  float depth = LinearizeDepth(gl_FragCoord.z) * 1.0 / far;
  gDepth = vec4(vec3(depth),1.0);
}