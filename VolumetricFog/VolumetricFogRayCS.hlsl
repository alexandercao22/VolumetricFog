RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> depthGBuffer : register(t2);

#define SHADOW_EPSILON 0.0001f

cbuffer camera : register(b8)
{
    float4 camPos;
    matrix viewProj;
}

cbuffer data : register(b9)
{
    float time;
    float deltaTime;
    uint totalSpotLights;
}

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

float PhaseHG(float cosTheta, float g)
{
    // https://omlc.org/classroom/ece532/class3/hg.html
    float g2 = g * g;
    return (1 - g2) / pow(1 + g2 * 2 * g * cosTheta, 1.5);
}

float3 ComputeWorldSpacePosition(float2 uv, float depth, matrix vp)
{
    float2 ndc;
    ndc.x = uv.x * 2.0f - 1.0f;
    ndc.y = 1.0f - uv.y * 2.0f;
    
    float4 clipPos = float4(ndc, depth, 1.0f);
    
    float4 worldPos = mul(clipPos, viewProj);
    worldPos /= worldPos.w;
    
    return worldPos.xyz;
}

float IGN(float2 pixel, int frame)
{
    // https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
    frame = frame % 64;
    float x = float(pixel.x) + 5.588238f * float(frame);
    float y = float(pixel.y) + 5.588238f * float(frame);
    return fmod(52.9829189f * fmod(0.06711056f * float(x) + 0.00583715f * float(y), 1.0f), 1.0f);
}

bool IsSampledPosShadowed(float3 samplePos, matrix lightViewProj, Texture2DArray<float> shadowMap)
{
    float4 lightWorldPos = mul(float4(samplePos, 1.0f), lightViewProj);
    float2 ndcSpace = lightWorldPos.xy / lightWorldPos.w;
    float calcDepth = lightWorldPos.z / lightWorldPos.w;
    
    // If the samplePos is outside the light shadow map
    if (lightWorldPos.w <= 0.0f)
        return false;
    if (abs(ndcSpace.x) > 1.0f || abs(ndcSpace.y > 1.0f))
        return false;
    
    float3 shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, 0);
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

float3 NormalizeByMaxComponent(float3 v)
{
    float m = max(max(abs(v.x), abs(v.y)), abs(v.z));
    return (m > 0) ? v / m : v;
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 resolution;
    backBufferUAV.GetDimensions(resolution.x, resolution.y);
    float2 uv = (DTid.xy + 0.5f) / resolution;
    float4 col = backBufferUAV[DTid.xy];
    
    float depth = depthGBuffer.Load(int3(DTid.xy, 0));
 
    float3 worldPos = ComputeWorldSpacePosition(uv, depth, viewProj);
    
    //float3 viewDir = worldPos - camPos.xyz; // Does not work for some reason
    float3 viewDir = worldPos - float3(0.0f, 0.0f, 0.0f);
    float viewLength = length(viewDir);
    float3 rayDir = normalize(viewDir);

    // Volumetric fog settings
    float density = 0.015f;
    float maxDistance = 50.0f;
    float stepSize = 2.0f;
    float noiseOffset = 2.0f;
    float4 fogColor = 0.2f;
    float scattering = 0.3f;
    
    float2 pixelCoords = DTid.xy * resolution;
    float distLimit = min(viewLength, maxDistance);
    float t = time / max(0.01f, deltaTime);
    float distTravelled = IGN(pixelCoords, t) * noiseOffset;
    float transmittance = 1.0f;
    
    // Ray-marching
    while (distTravelled < distLimit)
    {
        float3 sampleWorldPos = camPos.xyz + rayDir * distTravelled;
        
        // Directional light
        bool isShadowed = IsSampledPosShadowed(sampleWorldPos, directionalLight[0].vpMatrix, dirShadowMaps);
        if (density > 0.0f && !isShadowed)
        {
            float RdotL = CalculateRdotL(rayDir, directionalLight[0].direction);
            fogColor.rgb += directionalLight[0].colour * PhaseHG(RdotL, scattering) * density * stepSize;
            transmittance *= exp(-density * stepSize);
        }
        
        // Spot lights
        for (int i = 0; i < totalSpotLights; i++)
        {
            isShadowed = IsSampledPosShadowed(sampleWorldPos, spotLights[i].vpMatrix, spotShadowMaps);
            if (density > 0.0f && !isShadowed)
            {
                float RdotL = CalculateRdotL(rayDir, spotLights[i].direction);
                float attenuation = abs(CalculateAttenuation(spotLights[i], sampleWorldPos));
                fogColor.rgb += spotLights[i].colour * attenuation * PhaseHG(RdotL, scattering) * density * stepSize;
                transmittance *= exp(-density * stepSize);
            }
        }
        
        distTravelled += stepSize;
    }
    
    backBufferUAV[DTid.xy] = lerp(col, fogColor, 1.0f - saturate(transmittance));
}