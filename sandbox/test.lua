-- This is a sample file for a scriptable object
-- Only use void function calls 
-- 
-- @author Shadi Jiha
-- @Date 24 Oct. 2021
require "shado_opengl_api"

local entities = {};

function OnCreate()
    for i = 0, 9, 3 do
        local entity = Entity:new({width = 2, height = 2});
        entity:setTexture("assets/riven.png");
        entity:setPosition(i, 7.0);

        table.insert(entities, entity);
    end
end

function OnUpdate(dt)
    --print(entities["1"]);
end

function OnDestroy()
    
end