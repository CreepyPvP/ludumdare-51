#version 330

in vec2 uv;
in vec3 color;

uniform float seconds;

uniform int type;
uniform int modifier;

out vec4 out_Color;

float circle(vec2 pos, vec2 center, float radius)
{
    float dist = length(pos - center);
    return abs(dist - radius);
}

float line(vec2 pos, vec2 a, vec2 b, float r )
{
    vec2 pa = pos - a, ba = b - a;
    float h = clamp(dot(pa,ba) / dot(ba,ba), 0.0, 1.0 );
    return abs(length(pa - ba * h) - r);
}

vec2 distort_pos(vec2 pos)
{
    pos.x += cos(seconds * 10 + pos.x * 7.5 + pos.y * 3) * 0.04;
    pos.y += sin(seconds * 10 + pos.y * 7.5 + pos.x * 3) * 0.04;
    return pos;
}

void main()
{
    vec2 pos = 2 * uv - vec2(1);
    float d = 0;
    // NOTE: From 1 to 0

    //circle
    if(true) {
        d = circle(pos, vec2(0, 0), 0.8);
        // d = min(d, line(pos, vec2(-0.8, 0), vec2(0.8, 0), 0));
    }

    // triangle - light unit
    if(false) {
        float a = line(pos, vec2(-0.5, 0), vec2(0.5, 0), 0);
        float b = line(pos, vec2(0.5, 0), vec2(0, 0.5), 0);
        float c = line(pos, vec2(-0.5, 0), vec2(0, 0.5), 0);

        d = min(a, b);
        d = min(d, c);
    }

    // rectangle
    if(false) {
        float a = line(pos, vec2(-0.5, 0), vec2(0.5, 0), 0);
        float b = line(pos, vec2(0.5, 0), vec2(0.5, 0.5), 0);
        float c = line(pos, vec2(-0.5, 0.5), vec2(0.5, 0.5), 0);
        float e = line(pos, vec2(-0.5, 0), vec2(-0.5, 0.5), 0);

        d = min(a, b);
        d = min(d, c);
        d = min(d, e);
    }

    float thickness = 0.06;
    float intensity = thickness / d;
    intensity = intensity * intensity * intensity;
    out_Color = vec4(color * intensity, intensity);
    // out_Color += 0.5 * vec4(1, 1, 1, 1);
}
