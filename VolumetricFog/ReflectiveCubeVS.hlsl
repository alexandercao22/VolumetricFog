cbuffer constantBuffer : register(b3)
{
    matrix viewProjMatrix;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(float4(input.position, 1.0f), viewProjMatrix);
    output.worldPos = input.position;
    output.normal = normalize(input.normal);
    output.uv = input.uv;
    return output;
}