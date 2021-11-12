#version 460

layout (location=0) in vec2 uv;

layout (location=0) out vec4 FragColor;

uniform sampler2D screenTexture;

uniform float exposure;
uniform int gammaCorrection;

void main()
{
  vec4 tex = texture(screenTexture,uv);
  if (gammaCorrection == 1.0)
  {
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-tex.rgb * exposure);
    float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
  }
  else
  {
    FragColor = tex;
  }
}