#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;

uniform vec3 lightDir;

void main()
{
    vec3 norm = vec3(0.0f,1.0f,0.0f);
    float dotProduct = dot(normalize(lightDir),norm);
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    //night
    if(dotProduct<0.001)
    {
	color = 0.01*texture(skybox, textureCoordinates) + 0.9*vec4(0.0f,0.0f,0.0f,0.0f) + 0.09*fogColor;
    }
    else
    {
	color = dotProduct*texture(skybox, textureCoordinates) + (1-dotProduct)*vec4(0.0f,0.0f,0.0f,0.0f) + 0.3*fogColor;
    }

    
    //color = texture(skybox, textureCoordinates);

}
