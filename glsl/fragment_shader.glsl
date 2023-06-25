#version 330 core

uniform sampler2D Sampler;
uniform vec4 TexCoords;

out vec4 FragColor;
in  vec2 UV;

void main() {
    float u    = (UV.x + TexCoords.x) * TexCoords.z;
    float v    = (UV.y + TexCoords.y) * TexCoords.w;
    vec4 Color = texture(Sampler, vec2(u, v));

    if (Color.r == 1.0f && Color.g != 1.0f && Color.b == 1.0f) discard;

    FragColor = Color;
}
