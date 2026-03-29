local EditorRender = Rule {
    categories = {"Transform"},
    fields = {
        templateDrawable = {
            shader = Asset.Shader("./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs"),
            texture = Asset.Texture("./resources/Textures/EditorTexture.png"),
            model = Asset.Model("./resources/Models/FlatTexture.obj")
        },

        drawOutline = true,
        lastFrameInput = false,
        currentInput = false
    }
}

function EditorRender:Update(_entityData)
    local currentFrameInput = IO.GetKeyDown('o')

    if(currentFrameInput == nil) then
        Log.Message("nil")
    else
        if(currentFrameInput) then
        else
        end
    end

    self.fields.currentInput = currentFrameInput
end

function EditorRender:Draw(_entityData)
    Core.Camera:Draw(_entityData)

    if(self.fields.currentInput and not self.fields.lastFrameInput) then
        self.fields.drawOutline = not self.fields.drawOutline
    end

    self.fields.lastFrameInput = self.fields.currentInput

    if(self.fields.drawOutline) then
        local _entityLayer = _entityData["Transform"].Layer
        _entityData["Transform"].Layer = Core.Camera:getPosition().z - 0.5

        local _model = self.fields.templateDrawable.model
        local _shader = self.fields.templateDrawable.shader
        local _texture = self.fields.templateDrawable.texture

        Core.Camera:Draw(_entityData["Transform"], _model, _texture, _shader)

        _entityData.Transform.Layer = _entityLayer
    end
end

return EditorRender