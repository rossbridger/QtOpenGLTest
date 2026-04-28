#version 330 core
layout(location = 0) out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

float shininess = 1.0;

uniform vec3 viewPos;

void CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    ambient  = light.ambient;
    diffuse  = light.diffuse  * diff;
    specular = light.specular * spec;
}

void CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
    // combine results
    ambient  = light.ambient;
    diffuse  = light.diffuse  * diff;
    specular = light.specular * spec;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
}

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 total_ambient, total_diffuse, total_specular;
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(texture_specular1, TexCoords).rgb;

    // phase 1: Directional lighting
    CalcDirLight(dirLight, norm, viewDir, total_ambient, total_diffuse, total_specular);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        vec3 ambient, diffuse, specular;
        CalcPointLight(pointLights[i], norm, FragPos, viewDir, ambient, diffuse, specular);
        total_ambient += ambient;
        total_diffuse += diffuse;
        total_specular += specular;
    }
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    vec3 result = (total_ambient + total_diffuse) * diffuseColor + total_specular * specularColor;
    FragColor = vec4(result, 1.0);
}
