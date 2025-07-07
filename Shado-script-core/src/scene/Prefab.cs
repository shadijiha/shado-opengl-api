using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
    /// <summary>
    /// This struct stores the ID of a Prefab 
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Prefab
    {
        internal readonly ulong id;

        /// <summary>
        /// Instantiates a prefab and adds it to the runtime scene
        /// </summary>
        /// <param name="position">The position to where the prefab will be moved to after instantiation</param>
        /// <returns>The instantiated prefab entity object</returns>
        /// <exception cref="InvalidPrefabIdException">throws an exception if the prefab was not instantiated. Could be due to an invalid prefab ID</exception>
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

    /// <summary>
    /// Exception throwns when attempting to instantiate an invalid prefab ID
    /// </summary>
    public class InvalidPrefabIdException : Exception
    {
        public InvalidPrefabIdException(Prefab prefab) : base("Invalid prefab id " + prefab.id) { }
    }
}