// Data Types (HLSL)
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-data-types

// Shader Constants (HLSL)
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-constants

// Register
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-register

// float4, matrix
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math

#include "Common.hlsli"

// register(b0): GPU에 있는 레지스터에 넣어라.
// b: GPU register for constant buffer.
cbuffer ModelViewProjectionConstantBuffer : register(b0) {
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

// Intrinsic Functions
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions

PixelShaderInput main(VertexShaderInput input)
{
    // 모델(Model) 행렬은 모델 자신의 원점에서 
    // 월드 좌표계에서의 위치로 변환.
    // 모델 좌표계의 위치 -> [모델 행렬 곱하기] -> 월드 좌표계의 위치
    // -> [뷰 행렬 곱하기] -> 뷰 좌표계의 위치 -> [프로젝션 행렬 곱하기]
    // -> NDC에서의 위치
    
    // NDC에서는 조명의 방향이나 조명과 물체의 거리 등이 달라지기 때문에
    // 월드 좌표계에서 조명을 계산.
    
    PixelShaderInput output;
    /*구 매핑*/
    // 픽셀쉐이더에서 텍스처좌표 계산
    //output.posModel = input.posModel; 
    
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, model);
    
    output.posWorld = pos.xyz; // 월드 위치 따로 저장
    
    // Q. model, view, projection을 처음부터 곱한다음에
    //    GPU에 보내주면 효율적이지 않음?
    // A. 아님. 따로 보내주는 효율적임.
    //    이유는 나중에....
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.color = float3(0.0f, 0.0f, 0.0f); // 다른 쉐이더에서 사용
    
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    return output;
}
