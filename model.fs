#version 330 core
layout(location = 0) out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform vec3 viewPos;

void main()
{
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(texture_specular1, TexCoords).rgb;

    // ambient lighting
    vec3 ambient = light.ambient * diffuseColor;

    // diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;

    // specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * specularColor;

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}
