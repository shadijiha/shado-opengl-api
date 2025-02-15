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

        public Entity Instantiate(Vector3 position) {
            unsafe {
                ulong entityId = InternalCalls.Prefab_Instantiate(id, position);
                if (entityId == 0)
                    throw new InvalidPrefabIdException(this);
                return new Entity(entityId);
            }
        }

        public static implicit operator bool(Prefab prefab) => prefab.id != 0;
    }

    public class InvalidPrefabIdException : Exception
    {
        public InvalidPrefabIdException(Prefab prefab) : base("Invalid prefab id " + prefab.id) { }
    }
}