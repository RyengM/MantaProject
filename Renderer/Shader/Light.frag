uniform vec3 emmisive;

void main(void)
{ 
    gl_FragColor = vec4(emmisive, 1.0); 
}