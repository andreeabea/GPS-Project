#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;

void main()
{
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    color = 0.7*texture(skybox, textureCoordinates) + 0.3*fogColor;
    //night
    //color = 0.01*texture(skybox, textureCoordinates) + 0.99*vec4(0.0f,0.0f,0.0f,0.0f);
    
    //color = texture(skybox, textureCoordinates);

}
