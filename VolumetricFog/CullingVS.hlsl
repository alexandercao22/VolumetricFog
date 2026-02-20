cbuffer ConstantBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewProjMatrix;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 colour : COLOUR;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 colour : COLOUR;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    output.position = mul(mul(float4(input.position, 1.0f), worldMatrix), viewProjMatrix);
    output.colour = input.colour;
    
    return output;
}