using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public struct Scene
    {
        public static Entity[] GetAllEntities()
        {
            return InternalCalls.Scene_GetAllEntities();
        }
    }
}
