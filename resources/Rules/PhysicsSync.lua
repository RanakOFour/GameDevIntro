local PhysicsSync = Rule {
    categories = { "Transform", "PhysicsBody" },
    fields = {}
}

function PhysicsSync:Init(_entityData)
    local transform = _entityData["Transform"]
    local phys      = _entityData["PhysicsBody"]

    -- Create the rigid body at the entity's current transform position
    local body = Physics.CreateBody(transform.Position, phys.bodyType)

    -- Take half-extents from Transform.Scale so the physics shape matches
    local halfW = transform.Scale.x
    local halfH = transform.Scale.y

    -- Attach a box collider
    Physics.AddBoxShape(body, halfW, halfH,
                        phys.density, phys.friction, phys.restitution)

    -- Store the live body handle in the per-entity data (hidden from editor)
    phys._body = body

    Log.Message("PhysicsSync: created body at " .. transform.Position:ToString()
                .. " half-extents (" .. tostring(halfW) .. ", " .. tostring(halfH) .. ")")
end

function PhysicsSync:Update(_entityData)
    local transform = _entityData["Transform"]
    local phys      = _entityData["PhysicsBody"]

    -- Sync simulated position and rotation back to the Transform component
    if phys._body ~= nil and phys._body:IsValid() then
        transform.Position = phys._body:GetPosition()
        transform.Rotation = phys._body:GetAngle()
    end
end

return PhysicsSync
