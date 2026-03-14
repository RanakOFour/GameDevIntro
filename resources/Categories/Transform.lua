local categoryName = "Transform"
local baseAttributes = {
    position = Vector3.new(0, 0, 0),
    rotation = Vector3.new(),
    scale = Vector3.new(1.0)
}

return Category.new(categoryName, baseAttributes)