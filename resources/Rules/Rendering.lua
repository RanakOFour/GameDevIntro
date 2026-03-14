Rendering = Rule.new()
Rendering.name = "Rendering"
Rendering.categories = {"Transform", "Drawable"}

function Rendering:Update(_entityData)
    local message = "Deltatime: " .. Core.DeltaTime()

    if(Core.DeltaTime() == nil) then
        Log.Message("Core.DeltaTime is nil")
    else
        Log.Message("Core.DeltaTime is not nil")
    end

    Log.Message(message)
end

function Rendering:Draw(_entityData)
    local transform = _entityData["Transform"]
    local drawable = _entityData["Drawable"]

    if(Core.Camera == nil) then
        Log.Message("Camera is nil")
    end

    Core.Camera:Draw(transform, drawable)
    
    Log.Message("Entity drawn")
end

return Rendering