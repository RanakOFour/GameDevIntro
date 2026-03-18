local NewRule = Rule.new()
NewRule.name = "CameraSchenanigans"
NewRule.categories = {"Transform", "Drawable"}
NewRule.attributes = {
    audio = Asset.Audio("./resources/Audio/collect.wav"),
    orthoSize = Vector2.new(5.0, 5.0)
}

function NewRule:Init(_entityData)
    Log.Message("Started camera schenanigans")
    _entityData["Drawable"].texturePath = "./resources/Textures/triangle.png"
    _entityData["Drawable"].shaderPath = "./resources/Shaders/infinitegrid/frag.fs;./resources/Shaders/infinitegrid/vert.vs"
end

function NewRule:Update(_entityData)
    local rotation = _entityData["Transform"].rotation
    local orthoSize = self.attributes.orthoSize
    local guyScale = _entityData["Transform"].scale

    if(IO.GetKeyDown('p')) then
        IO.PlayAudio(self.attributes.audio, false)
    end

    if(IO.GetKeyDown('o')) then
        Core.Camera:setCameraSize(Vector2.new(1920.0, 1080.0))
        Core.Camera:setPerspective()
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
        Core.Camera:setCameraSize(self.attributes.orthoSize)
        Core.Camera:setOrthographic()
    end
end

return NewRule