// Object3D default shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 FragPos;
out vec3 Normal;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
	FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	Normal = a_Normal;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
uniform vec3 u_LightPos;
uniform vec3 u_LightColor;

in vec3 FragPos;
in vec3 Normal;

void main()
{
	vec3 lightPos = u_LightPos;
	vec3 lightColor = u_LightColor;

	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;


	vec3 temp = (ambient + diffuse);
	vec4 result = vec4(temp.x, temp.y, temp.z, 1.0) * u_Color;
	color = vec4(result.x, result.y, result.z, 1.0);
}