struct PixelShaderOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 diffuse : SV_Target2;
    float4 ambient : SV_Target3;
    float4 specular : SV_Target4;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.ambient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}