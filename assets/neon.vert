#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexColor;

uniform mat4 mvp;

varying vec2 uv;
varying vec3 color;

void main() {
    uv = vertexTexCoord;
    color = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1);
}
