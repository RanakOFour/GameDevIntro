local NewRule = Rule.new()
NewRule.name = "RotateGuy"
NewRule.categories = {"Transform", "Drawable"}
NewRule.data = {
    rotate = false
}

function NewRule:Update(_entityData)
    local rotation = _entityData["Transform"].rotation

    --if(IO.GetKeyDown('r'))then
      --  self.data.rotate = not self.data.rotate
    --end

    if(self.data.rotate == false) then
        return
    end

    if(IO.GetKeyDown('w')) then
        rotation.z = rotation.z - 0.05
        Log.Message("Moved forward")
    end

    if(IO.GetKeyDown('s')) then
        rotation.z = rotation.z + 0.05
        Log.Message("Moved back")
    end

    if(IO.GetKeyDown('a')) then
        rotation.x = rotation.x - 0.05
        Log.Message("Moved left")
    end

    if(IO.GetKeyDown('d')) then
        rotation.x = rotation.x + 0.05
        Log.Message("Moved right")
    end

    if(IO.GetKeyDown('q')) then
        rotation.y = rotation.y + 0.05
        Log.Message("Moved up")
    end

    if(IO.GetKeyDown('e')) then
        rotation.y = rotation.y - 0.05
        Log.Message("Moved down")
    end

    Log.Message("Rotated guy")
end

return NewRule