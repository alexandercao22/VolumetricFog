RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> positionGBuffer : register(t2);
Texture2D<float4> normalGBuffer : register(t3);
Texture2D<float4> diffuseGBuffer : register(t4);
Texture2D<float4> ambientGBuffer : register(t5);
Texture2D<float4> specularGBuffer : register(t6);

struct SpotLightBuffer
{
    matrix vpMatrix;
    float3 colour;
    float3 direction;
    float outAngle;
    float inAngle;
    float3 position;
    float range;
};

struct DirectionalLightBuffer
{
    matrix vpMatrix;
    float3 colour;
    float3 direction;
};

StructuredBuffer<SpotLightBuffer> spotLights : register(t0);
Texture2DArray<float> spotShadowMaps : register(t1);
sampler shadowMapSampler : register(s0);
StructuredBuffer<DirectionalLightBuffer> directionalLight : register(t7);
Texture2DArray<float> dirShadowMaps : register(t8);

#define SHADOW_EPSILON 0.0001f

cbuffer constantBuffer : register(b1)
{
    float4 camPos;
    uint totalSpotLights;
    float fullLight;
    float shadows;
}

float3 addSpotLight(SpotLightBuffer light, float3 position, float3 colour, float3 normal, float3 specular)
{
    float3 toLight = normalize(light.position - position);
    float3 toEye = camPos.xyz - position;
    
    // Phong diffuse
    float NDotL = saturate(dot(toLight, normalize(normal))); // Normal dot light
    float3 finalColour = light.colour * NDotL;
    
    // Blinn specular
    toEye = normalize(toEye);
    float3 halfVec = normalize(toEye + toLight);
    float NDotH = saturate(dot(halfVec, normal)); // Cosine the angle between normal vector and half vector
    float specExp = 10.0f; // Get from obj
    finalColour += finalColour * pow(NDotH, specExp) * specular;
    
    // Cone attenuation
    float dotCone = -dot(toLight, normalize(light.direction));
    float coneAngle = acos(dotCone); // The angle from direction vector to toLight vector.
    if (coneAngle < light.inAngle) // If inside inner cone, fully lit
    {
        return colour * finalColour;
    }
    else if (coneAngle < light.outAngle) // If outside innercone but inside outer cone, attenuate
    {
        float coneAttenuation = (coneAngle - light.inAngle) / (light.outAngle - light.inAngle);
        coneAttenuation = -(coneAttenuation - 1.0f);
        return colour * finalColour * coneAttenuation;
    }
    return float3(0.0f, 0.0f, 0.0f); // Outside light cone
}

float3 addDirectionalLight(DirectionalLightBuffer light, float3 position, float3 colour, float3 normal, float3 specular)
{
    float3 lightDirection = normalize(-light.direction);
    
    // Phong diffuse
    float NDotD = saturate(dot(lightDirection, normal)); // Normal dot direction
    float3 finalColour = light.colour * NDotD;
    
    // Blinn specular
    float3 toEye = normalize(camPos.xyz - position);
    float3 halfVec = normalize(toEye + lightDirection);
    float NDotH = saturate(dot(halfVec, normal)); // Cosine the angle between normal vector and half vector
    float specExp = 10.0f; // Get from obj
    finalColour += finalColour * pow(NDotH, specExp) * specular;
    
    return colour * finalColour;
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 position = positionGBuffer[DTid.xy].xyz;
    float3 normal = normalGBuffer[DTid.xy].xyz;
    float3 colour = diffuseGBuffer[DTid.xy].xyz;
    float3 ambient = ambientGBuffer[DTid.xy].xyz;
    float3 specular = specularGBuffer[DTid.xy].xyz;
    
    if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) // For particle colour
    {
        backBufferUAV[DTid.xy] = float4(colour, backBufferUAV[DTid.xy].w);
        return;
    }
    normal = normalize(normal);
    float3 worldAmbient = float3(0.25f, 0.25f, 0.25f) * ambient;
    if (fullLight)
    {
        worldAmbient = float3(1.0f, 1.0f, 1.0f) * ambient;
    }
    float3 result = colour * worldAmbient;
    if (camPos.w) // If light are on
    {
        // Directional light
        float4 lightWorldPos = mul(float4(position, 1.0f), directionalLight[0].vpMatrix);
        float2 ndcSpace = lightWorldPos.xy / lightWorldPos.w;
        float calcDepth = lightWorldPos.z / lightWorldPos.w;
        
        float3 shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, 0);
        float sampledDepth = dirShadowMaps.SampleLevel(shadowMapSampler, shadowMapUV, 0) + SHADOW_EPSILON;
        bool dirShadow = sampledDepth < calcDepth;
        
        if (!dirShadow || !shadows)
        {
            float3 dirColour = addDirectionalLight(directionalLight[0], position, colour, normal, specular);
            result += dirColour;
        }
        
        // Spot lights
        for (int i = 0; i < totalSpotLights; i++)
        {
            lightWorldPos = mul(float4(position, 1.0f), spotLights[i].vpMatrix);
            ndcSpace = lightWorldPos.xy / lightWorldPos.w;
            calcDepth = lightWorldPos.z / lightWorldPos.w;
            
            shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, i);
            sampledDepth = spotShadowMaps.SampleLevel(shadowMapSampler, shadowMapUV, 0) + SHADOW_EPSILON;
            bool spotShadow = sampledDepth < calcDepth;
            
            if (!spotShadow || !shadows)
            {
                float3 spotColour = addSpotLight(spotLights[i], position, colour, normal, specular);
                result += spotColour;
            }
        }
    }
    
    backBufferUAV[DTid.xy] = float4(result, backBufferUAV[DTid.xy].w);
}

// https://subscription.packtpub.com/book/game-development/9781849694209/1/ch01lvl1sec12/spot-light
// https://www.braynzarsoft.net/viewtutorial/q16390-21-spotlights

/*
dot product of two paralell vectors = 1 (0deg)
dot product of two orthogonal vectors = 0 (90deg)
*/