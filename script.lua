require("player")
require("stone")
require("text")

function script()
    local window = engine.create_window("Game", 800, 500, "./img/icon.bmp")

    if window then
        local framebuffer = engine.create_framebuffer(window)
        local font        = engine.load_font         ("./cmunrm.ttf")
        local text        = text:create              (font, "Hello, world!")
        local texture     = engine.load_texture      ("./img/stone.bmp")
        local stones      = create_stones            ()
        local player      = player:create            ()
        local shader      = engine.create_shader     ("./glsl/vertex_shader.glsl", "./glsl/fragment_shader.glsl")
        local FPS         = 60

        player:set_scale          (0.25, 0.25)
        player:set_position       (0.0, 0.0, 0.0)
        player:set_rotate         (0.0)
        player:set_speed          (0.01)
        player:set_animation_speed(8.0)

        while not engine.window_should_close(window) do
            local frame_start_time = os.clock()

            engine.clear_color(0.5, 0.5, 1.0)

            text:draw    (window, shader)
            player:update(window, os.clock(), 0)
            player:draw  (window, shader)

            draw_stones(window, stones, shader, texture)

            engine.swap_buffers(window)
            engine.poll_events ()

            local frame_end_time = os.clock()
            local wait_time      = (1.0 / FPS) - (frame_end_time - frame_start_time)

            if wait_time > 0 then engine.delay(wait_time) end
        end

        player:kill()

        destroy_stones(stones)

        engine.delete_font       (font)
        engine.delete_texture    (text)
        engine.delete_shader     (shader)
        engine.delete_framebuffer(framebuffer)
        engine.delete_window     (window)
    end
end
