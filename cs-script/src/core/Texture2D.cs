using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    /**
     * NOTE: When you pass a Texture2D object to a SpriteRendererComponent,
     * then a COPY of that texture is made by calling
     * [C++] CreateRef<Texture2D>(texture->getFilePath())
     * 
     * So this texture object is only usefull to get information
     * about Path, width, height, etc...
     * 
     * (No refrence to this object is used in the C++ side)
     * 
     * Try to avoid creating this class in OnUpdate
     */
    public class Texture2D
    {
        public string Path { get; private set; }
        public readonly uint Width;
        public readonly uint Height;
        public readonly uint DataFormat;
        public readonly uint InternalFormat;
        public readonly bool IsLoaded;

        //public readonly IntPtr Native;

        public Texture2D(string path) {
            Path = path;
            CreateTexture2D_Native(path,
                out Width, out Height,
                out DataFormat, out InternalFormat,
                out IsLoaded
            );
        }

        internal Texture2D() { }

        ~Texture2D() {
            //DestroyTexture2D_Native(Native);
        }

        public override string ToString()
        {
            string s = "Texture2D {\n";

            foreach (var prop in GetType().GetProperties())
            {
                s += string.Format("\t{0} = {1},\n", prop.Name, prop.GetValue(this));
            }

            foreach (var prop in GetType().GetFields())
            {
                s += string.Format("\t{0} = {1},\n", prop.Name, prop.GetValue(this));
            }

            return s + "}";
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateTexture2D_Native(
            string path,
            out uint width, out uint height,
            out uint dataFormat, out uint internalFormat,
            out bool loaded
           );

        //[MethodImpl(MethodImplOptions.InternalCall)]
        //private static extern void DestroyTexture2D_Native(IntPtr native);
    }
}
