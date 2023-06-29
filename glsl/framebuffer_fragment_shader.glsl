#version 330 core

uniform sampler2D Sampler;

out vec4 FragColor;
in  vec2 UV;

void main() {
    vec4 Color = texture(Sampler, UV);

    if (Color.r == 1.0f && Color.g != 1.0f && Color.b == 1.0f) {
        discard;
    }

    FragColor = Color;
}

