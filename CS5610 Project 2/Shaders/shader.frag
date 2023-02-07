#version 330 core

// shader inspored from: https://community.khronos.org/t/adding-normals-into-fragment-shader/73506

in vec3 Normal;
in vec3 FragPos;
out vec4 color;

// uniform vec3 objColor;
// uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
  vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
  vec3 objColor = vec3(1.0f, 0.0f, 0.0f);

  // ambient
  float ambientStrength = 0.1f;
  vec3 ambient = ambientStrength * lightColor;

  // Diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 result = (ambient + diffuse) * objColor;
  color = vec4(result, 1.0f);
}