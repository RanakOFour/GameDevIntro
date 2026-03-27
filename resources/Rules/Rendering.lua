local Rendering = Rule {
    categories = {"Transform", "Drawable"}
}

function Rendering:Update(_entityData)
    
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