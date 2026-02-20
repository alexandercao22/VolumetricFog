Texture2D ambient : register(t0);
Texture2D diffuse : register(t1);
Texture2D specular : register(t2);
sampler materialSampler : register(s1);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct PixelShaderOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 diffuse : SV_Target2;
    float4 ambient : SV_Target3;
    float4 specular : SV_Target4;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.position = float4(input.worldPos, 0.0f);
    output.normal = float4(normalize(input.normal), 0.0f);
    output.diffuse = diffuse.Sample(materialSampler, input.uv);
    output.ambient = ambient.Sample(materialSampler, input.uv);
    output.specular = specular.Sample(materialSampler, input.uv);
    return output;
}