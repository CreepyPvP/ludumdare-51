#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

varying vec2 uv;
varying vec3 color;

uniform float seconds;
uniform float distortion_amp;
uniform int entity_type;

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
    pos.x += cos(seconds * 10.0 + pos.x * 7.5 + pos.y * 3.0) * 0.04 * distortion_amp;
    pos.y += sin(seconds * 10.0 + pos.y * 7.5 + pos.x * 3.0) * 0.04 * distortion_amp;
    return pos;
}

void main()
{
    vec2 pos = distort_pos(2.0 * uv - vec2(1.0));
    float d = 0.0;
    // NOTE: From 1 to 0

    //circle
    if (entity_type == 0) {
        d = circle(pos, vec2(0.0, 0.0), 0.5);
        // d = min(d, line(pos, vec2(-0.8, 0), vec2(0.8, 0), 0));
    }

    // triangle - light unit
    if (entity_type == 1) {
        float a = line(pos, vec2(-0.5, -0.5), vec2(0.5, -0.5), 0.0);
        float b = line(pos, vec2(0.5, -0.5), vec2(0.0, 0.5), 0.0);
        float c = line(pos, vec2(-0.5, -0.5), vec2(0.0, 0.5), 0.0);

        d = min(a, b);
        d = min(d, c);
    }

    // rectangle
    // if(false)
    // {
    //     float a = line(pos, vec2(-0.5, 0.0), vec2(0.5, 0.0), 0.0);
    //     float b = line(pos, vec2(0.5, 0.0), vec2(0.5, 0.5), 0.0);
    //     float c = line(pos, vec2(-0.5, 0.5), vec2(0.5, 0.5), 0.0);
    //     float e = line(pos, vec2(-0.5, 0.0), vec2(-0.5, 0.5), 0.0);
    //
    //     d = min(a, b);
    //     d = min(d, c);
    //     d = min(d, e);
    // }

    // square
    if (entity_type == 2)
    {
        float a = line(pos, vec2(-0.5, -0.5), vec2(0.5, -0.5), 0.0);
        float b = line(pos, vec2(0.5, -0.5), vec2(0.5, 0.5), 0.0);
        float c = line(pos, vec2(-0.5, -0.5), vec2(-0.5, 0.5), 0.0);
        float e = line(pos, vec2(-0.5, 0.5), vec2(0.5, 0.5), 0.0);

        d = min(a, b);
        d = min(d, c);
        d = min(d, e);
    }

    // cross
    if (entity_type == 3)
    {

        float a = line(pos, vec2(-0.25, -0.5), vec2(0.25, -0.5), 0.0);
        float b = line(pos, vec2(0.25, -0.5), vec2(0.25, -0.25), 0.0);
        float c = line(pos, vec2(0.25, -0.25), vec2(0.5, -0.25), 0.0);
        float e = line(pos, vec2(0.5, -0.25), vec2(0.5, 0.25), 0.0);
        float f = line(pos, vec2(0.25, 0.25), vec2(0.5, 0.25), 0.0);
        float g = line(pos, vec2(0.25, 0.25), vec2(0.25, 0.5), 0.0);
        float h = line(pos, vec2(-0.25, 0.5), vec2(0.25, 0.5), 0.0);
        float i = line(pos, vec2(-0.25, 0.25), vec2(-0.25, 0.5), 0.0);
        float j = line(pos, vec2(-0.25, 0.25), vec2(-0.5, 0.25), 0.0);
        float k = line(pos, vec2(-0.5, 0.25), vec2(-0.5, -0.25), 0.0);
        float l = line(pos, vec2(-0.25, -0.25), vec2(-0.5, -0.25), 0.0);
        float m = line(pos, vec2(-0.25, -0.25), vec2(-0.25, -0.5), 0.0);


        float p = line(pos, vec2(-0.25, 0.5), vec2(0.25, 0.5), 0.0);

        d = min(a, b);
        d = min(d, c);
        d = min(d, e);
        d = min(d, f);
        d = min(d, g);
        d = min(d, h);
        d = min(d, i);
        d = min(d, j);
        d = min(d, k);
        d = min(d, l);
        d = min(d, m);
    }

    // pentagram, hail satan
    if (entity_type == 7) {
        float outline = circle(pos, vec2(0.0, 0.0), 0.5);

        float a = line(pos, vec2(0.0, 0.5), vec2(-0.588 * 0.5, -0.809 * 0.5), 0.0);
        float b = line(pos, vec2(0.0, 0.5), vec2(0.588 * 0.5, -0.809 * 0.5), 0.0);
        float c = line(pos, vec2(0.951 * 0.5, 0.309 * 0.5), vec2(-0.951 * 0.5, 0.309 * 0.5), 0.0);
        float e = line(pos, vec2(0.951 * 0.5, 0.309 * 0.5), vec2(-0.588 * 0.5, -0.809 * 0.5), 0.0);
        float f = line(pos, vec2(-0.951 * 0.5, 0.309 * 0.5), vec2(0.588 * 0.5, -0.809 * 0.5), 0.0);

        d = min(outline, a);
        d = min(d, b);
        d = min(d, c);
        d = min(d, e);
        d = min(d, f);
    }

    float thickness = 0.06;
    float intensity = thickness / d;
    intensity = intensity * intensity * intensity;
    gl_FragColor = vec4(color * intensity, intensity);
}
