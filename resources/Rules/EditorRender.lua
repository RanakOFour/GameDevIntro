local EditorRender = Rule.new()
EditorRender.name = "Editor Renderer"
EditorRender.categories = {"Transform"}
EditorRender.attributes = {
    --Template drawable
    templateDrawable = {
        shaderPath = "./resources/Shaders/default/frag.fs;./resources/Shaders/default/vert.vs",
        shader = nil,
        texturePath = "./resources/Textures/NoDrawable.jpg",
        texture = nil,
        modelPath = "./resources/Models/FlatTexture.obj",
        model = nil
    }
}

function EditorRender:Draw(_entityData)
    if(_entityData["Drawable"] == nil) then
        Core.Camera:Draw(_entityData["Transform"], self.attributes.templateDrawable)
    else
        Core.Camera:Draw(_entityData["Transform"], _entityData["Drawable"])
    end
end

return EditorRender