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
	vec4 WorldPosition;
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
	Output.WorldPosition = vec4(a_Position, 1.0);
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
	vec4 WorldPosition;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat float v_TexIndex;
layout (location = 5) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

uniform float u_Time;
uniform vec2 u_ScreenResolution;
uniform vec2 u_MousePos;
uniform vec4 u_Colour;
uniform float u_Alpha;

vec3 palette(float d){
	return mix(vec3(0.2,0.7,0.9),vec3(1.,0.,1.),d);
}

vec2 rotate(vec2 p,float a){
	float c = cos(a);
    float s = sin(a);
    return p*mat2(c,s,-s,c);
}

float map(vec3 p){
    for( int i = 0; i<8; ++i){
        float t = u_Time*0.2;
        p.xz =rotate(p.xz,t);
        p.xy =rotate(p.xy,t*1.89);
        p.xz = abs(p.xz);
        p.xz-=.5;
	}
	return dot(sign(p),p)/5.;
}

vec4 rm (vec3 ro, vec3 rd){
    float t = 0.;
    vec3 col = vec3(0.);
    float d;
    for(float i =0.; i<64.; i++){
		vec3 p = ro + rd*t;
        d = map(p)*.5;
        if(d<0.02){
            break;
        }
        if(d>100.){
        	break;
        }
        //col+=vec3(0.6,0.8,0.8)/(400.*(d));
        col+=palette(length(p)*.1)/(400.*(d));
        t+=d;
    }
    return vec4(col,1./(d*100.));
}

void main()
{
	vec4 texColor = Input.Color;
	// TexIndex between 0 and 31
	texColor *= texture(u_Textures[int(v_TexIndex)], Input.TexCoord * Input.TilingFactor);	

	vec2 fragCoord = gl_FragCoord.xy - Input.WorldPosition.xy * 5;
	vec2 iResolution = vec2(800, 800);
	float iTime = u_Time;

    vec2 uv = (fragCoord-(iResolution.xy/2.))/iResolution.x;

	vec3 ro = vec3(0.,0.,-50.);
    ro.xz = rotate(ro.xz,iTime);
    vec3 cf = normalize(-ro);
    vec3 cs = normalize(cross(cf,vec3(0.,1.,0.)));
    vec3 cu = normalize(cross(cf,cs));
    
    vec3 uuv = ro+cf*3. + uv.x*cs + uv.y*cu;
    
    vec3 rd = normalize(uuv-ro);
    
    vec4 col = texColor * rm(ro,rd);

	o_Color = col * u_Colour;
	o_Color.a = u_Alpha; //Input.Color.a;
	o_EntityID = v_EntityID;
}
