--- Controle the GLFW window using lua script

WindowModes = {FULLSCREEN = 0, WINDOWED = 1, BORDERLESS_WINDOWED = 3}

function GetWindowWidth()
    return _GetWindowWidth();
end

function GetWindowHeight()
    return _GetWindowHeight();
end

function SetWindowTitle(title)
    _SetWindowTitle(title);
end

function SetWindowMode(mode)
    _SetWindowMode(mode);
end

function SetWindowOpacity(opacity)
    _SetWindowOpacity(opacity);
end

function SetResizable(resizable)
    _SetResizable(resizable)
end


