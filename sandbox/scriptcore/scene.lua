--- Controle the Scene and the renderer
Renderer = {}
SceneUtils = {}

---Changes the Renderer Clear colour 
---@param r number red
---@param g number green
---@param b number blue
---@param a number alpha
Renderer.setClearColor = function (r, g, b, a)
    _SetClearColor(r, g, b, a);
end

---Get a C++ scene by name
---@param name any
---@return table
SceneUtils.getByName = function (name)
    local temp = _GetSceneByName(name);
    if temp == nil then
        print("Unknown scene name -->" .. name);
        return nil;
    end
    return Scene(temp);
end

---Gets the application's current active scene
---@return table Scene a scene object
SceneUtils.getActive = function ()
    return Scene(_GetActiveScene());
end

--- Created a scene object
---@param scenePtr Scene* a pointer to a C++ Scene
---@return table Scene a scene object
Scene = class(function (this, scenePtr)
    this.scene = scenePtr;
end)

--- Sets the caller scene as the application active scene
function Scene:setActive()
    _SetActiveScene(self.scene);
end
