#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 Texture;

uniform mat4 Model;
uniform mat4 Projection;
out vec2     UV;

void main() {
    gl_Position = Projection * Model * vec4(Position, 0.0f, 1.0f);
    UV          = Texture;
}

