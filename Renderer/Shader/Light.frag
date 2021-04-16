#version 430 core

out vec4 FragColor;

uniform vec3 emmisive;

void main(void)
{ 
    FragColor = vec4(emmisive, 1.0); 
}