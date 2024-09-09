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
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TexIndex;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;
	v_TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat float v_TexIndex;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

uniform float u_Time;
#define time u_Time*10.


float stripe(vec2 uv) {
    return cos(uv.x*20.-time+uv.y*-30.);
}

float glass(vec2 uv) {
    return cos(dot(uv.xy, vec2(12.41234,2442.123))*cos(uv.y));
}

void main()
{
	vec4 texColor = Input.Color;
	vec2 fragCoord = gl_FragCoord.xy;
	vec2 iResolution = vec2(1280, 720);

	vec2 uv = fragCoord.xy / iResolution.xy;
    float a = iResolution.x/iResolution.y;
    uv.x *= a;
    

    float g = stripe(uv);

    
    vec3 col = vec3(smoothstep(0., .2, g));

    col.r = .8;
    col /= (pow(glass(vec2(uv.x*30., uv.y)),2.))+.5;
    
  
    //Mask sides
    col *= smoothstep(.12, .0, abs(uv.x - .5*a));

    //Mask top and bottom
    col *= smoothstep(.33, .30, abs(uv.y - .5));

    if (uv.y > .80 && uv.y < .94 || uv.y < .2 && uv.y >.06) {
       col = vec3(smoothstep(.13, .0, abs(uv.x - .5*a)));
    
    }

    if (uv.y > .77 && uv.y < .87 || uv.y < .23 && uv.y >.13) {
       col = vec3(smoothstep(.15, .0, abs(uv.x - .5*a)));
        
    }

	texColor = vec4(col,1.0);
	texColor *= texture(u_Textures[int(v_TexIndex)], Input.TexCoord * Input.TilingFactor);
	o_Color = texColor;
	//o_Color.a = Input.Color.a;
	o_EntityID = v_EntityID;
}
