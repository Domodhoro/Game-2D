text = {}

function text:create(font, message)
    local o = {}

    setmetatable(o, {__index = text})

    o.mesh = engine.create_mesh()
    o.text = engine.create_text(font, message)

    engine.set_scale   (o.mesh, 1.0, 0.25)
    engine.set_rotate  (o.mesh, 0.0)
    engine.set_position(o.mesh, -0.75, 0.5, -0.1)

    return o
end

function text:draw(window, shader)
    engine.draw(self.mesh, window, shader, self.text, 0.0, 0.0, 1.0, 1.0)
end
