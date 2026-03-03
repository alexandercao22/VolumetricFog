RWTexture3D<float4> input : register(uX);        // Input
RWTexture3D<unorm float4> output : register(uX); // Output

cbuffer data : register(bX)
{
    float nearPlane;
    float farPlane;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint3 dimensions;
    output.GetDimensions(dimensions.x, dimensions.y, dimensions.z);
    
    // Bounds check?
    
    float4 accumulated = float4(0.0f, 0.0f, 0.0f, 1.0f); // 1.0f is transmittance
    float currentZ = nearPlane;
    
    // Raymarching
    for (int i = 0; i < dimensions.z; i++)
    {
        uint3 cellCoord = uint3(DTid.xy, i);
        
        float4 cellData = input[cellCoord];
        float3 cellScattering = cellData.xyz;
        float cellDensity = cellData.a;
        
        float nextSliceStart = float(cellCoord.z + 1) / float(dimensions.z);
        float nextZ = nearPlane * pow(farPlane / nearPlane, nextSliceStart);
        float stepSize = nextZ - currentZ;
        currentZ = nextZ; // For next iteration
        
        float transmittance = exp(-cellDensity * stepSize); // Beer-Lambert law
        
        float3 scatteringIntegral = cellScattering * (1.0f - transmittance) / max(cellDensity, 0.00001f);
        accumulated.rgb += scatteringIntegral * accumulated.a;
        
        output[cellCoord] = accumulated;
    }
}