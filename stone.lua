stone = {}

function stone:create()
    local o = {}

    setmetatable(o, {__index = stone})

    o.mesh = engine.create_mesh()

    o.tex_coords = {
        u  = 0.0,
        v  = 0.0,
        du = 0.125,
        dv = 0.125
    }

    return o
end

function stone:set_scale(h, w)
    engine.set_scale(self.mesh, w, h)
end

function stone:set_rotate(angle)
    engine.set_rotate(self.mesh, angle)
end

function stone:set_position(x, y, z)
    engine.set_position(self.mesh, x, y, z)
end

function stone:draw(window, shader, texture)
    local u  = self.tex_coords.u
    local v  = self.tex_coords.v
    local du = self.tex_coords.du
    local dv = self.tex_coords.dv

    engine.draw(self.mesh, window, shader, texture, u, v, du, dv)
end

function stone:destroy()
    engine.delete_mesh(self.mesh)
end

function create_stones()
    local stones = {}

    for i = -5, 5 do
        for j = -3, 3 do
            local new_stone = stone:create()

            new_stone:set_position(i * 0.25, j * 0.25, -0.1)
            new_stone:set_scale   (0.25, 0.25)
            new_stone:set_rotate  (0.0)

            table.insert(stones, new_stone)
        end
    end

    return stones
end

function draw_stones(window, stones, shader, texture)
    for _, s in ipairs(stones) do
        s:draw(window, shader, texture)
    end
end

function destroy_stones(stones)
    for _, s in ipairs(stones) do
        s:destroy()
    end
end
