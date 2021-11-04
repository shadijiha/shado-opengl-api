-- This is a sample file for a scriptable object
-- Only use void function calls 
-- 
-- @author Shadi Jiha
-- @Date 24 Oct. 2021

function OnCreate()
    entity = _CreateEntity(_scene);
    --_SetEntityTexture(entity, "assets/riven3.png");

    _SetColor(entity, 1.0, 0, 1, 1.0);
    _SetPosition(entity, 10.0, 1.5);
end

function OnUpdate(dt)
    
end

function OnDestroy()
    
end