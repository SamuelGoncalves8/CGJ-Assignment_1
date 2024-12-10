#version 330 core

in vec3 exPosition;
in vec2 exTexcoord;
in vec3 exNormal;

out vec4 FragmentColor;

uniform vec3 meshColor;

void main(void)
{
    vec3 N = normalize(exNormal);
    vec3 color;

    vec3 colorVariation = N * 0.1 + 0.6;
    color = meshColor * colorVariation;
    FragmentColor = vec4(color, 1.0);
}
