-- This is a sample file for a scriptable object
-- Only use void function calls 
-- 
-- @author Shadi Jiha
-- @Date 24 Oct. 2021
require "scriptcore/shado_opengl_api"

local scene2 = SceneUtils.getByName("Test scene2");

local function map(x, in_min, in_max, out_min, out_max)
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

local entities = {}

function OnCreate()
    -- Define ground
    local ground = Entity({width = 40.0, height = 0.3});
    ground:setType("static");
    ground:setPosition(0, -0.75);

    for i = 0, 1, 1 do
        local entity = Entity({width = 2, height = 2});
        entity:setTexture("assets/riven.png");
        entity:setPosition(0, 7.0);
        entities[i + 1] = entity;
    end

    local j = 2;
    for y = -5.0, 5.0, 0.5 do
        for x = -0.5, 5.0, 0.5 do
            local entity = Entity();
            entity:setColor((x + 0.5) / 10, 0.4, (y + 5.0)/10, 1.0);
            entity:setPosition(x, y);
            entities[j] = entity;   
            j = j + 1;       
        end
    end
end

function OnUpdate(dt)

end

function OnDestroy() 
    for index, value in ipairs(entities) do
        value:destroy();
    end   
end

function OnEvent(e)
    
    if IsMousePressedEvent(e) then
        if e.button == MouseCodes.MOUSE_BUTTON_LEFT then
            local mapX = map(Input.getMouseX(), 0, Window.getWidth(), -10, 10);
            local mapY = map(Input.getMouseY(), 0, Window.getHeight(), -10, 10);

            local e = Entity({width = 0.5, height = 0.5});
            e:setPosition(mapX, -mapY);
            e:setColor(0.6, 0.3, 0.7, 1);
        end
    end

    if IsKeyPressedEvent(e) then
        if (e.keyCode) == KeyCodes.KEY_ESCAPE then
            Window.setMode(WindowModes.WINDOWED);
        elseif e.keyCode == KeyCodes.KEY_F then
            Window.setMode(WindowModes.FULLSCREEN);

        elseif e.keyCode == KeyCodes.KEY_B then
            scene2:setActive();
        end
    end
end