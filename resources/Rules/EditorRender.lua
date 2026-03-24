local EditorRender = Rule.new()
EditorRender.name = "Editor Renderer"
EditorRender.categories = {"Transform"}
EditorRender.attributes = {
    --Template drawable
    templateDrawable = {
        shaderPath = "./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs",
        shader = Asset.Shader("./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs"),
        texturePath = "./resources/Textures/NoDrawable.png",
        texture = Asset.Texture("./resources/Textures/NoDrawable.png"),
        modelPath = "./resources/Models/FlatTexture.obj",
        model = Asset.Model("./resources/Models/FlatTexture.obj")
    },

    drawOutline = true,
    lastFrameInput = false,
    currentInput = false
}

function EditorRender:Update(_entityData)
    --Log.Message("Getting frame input")

    local currentFrameInput = IO.GetKeyDown('o')

    if(currentFrameInput == nil) then
        Log.Message("nil")
    else
        if(currentFrameInput) then
            --Log.Message("Current frame true")
        else
            --Log.Message("Current frame false")
        end
    end

    self.attributes.currentInput = currentFrameInput
end

function EditorRender:Draw(_entityData)
    if(_entityData["Drawable"] ~= nil) then
        Core.Camera:Draw(_entityData["Transform"], _entityData["Drawable"])
    end

    if(self.attributes.currentInputmeInput and not self.attributes.lastFrameInput) then
        self.attributes.drawOutline = not self.attributes.drawOutline
    end

    self.attributes.lastFrameInput = self.attributes.currentInput

    if(self.attributes.drawOutline) then
        -- Draw with scale outline
        Core.Camera:Draw(_entityData["Transform"], self.attributes.templateDrawable)
    end
end

return EditorRender