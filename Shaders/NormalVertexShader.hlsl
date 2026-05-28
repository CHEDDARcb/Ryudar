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
    float scale; // 그려지는 선분의 길이 조절
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    
    // Normal먼저 변환
    // 노멀벡터의 방향을 알아야 선분의 끝점을 그릴수있음.
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    pos = mul(pos, model);
    
    // 선분의 끝점인지 시작점인지 texcoord에1, 0을 넣어서 구분
    float t = input.texcoord.x;
    pos.xyz += output.normalWorld * t * scale;
    
    output.posWorld = pos.xyz;
    
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
    
    //시작점과 끝점의 색 구분.
    output.color = float3(1.0f, 1.0f, 0.0f) * (1.0f - t) + float3(1.0f, 0.0f, 0.0f) * t;
    
    return output;
}