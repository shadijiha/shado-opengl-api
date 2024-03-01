﻿using System;
using System.IO;
using UI = Shado.Editor.UI;

namespace Shado
{
    public class Shader
    {
        internal IntPtr native;
        public string filepath { get; private set; }
        internal Shader(string filepath) {
            this.filepath = filepath;
            InternalCalls.Shader_CreateShader(filepath, out native);
        }

        ~Shader() {
            InternalCalls.Shader_Destroy(native);
        }

        public void SetInt(string name, int value) {
            InternalCalls.Shader_SetInt(native, name, ref value);
        }
        
        public void SetIntArray(string name, int[] values) {
            InternalCalls.Shader_SetIntArray(native, name, values);
        }
        public void SetFloat(string name, float value) {
            InternalCalls.Shader_SetFloat(native, name, ref value);
        }
        public void SetFloat3(string name, Vector3 value) {
            InternalCalls.Shader_SetFloat3(native, name, ref value);
        }
        public void SetFloat4(string name, Vector4 value) {
            InternalCalls.Shader_SetFloat4(native, name, ref value);
        }

        public static Shader Create(string filepath) { 
            return new Shader(filepath);
        }

        public void Reset(string filepath) {
            InternalCalls.Shader_Reset(native, filepath, out native);
        }
    }

    [Editor.EditorTargetType(typeof(Shader))]
    public class ShaderEditor : Editor.Editor
    {
        static string[] shaderExtension = {
            ".shader", ".glsl"
        };
        protected override void OnEditorDraw()
        {
            if (target is null)
                return;

            Shader shader = (Shader)target;
            UI.Separator();
            UI.InputTextFileChoose(fieldName, shader.filepath, shaderExtension, path => {
                shader.Reset(path);
            });

            if (UI.Button("+")) {
                string path = UI.OpenFileDialog(shaderExtension, UI.FileChooserType.Save);
                if (path != null)
                    GenerateFile(path);                
            }
            if (UI.Button("Recompile"))
                shader.Reset(shader.filepath);
            UI.Separator();
        }

        private void GenerateFile(string path) {
            using (StreamWriter writer = new StreamWriter(path)) {
                writer.WriteLine(@"
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

void main()
{
	vec4 texColor = Input.Color;

    texColor *= texture(u_Textures[int(v_TexIndex)], Input.TexCoord * Input.TilingFactor);

	o_Color = texColor;
	//o_Color.a = Input.Color.a;
	o_EntityID = v_EntityID;
}
                ");
                writer.Close();
            }
        }
    }
}
