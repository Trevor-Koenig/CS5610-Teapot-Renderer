#version 330 core

// shader inspored from: https://community.khronos.org/t/adding-normals-into-fragment-shader/73506

in vec3 Normal;
in vec3 FragPos;
out vec4 color;

// uniform vec3 objColor;
// uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;


vec3 objColor = vec3(1.0f, 0.0f, 0.0f);
vec3 specColor = vec3(1.0, 1.0, 1.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float lightPower = 4.0;
float shininess = 128.0;

void main()
{

  vec3 lightDir = lightPos - FragPos;
  float distance = length(lightDir);
  distance = distance * distance;
  lightDir = normalize(lightDir);

  // ambient
  float ambientStrength = 0.3f;
  vec3 ambient = ambientStrength * lightColor;

  // Diffuse 
  vec3 norm = normalize(Normal);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // Specular (blinn phong shading)
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 halfDir = normalize(lightDir + viewDir);
  float specAngle = max(dot(halfDir, Normal), 0.0);
  float specular = pow(specAngle, shininess);


  vec3 result = (ambient + diffuse) * objColor + specColor * specular * lightColor * lightPower;
  color = vec4(result, 1.0f);
}