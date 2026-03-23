local categoryName = "Drawable"
local baseAttributes = {
    shaderPath = "",
    shader = nil,
    texturePath = "",
    texture = nil,
    modelPath = "",
    model = Asset.Model("./resources/Models/curuthers.obj")
}

return Category.new(categoryName, baseAttributes)