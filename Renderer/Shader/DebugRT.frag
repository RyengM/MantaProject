#version 430 core
out vec4 FragColor;

in vec2 texC;

uniform sampler2D rt;

void main()
{
    float color = texture(rt, vec2(texC.x, 1.0 - texC.y)).r;
    FragColor = vec4(vec3(color), 1.0);
}