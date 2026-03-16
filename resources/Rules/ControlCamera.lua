local NewRule = Rule.new()
NewRule.name = "ControlCamera"
NewRule.categories = {"Transform"}

function NewRule:Init(_entityData)
    Log.Message("Started controlling camera")
end

function NewRule:Update(_entityData)
    local position = Core.Camera:getPosition()

    if(IO.GetKeyDown('w')) then
        position.z = position.z - 0.05
        Log.Message("Moved forward")
    end

    if(IO.GetKeyDown('s')) then
        position.z = position.z + 0.05
        Log.Message("Moved back")
    end

    if(IO.GetKeyDown('a')) then
        position.x = position.x - 0.05
        Log.Message("Moved left")
    end

    if(IO.GetKeyDown('d')) then
        position.x = position.x + 0.05
        Log.Message("Moved right")
    end

    if(IO.GetKeyDown('q')) then
        position.y = position.y + 0.05
        Log.Message("Moved up")
    end

    if(IO.GetKeyDown('e')) then
        position.y = position.y - 0.05
        Log.Message("Moved down")
    end

    Core.Camera:setPosition(position)

    local message = "CamPos: " .. position:ToString()

    Log.Message(message)
end

return NewRule