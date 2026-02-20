cbuffer Camera : register(b0)
{
    float4x4 worldMatrix;
    float4x4 vpMatrix;
    float3 camPos;
    float particleSize;
};

struct GSOutput
{
	float4 position : SV_POSITION;
};

#define yAxis float3(0.0f, 1.0f, 0.0f)

[maxvertexcount(6)]
void main(
	point float3 input[1] : POSITION, 
	inout TriangleStream< GSOutput > output
)
{
    float4x4 matMul = mul(worldMatrix, vpMatrix);
    
    float3 direction = normalize(input[0] - camPos);
    float3 rightVec = normalize(cross(direction, yAxis));
    float3 upVec = normalize(cross(rightVec, direction));
    rightVec *= particleSize;
    upVec *= particleSize;
    
    GSOutput particles[6];
    
    particles[0].position = mul(float4(input[0] - rightVec + upVec, 1.0f), matMul); // Top Left
    particles[1].position = mul(float4(input[0] - rightVec - upVec, 1.0f), matMul); // Bottom Left
    particles[2].position = mul(float4(input[0] + rightVec - upVec, 1.0f), matMul); // Bottom Right
    
    for (int i = 0; i < 3; i++)
    {
        output.Append(particles[i]);
    }
    output.RestartStrip();
    
    particles[3].position = mul(float4(input[0] + rightVec - upVec, 1.0f), matMul); // Bottom Right
    particles[4].position = mul(float4(input[0] + rightVec + upVec, 1.0f), matMul); // Top Right
    particles[5].position = mul(float4(input[0] - rightVec + upVec, 1.0f), matMul); // Top Left
    
    for (i = 3; i < 6; i++)
    {
        output.Append(particles[i]);
    }
}