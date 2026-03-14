local NewRule = Rule.new()
NewRule.name = "RotateGuy"
NewRule.categories = {"Transform", "Drawable"}

function NewRule:Update(_entityData)
    local rotation = _entityData["Transform"].rotation
    rotation.x = rotation.x + 0.02

    Log.Message("Rotated guy")
end

return NewRule