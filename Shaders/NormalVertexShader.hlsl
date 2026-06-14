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
    float scale; // Normal Lineの長さ
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    
    // Inverse Transpose MatrixでWorld SpaceのNormal方向を先に求める。
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    pos = mul(pos, model);
    
    // texcoord.xの0と1でLineの開始点と終了点を区別する。
    float t = input.texcoord.x;
    pos.xyz += output.normalWorld * t * scale;
    
    output.posWorld = pos.xyz;
    
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
    
    // 開始点は黄色、終了点は赤色で表示する。
    output.color = float3(1.0f, 1.0f, 0.0f) * (1.0f - t) + float3(1.0f, 0.0f, 0.0f) * t;
    
    return output;
}
