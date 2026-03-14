local categoryName = "Transform"
local baseAttributes = {
    position = Vector3.new(0, 0, 0),
    rotation = Vector3.new(0),
    scale = Vector3.new(1)
}

return Category.new(categoryName, baseAttributes)