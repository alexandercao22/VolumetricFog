RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> depthGBuffer : register(t1);

struct DirectionalLightBuffer
{
    matrix vpMatrix;
    float3 colour;
    float3 direction;
};

StructuredBuffer<DirectionalLightBuffer> directionalLight : register(t7);
Texture2DArray<float> dirShadowMaps : register(t8);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    backBufferUAV[DTid.xy] = float4(backBufferUAV[DTid.xy].xyz + float3(1.0f, 0.0f, 0.0f), 1.0f);
}