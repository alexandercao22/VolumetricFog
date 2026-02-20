struct Particle
{
    float4 position;
};

StructuredBuffer<Particle> Particles : register(t0);

float4 main( uint vertexID : SV_VertexID ) : POSITION
{
    return Particles[vertexID].position;
}