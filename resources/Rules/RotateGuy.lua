local RotateGuy = Rule {
    categories = {"Transform", "Drawable"},
    fields = {
        audio = Asset.Audio("./resources/Audio/collect.wav"),
        orthoSize = Vector2.new(5.0, 5.0)
    }
}

function RotateGuy:Init(_entityData)
    _entityData["Drawable"].texturePath = "./resources/Textures/triangle.png"
    _entityData["Drawable"].modelPath = "./resources/Models/FlatTexture.obj"
end

function RotateGuy:Update(_entityData)
    local orthoSize = self.fields.orthoSize
    local guyScale = _entityData["Transform"].scale

    if(IO.GetKeyDown('p')) then
        IO.PlayAudio(self.fields.audio, false)
    end

    if(IO.GetKeyDown('k')) then
        orthoSize.x = orthoSize.x + 0.1
        orthoSize.y = orthoSize.y + 0.1
        Core.Camera:setCameraSize(orthoSize)
        Log.Message("Changed ortho size to: " .. orthoSize:ToString())
    end

    if(IO.GetKeyDown('l')) then
        orthoSize.x = orthoSize.x - 0.1
        orthoSize.y = orthoSize.y - 0.1
        Core.Camera:setCameraSize(orthoSize)
        Log.Message("Changed ortho size to: " .. orthoSize:ToString())
    end

    if(IO.GetKeyDown('h')) then
        guyScale.x = guyScale.x + 0.1
        guyScale.y = guyScale.y + 0.1
        Log.Message("Changed guy size to: " .. guyScale:ToString())
    end

    if(IO.GetKeyDown('j')) then
        guyScale.x = guyScale.x - 0.1
        guyScale.y = guyScale.y - 0.1
        Log.Message("Changed guy size to: " .. guyScale:ToString())
    end

    if(IO.GetKeyDown('i')) then
        Core.Camera:setCameraSize(self.fields.orthoSize)
        Core.Camera:setOrthographic()
    end

    if(IO.GetKeyDown('o')) then
        Core.Camera:setCameraSize(Vector2.new(1920.0, 1080.0))
        Core.Camera:setPerspective()
    end
end

return RotateGuy