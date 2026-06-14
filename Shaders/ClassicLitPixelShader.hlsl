#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

Texture2D g_texture0 : register(t0);
TextureCube g_diffuseCube : register(t1);
TextureCube g_specularCube : register(t2);
SamplerState g_sampler : register(s0);

cbuffer LightingConstantBuffer : register(b0)
{
    Light lights[MAX_LIGHTS];

    float3 eyeWorld;
    float padding;
};

cbuffer ShadingConstantBuffer : register(b1)
{
    Material material;

    float3 rimColor;
    float rimPower;

    float rimStrength;
    uint useRimLight;
    uint useSmoothstep;
    float rimPadding;

    uint useTexture;
    uint shadingModel; // 0: Phong, 1: Blinn-Phong
    float2 shadingPadding;

    uint useIBL;
    uint useEnvironmentReflection;
    float2 environmentPadding;
};

// Schlick approximation: Eq. 9.17 in "Real-Time Rendering 4th Ed."
// fresnelR0는 물질의 고유 성질
// Water : (0.02, 0.02, 0.02)
// Glass : (0.08, 0.08, 0.08)
// Plastic : (0.05, 0.05, 0.05)
// Gold: (1.0, 0.71, 0.29)
// Silver: (0.95, 0.93, 0.88)
// Copper: (0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 fresnelR0, float3 normal, float3 toEye)
{
    // 참고 자료들
    // THE SCHLICK FRESNEL APPROXIMATION by Zander Majercik, NVIDIA
    // http://psgraphics.blogspot.com/2020/03/fresnel-equations-schlick-approximation.html

    float normalDotView = saturate(dot(normal, toEye));

    float f0 = 1.0f - normalDotView; // 90도이면 f0 = 1, 0도이면 f0 = 0

    // 1.0 보다 작은 값은 여러 번 곱하면 더 작은 값이 된다..
    // 0도 -> f0 = 0 -> fresnelR0 반환
    // 90도 -> f0 = 1.0 -> float3(1.0) 반환
    // 0도에 가까운 가장자리는 Specular 색상, 90도에 가까운 안쪽은 고유
    // 색상(fresnelR0)
    return fresnelR0 + (1.0f - fresnelR0) * pow(f0, 5.0);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 toEye = normalize(eyeWorld - input.posWorld);
    float3 normal = normalize(input.normalWorld);
    float3 color = float3(0.0, 0.0, 0.0);
    
    int i = 0;
    
    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-for
    // https://forum.unity.com/threads/what-are-unroll-and-loop-when-to-use-them.1283096/
    
    [unroll] // warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(lights[i], material, normal, toEye, shadingModel);
    }
    
    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        color += ComputePointLight(lights[i], material, input.posWorld, normal, toEye, shadingModel);
    }
    
    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        color += ComputeSpotLight(lights[i], material, input.posWorld, normal, toEye, shadingModel);
    }
    
    // Image Based Light사용
    if (useIBL)
    {
        // 환경맵핑 사용.
        if (useEnvironmentReflection)
        {
            return g_specularCube.Sample(g_sampler, reflect(-toEye, normal));
        }
        else
        {
            float4 diffuse = g_diffuseCube.Sample(g_sampler,
                                            normal);
            float4 specular = g_specularCube.Sample(g_sampler,
                            reflect(-toEye, normal));
            
            diffuse.xyz *= material.diffuse;
            float specularBrightness = max((specular.r + specular.g + specular.b) / 3.0f, 0.f);
            specular *= pow(specularBrightness, material.shininess);
            specular.xyz *= material.specular;
            
            float3 f = SchlickFresnel(material.fresnelR0, normal, toEye);
            specular.xyz *= f;
            
            if (useTexture)
            {
                diffuse *= g_texture0.Sample(g_sampler, input.texcoord);
            }
    
            return diffuse + specular;
        }
    }
    else // 기본 라이트 사용.
    {
        if (useRimLight)
        {
            // Smoothstep
            // https://thebookofshaders.com/glossary/?search=smoothstep
            // Rim Lighting은 테두리를 밝게 하는것(역광을 인한 후광효과)
            // 테두리를 수학적으로 정의
            // 테두리에 해당하는 노멀벡터는 
            // toeye벡터(정점에서 눈으로 향하는 벡터)와 수직에 가까움.
            // ex) dot(테두리노멀벡터, toeye) =~ 0,
            //     dot(테두리아닌노멀벡터, toeye) =~ 1
            // color값에 테두리에 해당하는 값을 가중치로 더해주면됨.
            // 이때 가중치로 사용하고 싶은 값이 0이므로
            // 원하는 결과와 반대로됨.
            // 그러므로 1에서 빼줌으로써 가중치를 반전시킬수있음.
            float rim = saturate(1.0f - dot(toEye, normal));
            // 테두리일수록 더욱 후광을 선명하게,
            // 아닐수록 후광이 더욱 약하게 만들기 위해,
            // rimPower를 제곱시켜줌으로써 rim값을 극단적으로 조절할수있음.
            if (useSmoothstep)
            {
                rim = smoothstep(0.0f, 1.0f, rim);
            }
            rim = pow(rim, rimPower);
            color += rim * rimColor * rimStrength;
        }
        else
        {
            color = color;
        }
        return useTexture ? float4(color, 1.0) *
                            g_texture0.Sample(g_sampler,
                                              input.texcoord) :
                            float4(color, 1.0);
    }
}
