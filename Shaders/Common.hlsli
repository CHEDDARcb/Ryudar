#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 여러 셰이더가 공유하는 재질, 조명, 정점 입출력 형식을 정의한다.
// C++ 상수 버퍼 구조체와 필드 순서 및 크기를 동일하게 유지해야 한다.

#define MAX_LIGHTS 3
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

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

struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

static const uint SHADING_MODEL_PHONG = 0;
static const uint SHADING_MODEL_BLINN_PHONG = 1;

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // 감쇠 시작점부터 끝점까지 조명 세기를 선형으로 줄인다.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                  float3 toEye, Material mat, uint shadingModel)
{
    if (shadingModel == SHADING_MODEL_BLINN_PHONG)
    {
        float3 halfway = normalize(toEye + lightVec);
        float hdotn = dot(halfway, normal);
        float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess);
        return mat.ambient + (mat.diffuse + specular) * lightStrength;
    }
    else if (shadingModel == SHADING_MODEL_PHONG)
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
                               float3 toEye, uint shadingModel)
{
    float3 lightVec = -L.direction;
    // 입사각 가중치를 광원 세기에 반영해 디퓨즈와 스페큘러에 함께 적용한다.
    float ndotl = max(dot(normal, lightVec), 0.0f);
    float3 lightStrength = L.strength * ndotl;
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, shadingModel);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye, uint shadingModel)
{
    float3 lightVec = L.position - pos;
    
    // 조명 범위 밖의 픽셀은 계산을 생략한다.
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

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, shadingModel);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                        float3 toEye, uint shadingModel)
{
    float3 lightVec = L.position - pos;
    
    float d = length(lightVec);
    
    // 조명 범위 밖의 픽셀은 계산을 생략한다.
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
        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, shadingModel);

    }
}

// 정점 및 픽셀 셰이더가 공유하는 시맨틱 구조체.
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct VertexShaderInput
{
    float3 posModel : POSITION;   // 모델 좌표계 위치
    float3 normalModel : NORMAL;  // 모델 좌표계 노멀
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // 래스터라이저가 보간한 화면 위치
    float3 posWorld : POSITION;   // 조명 계산에 사용하는 월드 위치
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 color : COLOR;         // 노멀 선 셰이더에서 사용
};

#endif // __COMMON_HLSLI__
