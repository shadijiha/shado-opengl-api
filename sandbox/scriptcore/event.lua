--- This is event utility functions

function IsMouseMovedEvent(event)
    return event.name == "MouseMoved"
end

function IsMousePressedEvent(event)
    return event.name == "MouseButtonPressed"
end

function IsMouseReleasedEvent(event)
    return event.name == "MouseReleasedPressed"
end

function IsKeyPressedEvent(event)
    return event.name == "KeyPressed"
end