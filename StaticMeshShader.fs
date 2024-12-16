#version 330

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 Position0;

out vec4 FragColor;

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLight
{
    BaseLight Base;
    vec3 Direction;
};

uniform DirectionalLight gDirectionalLight;
uniform vec3 gCameraPos;
uniform sampler2D textureDiffuse;


vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    LightDirection = normalize(LightDirection);  // Add normalization here
    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;
    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
        DiffuseColor = vec4(Light.Color, 1.0f) *
                       Light.DiffuseIntensity *
                       DiffuseFactor; 

        vec3 PixelToCamera = normalize(gCameraPos - Position0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect); 

        SpecularColor = vec4(Light.Color, 1.0f) * SpecularFactor;
    }

    return AmbientColor + DiffuseColor + SpecularColor;
}


vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);
}

void main()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);

    FragColor = texture(textureDiffuse, TexCoord0.xy) * TotalLight;
}