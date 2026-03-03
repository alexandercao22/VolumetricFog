RWTexture3D<float4> fogDataUAV : register(u1);

#define SHADOW_EPSILON 0.0001f

cbuffer FroxelRaysCB : register(b8)
{
    float3 ray00; float _pad0;
    float3 ray10; float _pad1;
    float3 ray01; float _pad2;
    float3 ray11; float _pad3;
};

cbuffer FroxelCameraCB : register(b9)
{
    float4x4 invView;
    float nearZ;
    float farZ;
    uint totalSpotLights;
    float _pad4;
};

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
StructuredBuffer<DirectionalLightBuffer> directionalLight : register(t7);
Texture2DArray<float> dirShadowMaps : register(t8);
sampler shadowMapSampler : register(s0);

float3 FroxelToWorldPos(uint3 id)
{
    uint3 dimension;
    fogDataUAV.GetDimensions(dimension.x, dimension.y, dimension.z);
    
    float2 uv = (id.xy + 0.5f) / dimension.xy;

    float3 rayX0 = lerp(ray00, ray10, uv.x);
    float3 rayX1 = lerp(ray01, ray11, uv.x);
    float3 ray = lerp(rayX0, rayX1, uv.y);
    
    float slice = (id.z + 0.5f) / dimension.z;
    float z = nearZ * pow(farZ / nearZ, slice);
    
    float3 viewPos = ray * z;
    
    return mul(float4(viewPos, 1.0f), invView).xyz;
}

bool IsSampledPosShadowed(float3 samplePos, matrix lightViewProj, Texture2DArray<float> shadowMap, int index)
{
    float4 lightWorldPos = mul(float4(samplePos, 1.0f), lightViewProj);
    float2 ndcSpace = lightWorldPos.xy / lightWorldPos.w;
    float calcDepth = lightWorldPos.z / lightWorldPos.w;
    
    // If the samplePos is outside the light shadow map
    if (lightWorldPos.w <= 0.0f)
        return false;
    if (abs(ndcSpace.x) > 1.0f || abs(ndcSpace.y) > 1.0f)
        return false;
    
    float3 shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, index);
    float sampledDepth = shadowMap.SampleLevel(shadowMapSampler, shadowMapUV, 0) + SHADOW_EPSILON;
    return sampledDepth < calcDepth;
}

float CalculateAttenuation(SpotLightBuffer light, float3 samplePos)
{
    float3 toLight = normalize(light.position - samplePos);
    float dotCone = -dot(toLight, normalize(light.direction));
    float coneAngle = acos(dotCone); // The angle from direction vector to toLight vector.
    if (coneAngle < light.inAngle) // If inside inner cone, fully lit
    {
        return 1.0f;
    }
    else if (coneAngle < light.outAngle) // If outside innercone but inside outer cone, attenuate
    {
        float coneAttenuation = (coneAngle - light.inAngle) / (light.outAngle - light.inAngle);
        coneAttenuation = -(coneAttenuation - 1.0f);
        return coneAttenuation;
    }
    return 0.0f; // Outside light cone
}

float CalculateRdotL(float3 rayDir, float3 lightDir)
{
    lightDir = normalize(lightDir); // Direction of directional light
    return dot(rayDir, lightDir);
}

float PhaseHG(float cosTheta, float g)
{
    // https://omlc.org/classroom/ece532/class3/hg.html
    float g2 = g * g;
    return (1 - g2) / (2 * pow(1 + g2 - 2 * g * cosTheta, 3.0f / 2.0f));
}

[numthreads(8, 8, 4)] // UAV dimensions = 160, 90, 32. Dispatch(20, 12, 8)
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 worldPos = FroxelToWorldPos(DTid);
    
    // Volumetric fog settings
    float3 fogColor = float3(1.0f, 1.0f, 1.0f);
    float density = 0.5f;
    float scattering = fogColor * density;
    
    // Directional light
    bool isShadowed = IsSampledPosShadowed(worldPos, directionalLight[0].vpMatrix, dirShadowMaps, 0);
    if (!isShadowed)
    {
        float3 toLight = normalize(directionalLight[0].direction - worldPos);
        float RdotL = CalculateRdotL(-float3(DTid), toLight);
        fogDataUAV[DTid] += float4(directionalLight[0].colour * PhaseHG(RdotL, scattering), 1.0f);
    }
    
    // Spot lights
    for (int i = 0; i < totalSpotLights; i++)
    {
        isShadowed = IsSampledPosShadowed(worldPos, spotLights[i].vpMatrix, spotShadowMaps, i);
        if (!isShadowed)
        {
            float3 toLight = normalize(spotLights[i].direction - worldPos);
            float RdotL = CalculateRdotL(-float3(DTid), toLight);
            float attenuation = abs(CalculateAttenuation(spotLights[i], worldPos));
            fogDataUAV[DTid] += float4(spotLights[i].colour * attenuation * PhaseHG(RdotL, scattering), 1.0f);
        }
    }
}