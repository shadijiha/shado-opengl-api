//--------------------------
// - Hazel 2D -
// Renderer2D Circle Shader
// --------------------------

#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;
layout(location = 6) in vec2 a_TexCoord;
layout(location = 7) in float a_TexIndex;
layout(location = 8) in float a_TilingFactor;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;
layout (location = 5) out vec2 v_TexCoord;
layout (location = 6) out flat float v_TexIndex;
layout (location = 7) out float v_TilingFactor;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;

	v_EntityID = a_EntityID;

	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;
layout (location = 5) in vec2 v_TexCoord;
layout (location = 6) in flat float v_TexIndex;
layout (location = 7) in float v_TilingFactor;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
    // Calculate distance and fill circle with white
    float distance = 1.0 - length(Input.LocalPosition);
    float circle = smoothstep(0.0, Input.Fade, distance);
    circle *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

	if (circle == 0.0)
		discard;

    // Set output color
    o_Color = Input.Color;
	o_Color.a *= circle;

	o_EntityID = v_EntityID;

	switch(int(v_TexIndex))
	{
		case  0: o_Color *= texture(u_Textures[ 0], v_TexCoord * v_TilingFactor); break;
		case  1: o_Color *= texture(u_Textures[ 1], v_TexCoord * v_TilingFactor); break;
		case  2: o_Color *= texture(u_Textures[ 2], v_TexCoord * v_TilingFactor); break;
		case  3: o_Color *= texture(u_Textures[ 3], v_TexCoord * v_TilingFactor); break;
		case  4: o_Color *= texture(u_Textures[ 4], v_TexCoord * v_TilingFactor); break;
		case  5: o_Color *= texture(u_Textures[ 5], v_TexCoord * v_TilingFactor); break;
		case  6: o_Color *= texture(u_Textures[ 6], v_TexCoord * v_TilingFactor); break;
		case  7: o_Color *= texture(u_Textures[ 7], v_TexCoord * v_TilingFactor); break;
		case  8: o_Color *= texture(u_Textures[ 8], v_TexCoord * v_TilingFactor); break;
		case  9: o_Color *= texture(u_Textures[ 9], v_TexCoord * v_TilingFactor); break;
		case 10: o_Color *= texture(u_Textures[10], v_TexCoord * v_TilingFactor); break;
		case 11: o_Color *= texture(u_Textures[11], v_TexCoord * v_TilingFactor); break;
		case 12: o_Color *= texture(u_Textures[12], v_TexCoord * v_TilingFactor); break;
		case 13: o_Color *= texture(u_Textures[13], v_TexCoord * v_TilingFactor); break;
		case 14: o_Color *= texture(u_Textures[14], v_TexCoord * v_TilingFactor); break;
		case 15: o_Color *= texture(u_Textures[15], v_TexCoord * v_TilingFactor); break;
		case 16: o_Color *= texture(u_Textures[16], v_TexCoord * v_TilingFactor); break;
		case 17: o_Color *= texture(u_Textures[17], v_TexCoord * v_TilingFactor); break;
		case 18: o_Color *= texture(u_Textures[18], v_TexCoord * v_TilingFactor); break;
		case 19: o_Color *= texture(u_Textures[19], v_TexCoord * v_TilingFactor); break;
		case 20: o_Color *= texture(u_Textures[20], v_TexCoord * v_TilingFactor); break;
		case 21: o_Color *= texture(u_Textures[21], v_TexCoord * v_TilingFactor); break;
		case 22: o_Color *= texture(u_Textures[22], v_TexCoord * v_TilingFactor); break;
		case 23: o_Color *= texture(u_Textures[23], v_TexCoord * v_TilingFactor); break;
		case 24: o_Color *= texture(u_Textures[24], v_TexCoord * v_TilingFactor); break;
		case 25: o_Color *= texture(u_Textures[25], v_TexCoord * v_TilingFactor); break;
		case 26: o_Color *= texture(u_Textures[26], v_TexCoord * v_TilingFactor); break;
		case 27: o_Color *= texture(u_Textures[27], v_TexCoord * v_TilingFactor); break;
		case 28: o_Color *= texture(u_Textures[28], v_TexCoord * v_TilingFactor); break;
		case 29: o_Color *= texture(u_Textures[29], v_TexCoord * v_TilingFactor); break;
		case 30: o_Color *= texture(u_Textures[30], v_TexCoord * v_TilingFactor); break;
		case 31: o_Color *= texture(u_Textures[31], v_TexCoord * v_TilingFactor); break;
	}
}