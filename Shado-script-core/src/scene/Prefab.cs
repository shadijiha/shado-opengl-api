using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Prefab
    {
        internal readonly ulong id;

        public static implicit operator bool(Prefab prefab) => prefab.id != 0;
    }

    public static class PrefabExt
    {
        public static Entity Instantiate(this Prefab prefab, Vector3 position) {
            ulong entityId = 0; //InternalCalls.PrefabExt_Instantiate(prefab.id, position);
            if (entityId == 0)
                throw new InvalidPrefabIdException(prefab);
            return new Entity(entityId);
        }
    }

    public class InvalidPrefabIdException : Exception
    {
        public InvalidPrefabIdException(Prefab prefab) : base("Invalid prefab id " + prefab.id) { }
    }
}