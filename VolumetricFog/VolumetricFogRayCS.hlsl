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
    backBufferUAV[DTid.xy] = float4(backBufferUAV[DTid.xy].xyz + float3(1.0f, 0.0f, 0.0f), 1.0f);
}