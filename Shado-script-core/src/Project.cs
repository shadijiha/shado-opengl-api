namespace Shado;

public readonly struct Project
{
    public static Project current => new();
    
    /// <summary>
    /// Returns the absolute path of the Asset directory of the loaded project
    /// </summary>
    public string? assetDirectory {
        get {
            unsafe
            {
                string projectAssetDir = InternalCalls.Project_GetAssetDirectory();
                return projectAssetDir == "" ? null : projectAssetDir;
            }            
        }
    }

    /// <summary>
    /// Returns the absolute path of the loaded project (where the project solution is located)
    /// </summary>
    public string? projectDirectory {
        get {
            unsafe {
                string projectDir = InternalCalls.Project_GetProjectDirectory();
                return projectDir == "" ? null : projectDir;
            }
        }
    }

    /// <summary>
    /// Returns the path relative to the project directory
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    public string GetRelativePath(string path) {
        unsafe {
            return InternalCalls.Project_GetRelativePath(path);
        }
    }
}