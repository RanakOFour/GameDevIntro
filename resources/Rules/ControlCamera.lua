local NewRule = Rule.new()
NewRule.name = "ControlCamera"
NewRule.categories = {"Transform"}
NewRule.data = {
    position = Vector3.new(0, 0, 3),
    rotation = 0
}

function NewRule:Update(_entityData)
    local position = self.data.position

    if(IO.GetKeyDown('w')) then
        position.x = position.x + 0.05
        Log.Message("Moved forward")
    end

    if(IO.GetKeyDown('s')) then
        position.x = position.x - 0.05
        Log.Message("Moved back")
    end

    if(IO.GetKeyDown('a')) then
        position.z = position.z + 0.05
        Log.Message("Moved left")
    end

    if(IO.GetKeyDown('d')) then
        position.z = position.z - 0.05
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
    
    Core.MoveCamera(position)

    local message = "CamPos: " .. position.ToString()

    Log.Message(message)
end

return NewRule