cbuffer Positions : register(b0)
{
    float4 camPosition;
    float4 objPosition;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

struct HullShaderOutput
{
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

// Get the inside tessellation factor depending on how far the camera is from the object
uint CalcTessFactor()
{
    float distance = length(objPosition.xyz - camPosition.xyz);
    if (distance <= 5.0f)
    {
        return 8;
    }
    if (distance > 5.0f && distance <= 10.0f)
    {
        return 2;
    }
    if (distance > 10.0f)
    {
        return 1;
    }
    return 1;
}

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip)
{
	HS_CONSTANT_DATA_OUTPUT output;

    output.EdgeTessFactor[0] =
	output.EdgeTessFactor[1] =
	output.EdgeTessFactor[2] =
	output.InsideTessFactor = CalcTessFactor(); // Total inner triangles are determined by Floor(innerFactor / 2)

    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HullShaderOutput main(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID)
{
    HullShaderOutput output;
    
    output.worldPos = ip[i].worldPos;
    output.normal = ip[i].normal;
    output.uv = ip[i].uv;
	
	return output;
}
