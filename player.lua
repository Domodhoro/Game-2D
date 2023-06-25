player = {}

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

function player:kill()
    engine.delete_texture(self.texture)
    engine.delete_mesh   (self.mesh)
end
