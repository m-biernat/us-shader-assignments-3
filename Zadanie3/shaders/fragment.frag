#version 330

// informacje o zrodle swiatla
const int NUM_OF_LIGHTS = 2;

struct Light
{
	vec4 position;
	vec3 direction;
	
	float cutOffAngle;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	bool enabled;
};

uniform Light light[NUM_OF_LIGHTS];

// Parametry swiatla punktowego
float constant = 1.0;
float linear = 0.007;
float quadratic = 0.0002;

// informacje o materiale
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

in vec3 position;
in vec3 normal;

out vec4 fColor;
 
vec3 CalcLight(Light light, vec3 norm, vec3 viewDir);

void main()
{
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(vec3(0.0, 0.0, 0.0) - position);

	vec3 result = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < NUM_OF_LIGHTS; i++)
	{
		if (light[i].enabled)
			result += CalcLight(light[i], norm, viewDir);
	}

	fColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}

vec3 CalcLight(Light light, vec3 norm, vec3 viewDir)
{
	vec3 ambient = materialAmbient * light.ambient;
	vec3 lightDir, specular;
	
	float theta;

	if (light.position.w == 1.0)
		lightDir = normalize(vec3(light.position) - position);
	else
		lightDir = normalize(-light.direction);

	if (light.cutOffAngle == radians(180.0))
		theta = radians(360.0);
	else
		theta = dot(lightDir, normalize(-light.direction));

	if (theta > light.cutOffAngle)
	{
		vec3 diffuse = materialDiffuse * max(dot(norm, lightDir), 0.0) * light.diffuse;
	
		if (dot(lightDir, viewDir) > 0.0)
		{
			vec3 refl = reflect(-lightDir, norm);
			specular = pow(max(dot(viewDir, refl), 0.0), materialShininess) * materialSpecular * light.specular;
		}

		if (light.position.w == 1.0)
		{
			float dist = length(vec3(light.position) - position);
			float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

			ambient *= attenuation;
			diffuse *= attenuation;
			specular *= attenuation;
		}
		
		return (ambient + diffuse + specular);
	}
	else
		return vec3(0.0);
}
