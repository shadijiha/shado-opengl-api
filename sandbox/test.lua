-- This is a sample file for a scriptable object
-- Only use void function calls 
-- 
-- @author Shadi Jiha
-- @Date 24 Oct. 2021
require "shado_opengl_api"

local function map(x, in_min, in_max, out_min, out_max)
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

function OnCreate()
    
    -- Define ground
    local ground = Entity:new({width = 40.0, height = 0.3});
    ground:setType("static");
    ground:setPosition(0, -0.75);

    for i = 0, 9, 3 do
        local entity = Entity:new({width = 2, height = 2});
        entity:setTexture("assets/riven.png");
        entity:setPosition(i, 7.0);
    end
end

function OnUpdate(dt)
    -- if IsKeyDown(KeyCodes.KEY_W) then
    --     print("W is pressed");
    -- end
end

function OnDestroy()
    
end

function OnEvent(e)
    if IsMousePressedEvent(e) then
        if e.button == MouseCodes.MOUSE_BUTTON_LEFT then
            local mapX = map(GetMouseX(), 0, GetWindowWidth(), -10, 10);
            local mapY = map(GetMouseY(), 0, GetWindowHeight(), -10, 10);

            local entity = Entity:new({width = 0.5, height = 0.5});
            entity:setPosition(mapX, -mapY);
            entity:setColor(0.6, 0.3, 0.7, 1);
        end
    end

    if IsKeyPressedEvent(e) then
        if (e.keyCode) == KeyCodes.KEY_ESCAPE then
            SetWindowMode(WindowModes.WINDOWED);
        elseif e.keyCode == KeyCodes.KEY_F then
            SetWindowMode(WindowModes.FULLSCREEN);
        end
    end
end