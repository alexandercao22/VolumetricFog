cbuffer Camera : register(b0) // Reuse ParticleGS cbuffer
{
    float4x4 worldMatrix;
    float4x4 vpMatrix;
    float4 garbage;
};

struct DomainShaderOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HullShaderOutput
{
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

float2 BarycentricInterpolate(float3 bary, float2 p0, float2 p1, float2 p2)
{
    float2 interpolatedPos = p0 * bary.x + p1 * bary.y + p2 * bary.z;
    return interpolatedPos;
}

float3 BarycentricInterpolate(float3 bary, float3 p0, float3 p1, float3 p2)
{
    float3 interpolatedPos = p0 * bary.x + p1 * bary.y + p2 * bary.z;
    return interpolatedPos;
}

// Find the closest point from the flat barycentric interpolated position to the plane created by the corner position and its normal
float3 PhongProjection(float3 flatPosition, float3 cornerPosition, float3 normal)
{
    return flatPosition - dot(flatPosition - cornerPosition, normal) * normal;
}

// Find the barycentric interpolated position from the projected positions on the planes
float3 CalculatePhongPosition(float3 bary, float smoothing, float3 flatPos,
    float3 p0Pos, float3 p0Nor, float3 p1Pos, float3 p1Nor, float3 p2Pos, float3 p2Nor)
{
    float3 smoothedPosition =
        bary.x * PhongProjection(flatPos, p0Pos, p0Nor) +
        bary.y * PhongProjection(flatPos, p1Pos, p1Nor) +
        bary.z * PhongProjection(flatPos, p2Pos, p2Nor);
    return lerp(flatPos, smoothedPosition, smoothing);
}

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DomainShaderOutput main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 barycentric : SV_DomainLocation,
	const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch)
{
    DomainShaderOutput output;

    output.worldPos = BarycentricInterpolate(barycentric, patch[0].worldPos, patch[1].worldPos, patch[2].worldPos);
    output.normal = normalize(BarycentricInterpolate(barycentric, patch[0].normal, patch[1].normal, patch[2].normal));
    output.uv = BarycentricInterpolate(barycentric, patch[0].uv, patch[1].uv, patch[2].uv);
    
    float4x4 matMul = mul(worldMatrix, vpMatrix);
    float smoothing = 0.5f;
    float3 newPos = CalculatePhongPosition(barycentric, smoothing, output.worldPos,
        patch[0].worldPos, patch[0].normal, patch[1].worldPos, patch[1].normal, patch[2].worldPos, patch[2].normal);
    output.position = mul(float4(newPos, 1.0f), matMul);

	return output;
}
