#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexColor;

uniform mat4 mvp;

out vec2 uv;
out vec3 color;

void main() {
    uv = vertexTexCoord;
    color = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1);
}
