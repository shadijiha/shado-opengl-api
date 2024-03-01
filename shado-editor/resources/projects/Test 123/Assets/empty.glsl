// This is an example of a shader file
// Both vertex and fragment shaders are in the same file
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
	vec4 position;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat float v_TexIndex;
layout (location = 5) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;

	v_TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

	Output.position = gl_Position;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
	vec4 position;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat float v_TexIndex;
layout (location = 5) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

uniform float u_Dt;
uniform vec4 u_Color;

#define hsl2rgb(h,s,l) l - s * min(l, 1.0 - l) * kHSx.www * clamp(kHSx.xxx - abs(fract(h + kHSx.xyz) * 4.0 - 2.0), -1.0, 1.0)
const vec4 kHSx = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);

void main()
{
	vec2 uv = Input.position.xy;
	//o_Color = vec4(Input.position.xyz * 0.5 + 0.5, 1.0);
	o_Color = u_Color; //vec4(hsl2rgb(u_Dt * 0.1, uv.x, uv.y), 1.0);
	o_EntityID = v_EntityID;
}