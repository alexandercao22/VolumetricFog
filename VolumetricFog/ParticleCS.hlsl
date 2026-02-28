struct Particle
{
    float4 pos;
};

cbuffer buffer : register(b0)
{
    float deltaTime;
    float3 padding;
}

RWStructuredBuffer<Particle> Particles : register(u0);

#define PI 3.1415927f

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Particle gettingProcessed = Particles[DTid.x];
    
    // Logic manipulating "gettingProcessed" goes here
    float fallSpeed = 2.0f * deltaTime;
    gettingProcessed.pos.y -= fallSpeed;
    if (gettingProcessed.pos.y < -10.0f)
    {
        gettingProcessed.pos.y = 20.0f;
    }
    
    float sideSpeed = 1.0f * deltaTime;
    gettingProcessed.pos.w -= sideSpeed;
    if (gettingProcessed.pos.w < -PI)
    {
        gettingProcessed.pos.w = PI;
    }
    gettingProcessed.pos.x += cos(gettingProcessed.pos.w) * sideSpeed;
    gettingProcessed.pos.z += sin(gettingProcessed.pos.w) * sideSpeed;
    
    Particles[DTid.x] = gettingProcessed;
}