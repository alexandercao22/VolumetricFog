struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 colour : COLOUR;
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
    output.position = input.position;
    output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.diffuse = float4(input.colour, 1.0f);
    output.ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}