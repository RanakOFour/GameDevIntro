local Test = Rule {
    categories = {"TestCategory"}
}

function Test:Update(_entityData)
    --Log.Message("Test message")

    local msgY = "Nil"
    local msgN = "Not Nil"

    local catData = _entityData["TestCategory"]

    if(catData == nil) then
        Log.Message(msgY)
    else
        Log.Message(msgN)
        Log.Table(catData)
    end

    catData.position.x = catData.position.x + 1

    local message = "Data: " .. catData.position:ToString()
    Log.Message(message)
end

return Test