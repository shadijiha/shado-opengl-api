// Basic Texture Shader

#type vertex
#version 430 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;


uniform mat4 u_ViewProjection;
out vec4 v_Color;
out vec3 v_LocalPosition;
out float v_Thickness;
out float v_Fade;

void main()
{
	v_Color = a_Color;
	v_LocalPosition = a_LocalPosition;
	v_Thickness = a_Thickness;
	v_Fade = a_Fade;
	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec3 v_LocalPosition;
in float v_Thickness;
in float v_Fade;

void main()
{
	float distance = 1.0 - length(v_LocalPosition);
	float circleAlpha = smoothstep(0.0, v_Fade, distance);
	circleAlpha *= smoothstep(v_Thickness + v_Fade, v_Thickness, distance);

	
	color = v_Color;
	color.a *= circleAlpha;
}