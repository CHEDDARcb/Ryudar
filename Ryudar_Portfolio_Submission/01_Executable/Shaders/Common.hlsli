#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 複数Shaderが共有するMaterial、Light、頂点入出力形式を定義する。
// C++側Constant Buffer構造体とフィールド順およびサイズを一致させる。

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
    // Attenuation開始点から終了点までLight強度を線形に減衰させる。
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
    // 入射角WeightをLight強度へ反映し、DiffuseとSpecularへ共通適用する。
    float ndotl = max(dot(normal, lightVec), 0.0f);
    float3 lightStrength = L.strength * ndotl;
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat, shadingModel);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye, uint shadingModel)
{
    float3 lightVec = L.position - pos;
    
    // Light範囲外のPixelは計算を省略する。
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
    
    // Light範囲外のPixelは計算を省略する。
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

// Vertex ShaderとPixel Shaderが共有するSemantic構造体。
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct VertexShaderInput
{
    float3 posModel : POSITION;   // Model Space位置
    float3 normalModel : NORMAL;  // Model Space Normal
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Rasterizerが補間した画面位置
    float3 posWorld : POSITION;   // Light計算に使用するWorld位置
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 color : COLOR;         // Normal Line Shaderで使用
};

#endif // __COMMON_HLSLI__
