NewRule = Rule.new()
NewRule.name = "TestRule"
NewRule.categories = {"TestCategory"}

function NewRule:Update(_entityData)
    Log.Message("Test message")
    
    local catData = _entityData["My Category"]
    catData["position"].x = catData["position"].x + 1

    local message = "Data: (" .. catData["position"].x .. ", " .. catData["position"].y .. ")"
    Log.Message(message)
end

function NewRule:Draw(_entityData)
    
end

return NewRule