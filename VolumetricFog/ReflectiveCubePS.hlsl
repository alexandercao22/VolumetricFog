TextureCube cubeTexture : register(t0);
Texture2D ambient : register(t1);
Texture2D specular : register(t2);
sampler cubeSampler : register(s0);

cbuffer CameraInfo : register(b2)
{
    float3 cameraPos;
};

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
    float3 incomingView = (input.worldPos - cameraPos);
    float3 reflectedView = reflect(incomingView, input.normal);
    output.diffuse = cubeTexture.Sample(cubeSampler, reflectedView);
    output.ambient = ambient.Sample(cubeSampler, input.uv);
    output.specular = specular.Sample(cubeSampler, input.uv);
    return output;
}
