#version 330 core
layout(location = 0) out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

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

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight spotLight;

float shininess = 1.0;

uniform vec3 viewPos;

void CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // combine results
    ambient  = light.ambient;
    diffuse  = light.diffuse  * diff;
    specular = light.specular * spec;
}

void CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
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

void CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    ambient = vec3(0.0);
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    ambient  = light.ambient;
    if(theta > light.outerCutOff)
    {
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        // specular shading
        float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        // attenuation
        float distance    = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance +
                                 light.quadratic * (distance * distance));
        // combine results
        diffuse  = light.diffuse  * diff;
        specular = light.specular * spec;
    } else {
        diffuse = vec3(0.0);
        specular = vec3(0.0);
    }

    // we'll leave ambient unaffected so we always have a little light.
    diffuse  *= intensity;
    specular *= intensity;
}

void main()
{
    // normal mapping
    // obtain normal from normal map in range [0,1]
        vec3 norm = texture(texture_normal1, TexCoords).rgb;
        // transform normal vector to range [-1,1]
        norm = normalize(norm * 2.0 - 1.0);
        norm = normalize(TBN * norm);

    // properties
    //vec3 norm = normalize(Normal);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 total_ambient, total_diffuse, total_specular;
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(texture_specular1, TexCoords).rgb;

    // phase 1: Directional lighting
    CalcDirLight(dirLight, norm, viewDir, total_ambient, total_diffuse, total_specular);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        vec3 ambient, diffuse, specular;
        //CalcPointLight(pointLights[i], norm, FragPos, viewDir, ambient, diffuse, specular);
        total_ambient += ambient;
        total_diffuse += diffuse;
        total_specular += specular;
    }
    // phase 3: Spot light
    vec3 ambient, diffuse, specular;
    CalcSpotLight(spotLight, norm, FragPos, viewDir, ambient, diffuse, specular);
    total_ambient += ambient;
    total_diffuse += diffuse;
    total_specular += specular;

    vec3 result = (total_ambient + total_diffuse) * diffuseColor + total_specular * specularColor;
    FragColor = vec4(result, 1.0);
}
