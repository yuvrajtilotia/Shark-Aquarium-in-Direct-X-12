struct Vertex
{
    float3 inPositionLS;
    float3 inNormal;
};

struct VS_OUT
{
    float4 outPositionCS    : SV_Position;
    float4 outPosWorld      : POSWORLD;
    float3 outNormal        : NORMAL;
};

StructuredBuffer<Vertex> vertices : register(t0, space0);
StructuredBuffer<unsigned int> indices: register(t1, space0);

struct VPConstantBuffer
{
    matrix VPMatrix;
};

ConstantBuffer<VPConstantBuffer> vpConstantBuffer : register(b0, space0);

cbuffer WorldConstantBuffer : register(b1, space0)
{
    matrix worldMatrix;
};

VS_OUT main(uint vertexID : SV_VertexID)
{
    Vertex input = vertices[indices[vertexID]];
    VS_OUT vsOut = (VS_OUT)0;
    vsOut.outPosWorld = mul(float4(input.inPositionLS, 1.0f), worldMatrix);
    
    vsOut.outPositionCS = mul(vsOut.outPosWorld, vpConstantBuffer.VPMatrix);
    vsOut.outNormal = normalize(mul(float4(normalize(input.inNormal), 0.0f), worldMatrix).xyz);
    
    return vsOut;
}