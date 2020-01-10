#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform vec3 lightColor2;
uniform	vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform mat4 view;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 64.0f;

uniform samplerCube skybox;

vec3 reflection;
vec3 colorFromSkybox;

float constant = 1.0f;
float linear = 0.045f;
float quadratic = 0.0075f;

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

vec3 computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
		
        float shadow = computeShadow();

    	//modulate with diffuse map
    	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
    	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
    	//modulate woth specular map
    	specular *= vec3(texture(specularTexture, fragTexCoords));

	//modulate with shadow
    	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
   
    	return color;
}

// calculates the color when using a point light.
vec3 computePointLight(vec3 lightPos, vec3 lightColor)
{
    vec3 lightPosEye = vec3(view * vec4(lightPos,1.0)); 

	//compute distance to light
	float dist = length(lightPosEye -fragPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	//att = 1.0f;
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPosEye - fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	ambient = att * ambientStrength * lightColor;
	ambient *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	diffuse *= vec3(texture(diffuseTexture,fragTexCoords));
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	//compute specular light
	float specCoeff = pow(max(dot(viewDirN, halfVector), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * lightColor;
	specular *= vec3(texture(specularTexture,fragTexCoords));
	
	vec3 color = min((ambient + diffuse) + specular, 1.0f);
	
	return color;
}

float computeFog()
{
	float fogDensity = 0.003f;
	float fragmentDistance = length(fragPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

// to calculate the color when using a spot light.

void main() 
{
	vec4 colorFromTexture = texture(diffuseTexture, fragTexCoords);
	if(colorFromTexture.a < 0.1) 
		discard;

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	vec3 color1 = computeLightComponents();
	vec3 color2 = computePointLight(lightPos, lightColor2);
	vec3 color3 = computePointLight(lightPos2, lightColor2);

	//vec3 color =color1;
	vec3 color = color2 + 0.3*color1 + color3;
	
	//night
	//vec3 color = color2 + color3;

	fColor = fogColor*(1-fogFactor) + vec4(color * fogFactor, 1.0f);

        //fColor = vec4(color, 1.0f);
    //fColor = vec4(o, 1.0f);
 
    //fColor += texture(diffuseTexture, fragTexCoords);
}