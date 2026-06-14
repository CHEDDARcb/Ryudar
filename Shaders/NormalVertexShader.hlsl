#include "Common.hlsli"

cbuffer NormalVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
}

cbuffer NormalVertexConstantBuffer : register(b1)
{
    float scale; // 노멀 선분의 길이
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    
    // 역전치 행렬로 월드 공간의 노멀 방향을 먼저 구한다.
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    pos = mul(pos, model);
    
    // texcoord.x의 0과 1로 선분의 시작점과 끝점을 구분한다.
    float t = input.texcoord.x;
    pos.xyz += output.normalWorld * t * scale;
    
    output.posWorld = pos.xyz;
    
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
    
    // 시작점은 노란색, 끝점은 빨간색으로 표시한다.
    output.color = float3(1.0f, 1.0f, 0.0f) * (1.0f - t) + float3(1.0f, 0.0f, 0.0f) * t;
    
    return output;
}
