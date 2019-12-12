#version 330
 
uniform mat4 modelViewMatrix; // macierz model-widok
uniform mat3 normalMatrix; // macierz do transformacji normala
uniform mat4 projectionMatrix; // macierz projekcji

// informacje o zrodle swiatla
uniform vec4 lightPosition; // we wspolrzednych oka
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

// informacje o materiale
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
 
layout (location = 0) in vec3 vPosition; // pozycja wierzcholka w lokalnym ukladzie wspolrzednych
layout (location = 1) in vec3 vNormal; // normal

const float k = 2.0;

out vec3 color; // kolor obliczony dla wierzcholka
 
void main()
{
	vec3 position = vec3(modelViewMatrix * vec4(vPosition, 1.0));
	vec3 normal = normalize(normalMatrix * vNormal);
	vec3 lightDir = normalize(vec3(lightPosition) - position);
	vec3 viewDir = normalize(vec3(0.0, 0.0, 0.0) - position);

	vec3 ambient = lightAmbient * materialAmbient;

	vec3 diffuse = lightDiffuse * materialDiffuse * max(dot(lightDir, normal), 0.0);

	vec3 specular = vec3(0.0, 0.0, 0.0);
	
	if(dot(lightDir, viewDir) > 0.0)
	{
		vec3 halfDir = normalize(lightDir + viewDir);
		
		float nh2 = pow(dot(normal, halfDir), 2);
		
		float tg2theta = (1 - nh2) / nh2; 

		specular = lightSpecular * materialSpecular * exp(-k * tg2theta);
	}

	color = clamp(ambient + diffuse + specular, 0.0, 1.0);
    gl_Position = projectionMatrix * modelViewMatrix * vec4(vPosition, 1.0);
}