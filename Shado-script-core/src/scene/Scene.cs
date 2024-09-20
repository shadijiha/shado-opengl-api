using System;
using System.Runtime.CompilerServices;

namespace Shado
{
    public struct Scene
    {
        public static Entity[] GetAllEntities() {
            unsafe {
                ulong[] entities = InternalCalls.Scene_GetAllEntities();
                Entity[] result = new Entity[entities.Length];
                for (int i = 0; i < entities.Length; i++) {
                    result[i] = new Entity(entities[i]);
                }

                return result;
            }
        }

        /// <summary>
        /// Changes the current scene to the scene with the given name.
        /// It will recursively search for the scene in the project directory.
        /// </summary>
        /// <param name="sceneName">The name of the scene</param>
        /// <returns>The full path of the loaded scene</returns>
        public static string LoadScene(string sceneName) {
            if (string.IsNullOrEmpty(sceneName)) {
                throw new ArgumentException("Scene name cannot be null or empty");
            }

            if (!sceneName.EndsWith(".shadoscene"))
                sceneName += ".shadoscene";

            unsafe {
                return InternalCalls.Scene_LoadScene(sceneName);
            }
        }
    }
}