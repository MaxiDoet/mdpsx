#version 330 core

in ivec2 v_pos;
in uvec3 v_color;

out vec3 color;

void main() {
    float x = (float(v_pos.x) / 512) - 1.0;

    float y = 1.0 - (float(v_pos.y) / 256);

    gl_Position.xyzw = vec4(x, y, 0.0, 1.0);

    color = vec3(float(v_color.r) / 255,
                 float(v_color.g) / 255,
                 float(v_color.b) / 255);
}