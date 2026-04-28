#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depth = LinearizeDepth(texture(depthTexture, TexCoords).r) / far_plane;

    FragColor = texture(screenTexture, TexCoords);
    //FragColor.rgb *= depth;
}
