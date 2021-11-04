-- This file defines the standard Shado OpenGL api lua classes
-- Only use void function calls 
-- 
-- @author Shadi Jiha
-- @Date 24 Oct. 2021

Entity = {entity = nil}

function Entity:new(options)
    o = {};
    setmetatable(o, self);
    self.__index = self;

    options = options or {width = 1, height = 1}

    self.entity = _CreateEntity(_scene, options["width"], options["height"]);
    return o;
end

function Entity:setTexture(path)
    _SetEntityTexture(self.entity, path)
end

function Entity:setTillingFactor(factor)
    _SetEntityTillingFactor(self.entity, factor)
end

function Entity:setColor(r, g, b, a)
    _SetEntityTillingFactor(self.entity, r, g, b, a)
end

function Entity:setPosition(x, y)
    _SetEntityPosition(self.entity, x, y)
end

function Entity:setType(type)
    _SetEntityType(self.entity, type)
end