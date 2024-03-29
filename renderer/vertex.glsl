#version 330 core

layout(location = 0) in ivec2 v_pos;
layout(location = 1) in ivec2 v_uv;
layout(location = 2) in vec3 v_color;

out vec2 f_uv;
out vec3 f_color;

void main() {
    float x = (float(v_pos.x) / 512) - 1.0;
    float y = 1.0 - (float(v_pos.y) / 256);

    gl_Position.xyzw = vec4(x, y, 0.0, 1.0);

    f_uv = v_uv;
    f_color = v_color;
}