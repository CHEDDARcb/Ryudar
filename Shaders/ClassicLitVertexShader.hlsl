#include "Common.hlsli"

// CPU側のVertexConstantDataと同じフィールド順を維持する。
cbuffer ModelViewProjectionConstantBuffer : register(b0) {
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, model);
    
    // Light計算はWorld Spaceで行うため、Projection前の位置を別途渡す。
    output.posWorld = pos.xyz;
    
    // Rasterizerへ渡す最終Clip Space位置を計算する。
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.color = float3(0.0f, 0.0f, 0.0f); // 共通出力形式のDefault値
    
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    return output;
}
