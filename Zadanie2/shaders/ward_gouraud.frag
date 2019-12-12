#version 330

in vec3 color; // kolor interpolowany z shadera wierzcholkow
out vec4 fColor; // wyjsciowy kolor fragmentu
 
void main()
{
    fColor = vec4(color, 1.0);
}