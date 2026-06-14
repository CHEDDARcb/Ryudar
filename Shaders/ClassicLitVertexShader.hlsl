#include "Common.hlsli"

// CPU의 VertexConstantData와 같은 순서로 유지한다.
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
    
    // 조명 계산은 월드 좌표계에서 수행하므로 투영 전 위치를 별도로 전달한다.
    output.posWorld = pos.xyz;
    
    // 래스터라이저에 전달할 최종 클립 공간 위치를 계산한다.
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.color = float3(0.0f, 0.0f, 0.0f); // 공통 출력 형식을 위한 기본값
    
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    return output;
}
