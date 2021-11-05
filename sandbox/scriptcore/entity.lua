---
--- This file has definitions for a Shado OpenGL API Lua script
---

Entity = {entity = nil, scene = nil, is_alive = true}

function Entity:new(options)
    o = {};
    setmetatable(o, self);
    self.__index = self;

    options = options or {width = 1, height = 1}

    self.entity = _CreateEntity(_scene, options["width"], options["height"]);
    self.scene = _scene;
    return o;
end

function Entity:setTexture(path)
    if self.is_alive then
        _SetEntityTexture(self.entity, path)
    end
end

function Entity:setTillingFactor(factor)
    if self.is_alive then
        _SetEntityTillingFactor(self.entity, factor)
    end
end

function Entity:setColor(r, g, b, a)
    if self.is_alive then
        _SetEntityColor(self.entity, r, g, b, a)
    end   
end

function Entity:setPosition(x, y)
    if self.is_alive then
        _SetEntityPosition(self.entity, x, y)
    end
end

function Entity:setType(type)
    if self.is_alive then
        _SetEntityType(self.entity, type)
    end
end

function Entity:destroy()
    -- if self.is_alive and _DestroyEntity(self.entity, self.scene) then
    --     self.is_alive = false;
    --     self.entity = nil;
    -- end
end