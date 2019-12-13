#version 330
 
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
 
layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;

out vec3 position;
out vec3 normal;
 
void main()
{
	position = vec3(modelViewMatrix * vPosition);
	normal = normalize(normalMatrix * vNormal);
	
    gl_Position = projectionMatrix * modelViewMatrix * vPosition;
}