// 정점 셰이더가 계산한 노멀 선의 디버그 색상을 출력한다.
#include "Common.hlsli"

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}
