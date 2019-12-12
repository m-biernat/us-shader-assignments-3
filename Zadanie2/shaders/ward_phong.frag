#version 330

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

in vec3 position; // interpolowana pozycja
in vec3 normal; // interpolowany normal

const float k = 2.0;

out vec4 fColor; // wyjsciowy kolor fragmentu
 
void main()
{
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(vec3(lightPosition) - position);
	vec3 viewDir = normalize(vec3(0.0, 0.0, 0.0) - position);

	vec3 ambient = lightAmbient * materialAmbient;

	vec3 diffuse = lightDiffuse * materialDiffuse * max(dot(lightDir, norm), 0.0);

	vec3 specular = vec3(0.0, 0.0, 0.0);
	
	if (dot(lightDir, viewDir) > 0.0)
	{
		vec3 halfDir = normalize(lightDir + viewDir);
		
		float nh2 = pow(dot(norm, halfDir), 2);
		
		float tg2theta = (1 - nh2) / nh2; 

		specular = lightSpecular * materialSpecular * exp(-k * tg2theta);
	}

    fColor = vec4(clamp(ambient + diffuse + specular, 0.0, 1.0), 1.0);
}