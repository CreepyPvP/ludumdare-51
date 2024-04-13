#version 330

in vec2 uv;
in vec3 color;

out vec4 out_Color;

void main() 
{
    // NOTE: From 1 to 0
    float d = length(2 * uv - vec2(1));

    float thickness = 0.07;
    out_Color = vec4(max(thickness * color / d - thickness * color, vec3(0)), 1);
}
