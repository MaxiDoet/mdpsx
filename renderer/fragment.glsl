#version 330 core

in vec2 f_uv;
in vec3 f_color;

out vec3 color;

uniform sampler2D ourTexture;

void main() {
    color = f_color;
    //color = texture(ourTexture, f_uv).rgb;
    //color = texture(ourTexture, f_uv).rgb;
}