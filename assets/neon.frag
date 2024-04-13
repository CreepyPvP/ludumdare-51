#version 330

in vec2 uv;
in vec3 color;

out vec4 out_Color;

void main() 
{
    out_Color = vec4(uv, 0, 1);
    // out_Color = vec4(color, 1);
}
