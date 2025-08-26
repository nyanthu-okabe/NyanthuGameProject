return {
    {
        input = { "stick" },
        output = "torch",
        onUse = function(player, world)
            entity = world:loaded_entity
            entity:damage(10)
        end
    }
}
