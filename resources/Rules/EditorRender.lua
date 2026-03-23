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