Texture3D<float4> froxelLightSRV : register(t1); // Input
RWTexture3D<float4> froxelAccUAV : register(u1); // Output

cbuffer FroxelCameraCB : register(b9)
{
    float4x4 invView;
    float4 cameraPos;
    float nearZ;
    float farZ;
    uint totalSpotLights;
    float _pad4;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint3 dimensions;
    froxelAccUAV.GetDimensions(dimensions.x, dimensions.y, dimensions.z);
    
    // Bounds check?
    
    float4 accumulated = float4(0.0f, 0.0f, 0.0f, 1.0f); // 1.0f is transmittance
    float currentZ = nearZ;
    
    // Raymarching
    for (int i = 0; i < dimensions.z; i++)
    {
        uint3 cellCoord = uint3(DTid.xy, i);
        
        float4 cellData = froxelLightSRV[cellCoord];
        float3 cellScattering = cellData.xyz;
        float cellDensity = cellData.a;
        
        float nextSliceStart = float(cellCoord.z + 1) / float(dimensions.z);
        float nextZ = nearZ * pow(farZ / nearZ, nextSliceStart);
        float stepSize = nextZ - currentZ;
        currentZ = nextZ; // For next iteration
        
        float transmittance = exp(-cellDensity * stepSize); // Beer-Lambert law
        
        float3 scatteringIntegral = cellScattering * (1.0f - transmittance) / max(cellDensity, 0.00001f);
        accumulated.rgb += scatteringIntegral * accumulated.a;
        accumulated.a *= transmittance;
        
        froxelAccUAV[cellCoord] = accumulated;
    }
}