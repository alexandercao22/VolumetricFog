RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> depthGBuffer : register(t1);

cbuffer camera : register(b8)
{
    float4 camPos;
    matrix viewProj;
}

cbuffer data : register(b9)
{
    uint2 resolution;
    float time;
    float deltaTime;
}

struct DirectionalLightBuffer
{
    matrix vpMatrix;
    float3 colour;
    float3 direction;
};

StructuredBuffer<DirectionalLightBuffer> directionalLight : register(t7);
Texture2DArray<float> dirShadowMaps : register(t8);

float PhaseHG(float cosTheta, float g)
{
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

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 uv = (DTid.xy + 0.5f) / resolution;
    float4 col = backBufferUAV[DTid.xy];
    
    float depth = depthGBuffer.Load(int3(DTid.xy, 0));
 
    float3 worldPos = ComputeWorldSpacePosition(uv, depth, viewProj);
    
    float3 viewDir = worldPos - camPos.xyz;
    float viewLength = length(viewDir);
    float3 rayDir = normalize(viewDir);

    // Volumetric fog settings
    float density = 0.02f; // Fog density
    float maxDistance = 50.0f;
    uint steps = 20;
    float stepSize = 2.0f;
    float noiseOffset = 2.0f;
    float4 fogColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float scattering = 0.6f;
    
    float2 pixelCoords = DTid.xy * resolution;
    float distLimit = min(viewLength, maxDistance);
    float t = time / max(0.01f, deltaTime);
    float distTravelled = IGN(pixelCoords, t) * noiseOffset;
    float transmittance = 1.0f;
    
    // Ray-marching
    while (distTravelled < distLimit)
    {
        if (density > 0.0f)
        {
            fogColor.rgb += directionalLight[0].colour * density * stepSize;
            transmittance *= exp(-density * stepSize);
        }
        distTravelled += stepSize;
    }
    
    backBufferUAV[DTid.xy] = lerp(col, fogColor, 1.0f - saturate(transmittance));
}