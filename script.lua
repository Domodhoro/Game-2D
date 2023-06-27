local player      = {}
local stone       = {}
local text        = {}
local framebuffer = {}

function player:create()
    local o = {}

    setmetatable(o, {__index = player})

    o.mesh            = engine.create_mesh ()
    o.texture         = engine.load_texture("./img/player.bmp")
    o.animation_speed = 1.0
    o.speed           = 0.1

    o.position = {
        x = 0.0,
        y = 0.0,
        z = 0.0
    }

    o.tex_coords = {
        u  = 0.0,
        v  = 0.0,
        du = 0.0625,
        dv = 0.0625
    }

    return o
end

function player:delete()
    engine.delete_texture(self.texture)
    engine.delete_mesh   (self.mesh)
end

function player:set_scale(h, w)
    engine.set_scale(self.mesh, w, h)
end

function player:set_rotate(angle)
    engine.set_rotate(self.mesh, angle)
end

function player:set_position(x, y, z)
    self.position.x = x
    self.position.y = y
    self.position.z = z
end

function player:set_speed(speed)
    self.speed = speed
end

function player:set_animation_speed(animation_speed)
    self.animation_speed = animation_speed
end

function player:update(window, clock, ID)
    self.tex_coords.u = ID

    if (engine.get_key(window, KEY_W)) then
        self.position.y   = self.position.y + self.speed
        self.tex_coords.u = (math.floor(self.animation_speed * clock) % 4) + ID
        self.tex_coords.v = 1.0
    end

    if (engine.get_key(window, KEY_S)) then
        self.position.y   = self.position.y - self.speed
        self.tex_coords.u = (math.floor(self.animation_speed * clock) % 4) + ID
        self.tex_coords.v = 0.0
    end

    if (engine.get_key(window, KEY_A)) then
        self.position.x   = self.position.x - self.speed
        self.tex_coords.u = (math.floor(self.animation_speed * clock) % 4) + ID
        self.tex_coords.v = 3.0
    end

    if (engine.get_key(window, KEY_D)) then
        self.position.x   = self.position.x + self.speed
        self.tex_coords.u = (math.floor(self.animation_speed * clock) % 4) + ID
        self.tex_coords.v = 2.0
    end

    engine.set_position(self.mesh, self.position.x, self.position.y, self.position.z)
end

function player:draw(window, shader)
    local u  = self.tex_coords.u
    local v  = self.tex_coords.v
    local du = self.tex_coords.du
    local dv = self.tex_coords.dv

    engine.draw(self.mesh, window, shader, self.texture, u, v, du, dv)
end

function text:create(font, message)
    local o = {}

    setmetatable(o, {__index = text})

    o.mesh = engine.create_mesh()
    o.text = engine.create_text(font, message)

    engine.set_scale   (o.mesh, 1.0, 0.25)
    engine.set_rotate  (o.mesh, 0.0)
    engine.set_position(o.mesh, 0.0, 0.0, 0.0)

    return o
end

function text:delete()
    engine.delete_mesh   (self.mesh)
    engine.delete_texture(self.text)
end

function text:set_position(x, y, z)
    engine.set_position(self.mesh, x, y, z)
end

function text:draw(window, shader)
    engine.draw(self.mesh, window, shader, self.text, 0.0, 0.0, 1.0, 1.0)
end

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

function delete_stones(stones)
    for _, s in ipairs(stones) do
        s:destroy()
    end
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

            new_stone:set_scale   (0.25, 0.25)
            new_stone:set_rotate  (0.0)
            new_stone:set_position(i * 0.25, j * 0.25, -0.1)

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

function framebuffer:create(window)
    local o = {}

    setmetatable(o, {__index = framebuffer})

    o.framebuffer = engine.create_framebuffer(window)
    o.mesh        = engine.create_mesh       ()

    engine.set_scale   (o.mesh, 1.0, 1.0)
    engine.set_rotate  (o.mesh, 0.0)
    engine.set_position(o.mesh, 0.0, 0.0, 0.0)

    return o
end

function framebuffer:draw(window, shader)
    engine.draw(self.mesh, window, shader, self.framebuffer, 0.0, 0.0, 1.0, 1.0)
end

function script()
    local window = engine.create_window("Game", 800, 500, "./img/icon.bmp")

    if window then
        local framebuffer = framebuffer:create       (window)
        local font        = engine.load_font         ("./cmunrm.ttf")
        local text        = text:create              (font, "Hello, world!")
        local texture     = engine.load_texture      ("./img/stone.bmp")
        local stones      = create_stones            ()
        local player      = player:create            ()
        local shader      = engine.create_shader     ("./glsl/vertex_shader.glsl", "./glsl/fragment_shader.glsl")
        local FPS         = 60

        text:set_position         (-0.75, 0.5, -0.1)
        player:set_scale          (0.25, 0.25)
        player:set_position       (0.0, 0.0, 0.0)
        player:set_rotate         (0.0)
        player:set_speed          (0.01)
        player:set_animation_speed(8.0)

        while not engine.window_should_close(window) do
            local frame_start_time = os.clock()

            if engine.get_key(window, KEY_ESC) then
                engine.set_window_should_close(window)
            end

            engine.clear_color(0.5, 0.5, 1.0)
            --engine.enable_framebuffer(framebuffer)

            player:update(window, os.clock(), 0)
            player:draw  (window, shader)
            text:draw    (window, shader)

            draw_stones(window, stones, shader, texture)

            --engine.disable_framebuffer(framebuffer)

            framebuffer:draw(window, shader)

            engine.swap_buffers(window)
            engine.poll_events ()

            local frame_end_time = os.clock()
            local wait_time      = (1.0 / FPS) - (frame_end_time - frame_start_time)

            if wait_time > 0 then engine.delay(wait_time) end
        end

        delete_stones            (stones)
        player:delete            ()
        text:delete              (window, shader)
        engine.delete_shader     (shader)
        engine.delete_framebuffer(framebuffer)
        engine.delete_window     (window)
    end
end
