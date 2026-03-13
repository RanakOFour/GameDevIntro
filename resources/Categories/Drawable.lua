local categoryName = "Drawable"
local baseAttributes = {
    shaderPath = "./resources/shaders/default/vert.vs;./resources/shaders/default/frag.fs",
    shader = nil,
    texturePath = "",
    texture = nil,
    modelPath = "",
    model = nil
}

return Category.new(categoryName, baseAttributes)