#version 330 core
layout(location = 0) out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D texture_albedo1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_ao1;

uniform sampler2D BRDFIntegrationMap;
uniform samplerCube environmentMap;

struct DirLight {
    vec3 direction;
    vec3 color;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;
    vec3 color;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;
    vec3 color;
};
uniform SpotLight spotLight;

const float PI = 3.14159265359;

uniform vec3 viewPos;

// helper function for Cook-Torrence BRDF.
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CookTorrenceBrdf(vec3 N, vec3 L, vec3 V, vec3 F0, vec3 albedo, vec3 radiance, float roughness, float metallic)
{
    // cook-torrance brdf
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    vec3 lightDir = normalize(-light.direction);
    return CookTorrenceBrdf(normal, lightDir, viewDir, F0, albedo, light.color, roughness, metallic);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.color * attenuation;

    return CookTorrenceBrdf(normal, lightDir, viewDir, F0, albedo, radiance, roughness, metallic);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    vec3 radiance = vec3(0.0);
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    if(theta > light.outerCutOff)
    {
        radiance  = light.color;
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (distance * distance);
        radiance = light.color * attenuation;
    }
    return CookTorrenceBrdf(normal, lightDir, viewDir, F0, albedo, radiance * intensity, roughness, metallic);
}

void main()
{
    // normal mapping
    // obtain normal from normal map in range [0,1]
    vec3 albedo = texture(texture_albedo1, TexCoords).rgb;
    vec3 normal = texture(texture_normal1, TexCoords).rgb;
    float roughness = texture(texture_roughness1, TexCoords).r;
    float metallic = texture(texture_metallic1, TexCoords).r;
    float ao = texture(texture_ao1, TexCoords).r;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(TBN * normal);

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // phase 1: Directional lighting
    Lo += CalcDirLight(dirLight, normal, viewDir, F0, albedo, roughness, metallic);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        vec3 ambient, diffuse, specular;
        //CalcPointLight(pointLights[i], norm, FragPos, viewDir, ambient, diffuse, specular);
        Lo += CalcPointLight(pointLights[i], normal, FragPos, viewDir, F0, albedo, roughness, metallic);
    }
    // phase 3: Spot light
    Lo += CalcSpotLight(spotLight, normal, FragPos, viewDir, F0, albedo, roughness, metallic);

    vec3 R = reflect(-viewDir, normal);
    vec3 F = FresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(environmentMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(BRDFIntegrationMap, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    vec3 ambient = specular * ao;

    vec3 color = ambient + Lo;

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
