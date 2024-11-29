#version 330 core

in vec4 inPosition;
in vec4 inColor;
out vec4 exColor;

uniform mat4 Matrix;

void main(void) {
    gl_Position = Matrix * inPosition;
    exColor = inColor;
}
