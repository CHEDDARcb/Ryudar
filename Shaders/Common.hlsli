#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 쉐이더에서 include할 내용들은 .hlsli파일에 작성
// Properties -> Item Type: Does not participate in build으로 설정

// C++ SimpleMath -> HLSL
// Matrix -> matrix 또는 float4x4
// Vector3 -> float3
// float3 a = normalize(b);
// float a = dot(v1, v2);
// Satuarate() -> saturate() 사용
// float l = length(v);
// struct A{ float a = 1.0f; }; <- 구조체 안에서 초기화 불가
// Vector3(0.0f) -> float3(0.0, 0.0, 0.0) // 실수 뒤에 f 불필요
// Vector4::Transform(v, M) -> mul(v, M)

#define MAX_LIGHTS 3
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

// 재질
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float padding0;
    float3 specular;
    float padding1;
    float3 fresnelR0;
    float padding2;
};

// 조명
struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                  float3 toEye, Material mat, bool useBlinnPhong, bool usePhong)
{
    //bool useBlinnPhong, bool usePhong
    if (useBlinnPhong)
    {
        float3 halfway = normalize(toEye + lightVec);
        float hdotn = dot(halfway, normal);
        float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess);
        return mat.ambient + (mat.diffuse + specular) * lightStrength;
    }
    else if (usePhong)
    {
        float3 r = -reflect(lightVec, normal);
        float3 specular = mat.specular * pow(max(dot(toEye, r), 0.0f), mat.shininess);
        return mat.ambient + (mat.diffuse + specular) * lightStrength;
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal,
                               float3 toEye, bool useBlinnPhong, bool usePhong)
{
    float3 lightVec = -L.direction;
    // 원래 ndotl은 diffuse에 영향을 주는 가중치.
    // 여기서는 lightStrength에 곱해서, 
    // BlinnPhong함수에서 diffuse와 specular둘다에 곱해주는 형태가 되버림.
    // -> 미세조정을 위해서 가중치를 분리시키는 것이 좋지만,
    // 광량에따라 전체가 줄어들듯이 표현함.
    float ndotl = max(dot(normal, lightVec), 0.0f);
    float3 lightStrength = L.strength * ndotl;
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, useBlinnPhong, usePhong);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye, bool useBlinnPhong, bool usePhong)
{
    float3 lightVec = L.position - pos;
    
    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);
    
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;
        
        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.strength * ndotl;
        
        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, useBlinnPhong, usePhong);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                        float3 toEye, bool useBlinnPhong, bool usePhong)
{
    float3 lightVec = L.position - pos;
    
    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);
    
    // 너무 멀면 조명이 적용되지 않게함.
    if (d > L.fallOffEnd)
    {
        return float3(0, 0, 0);
    }
    else
    {
        lightVec /= d;
        
        float ndotl = max(dot(normal, lightVec), 0.0f);
        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        float3 lightStrength = L.strength * ndotl * att;
        
        float spotFactor = pow(max(dot(-lightVec, L.direction), 0), L.spotPower);
        lightStrength *= spotFactor;
        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, useBlinnPhong, usePhong);

    }
}

// Semantics
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct VertexShaderInput
{
    //<- Semantics: 주석과 비슷한 의미. 어떠한 용도의 변수인지 지정.
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD0; // 숫자는 종류가 여러개 일때, 숫자를 붙여 구분함. 하나밖에 없으면 숫자 생략가능.
};


// SV_(System-value semantics): for rasterizer state.
// SV_없으면 화면에 렌더링 안됨.
// PixelShader로 들어가는 데이터는 SV붙여줘야함.
struct PixelShaderInput
{
    //SV_POSITION: Vertex단위 데이터가 아니라, Pixel단위로interpolation된 정보가 들어감.
    float4 posProj : SV_POSITION; // Screen position
    // float3 posModel : POSITION0; // Model Position (텍스처좌표 계산-구 매핑 사용)
    float3 posWorld : POSITION; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 color : COLOR; // Normal lines쉐이더에서 사용
};

#endif // __COMMON_HLSLI__