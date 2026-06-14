// 스카이박스 정점을 카메라의 뷰 및 투영 공간으로 변환한다.
#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix viewProj;
};

struct CubeMappingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

CubeMappingPixelShaderInput main(VertexShaderInput input)
{

    CubeMappingPixelShaderInput output;
    output.posModel = input.posModel;
    output.posProj = mul(float4(input.posModel, 1.0f), viewProj);

    return output;
}
