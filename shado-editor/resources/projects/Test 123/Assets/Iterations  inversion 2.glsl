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

#define AA 2
vec3 shape( in vec2 uv )
{
	float time = u_Time*0.05  + 47.0;
    
	vec2 z = -1.0 + 2.0*uv;
	z *= 1.5;
    
    vec3 col = vec3(1.0);
	for( int j=0; j<48; j++ )
	{
        float s = float(j)/16.0;
        float f = 0.2*(0.5 + 1.0*fract(sin(s*20.0)));

		vec2 c = 0.5*vec2( cos(f*time+17.0*s),sin(f*time+19.0*s) );
		z -= c;
		float zr = length( z );
	    float ar = atan( z.y, z.x ) + zr*0.6;
	    z  = vec2( cos(ar), sin(ar) )/zr;
		z += c;

        // color		
        col -= 0.5*exp( -10.0*dot(z,z) )* (0.25+0.4*sin( 5.5 + 1.5*s + vec3(1.6,0.8,0.5) ));
	}
        
    return col;
}

void main()
{
	vec2 iResolution = vec2(1280, 720);
	float e = 1.0/iResolution.x;
	vec2 fragCoord = Input.TexCoord*iResolution.xy;

    vec3 tot = vec3(0.0);
    for( int m=0; m<AA; m++ )
    for( int n=0; n<AA; n++ )
    {        
        vec2 uv = (fragCoord+vec2(m,n)/float(AA))/iResolution.xy;
	    vec3 col = shape( uv );
        float f = dot(col,vec3(0.333));
        vec3 nor = normalize( vec3( dot(shape(uv+vec2(e,0.0)),vec3(0.333))-f, 
                                    dot(shape(uv+vec2(0.0,e)),vec3(0.333))-f, 
                                    e ) );
        col += 0.2*vec3(1.0,0.9,0.5)*dot(nor,vec3(0.8,0.4,0.2));;
	    col += 0.3*nor.z;
        tot += col;
    }
    tot /= float(AA*AA);

    tot = pow( clamp(tot,0.0,1.0), vec3(0.8,1.1,1.3) );
	
    vec2 uv = fragCoord/iResolution.xy;
    tot *= 0.4 + 0.6*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 );

	vec4 texColor = vec4( tot, 1.0 );

	texColor *= texture(u_Textures[int(v_TexIndex)], Input.TexCoord * Input.TilingFactor);

	o_Color = texColor;
	//o_Color.a = Input.Color.a;
	o_EntityID = v_EntityID;
}
