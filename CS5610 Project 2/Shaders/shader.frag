#version 330 core

// shader inspored from: https://community.khronos.org/t/adding-normals-into-fragment-shader/73506

in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord;
in vec3 camPos;

layout(location = 0) out vec4 color;

// uniform vec3 objColor;
// uniform vec3 lightColor;
uniform vec3 lightPos;

vec4 texColor = vec4(1,0,0,1);
vec3 objColor = texColor.rgb;
float alpha = texColor.a;
vec3 specColor = vec3(1.0, 0.8, 0.1);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float lightPower = 5.0;
float shininess = 200.0;
float shadowEffect = 0.5;

void main()
{

	vec3 lightDir = lightPos - FragPos;
	float distance = length(lightDir);
	distance = distance * distance;
	lightDir = normalize(lightDir);

	// ambient
	float ambientStrength = 0.5f;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse 
	vec3 norm = normalize(Normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Specular (blinn phong shading)
	float cosAngIncidence = dot(norm, lightDir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	vec3 viewDir = normalize(camPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(norm, halfDir), 0.0);
	specAngle = clamp(specAngle, 0, 1);
	specAngle = cosAngIncidence != 0.0 ? specAngle : 0.0;
	vec3 specular = pow(specAngle, shininess) * specColor;
	
	// calculate color of obj without shadows
	vec3 result = (ambient + diffuse) * objColor  + specular;
	color = vec4(result, alpha);
}