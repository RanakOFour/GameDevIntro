Rendering = Rule.new()
Rendering.name = "Rendering"
Rendering.categories = {"Transform", "Drawable"}
Rendering.data = {
    Camera = nil
}

function Rendering:Init(_entityData)
    Camera = Core.GetCamera()
end

function Rendering:Update(_entityData)
    local message = "" + Core.DeltaTime()

    if(Core.DeltaTime() == nil) then
        Log.Message("Core.DeltaTime is nil")
    end

    Log.Message(message)
end

function Rendering:Draw(_entityData)
    local transform = _entityData["Transform"]
    local drawable = _entityData["Drawable"]

    if(Camera == nil) then
        Log.Message("Camera is nil")
    end

    Core.Draw(transform, drawable)
    
    Log.Message("Entity drawn")
end

return Rendering