using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public class Texture2D
    {
        public string Path { get; private set; }
        public readonly uint Width;
        public readonly uint Height;
        public readonly uint DataFormat;
        public readonly uint InternalFormat;
        public readonly bool IsLoaded;

        public readonly IntPtr Native;

        public Texture2D(string path) {
            Path = path;
            CreateTexture2D_Native(path, out Native,
                out Width, out Height,
                out DataFormat, out InternalFormat,
                out IsLoaded
            );
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CreateTexture2D_Native(
            string path, out IntPtr ptr,
            out uint width, out uint height,
            out uint dataFormat, out uint internalFormat,
            out bool loaded
           );
    }
}
