--- Controle the GLFW window using lua script

WindowModes = {FULLSCREEN = 0, WINDOWED = 1, BORDERLESS_WINDOWED = 3}

Window = {}

Window.getWidth = function ()
    return _GetWindowWidth();
end

Window.getHeight = function ()
    return _GetWindowHeight();
end

Window.setTitle = function (title)
    _SetWindowTitle(title);
end

Window.setMode = function (m)
    _SetWindowMode(m);
end

Window.setOpacity = function (opacity)
    _SetWindowOpacity(opacity);
end

Window.setReziable = function (resizable)
    _SetResizable(resizable);
end
