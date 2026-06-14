#include "Common.hlsli"

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

// Real-Time Rendering 4판의 Schlick 근사식을 사용한다.
// fresnelR0는 표면을 정면에서 볼 때의 재질 고유 반사율이다.
float3 SchlickFresnel(float3 fresnelR0, float3 normal, float3 toEye)
{
    // http://psgraphics.blogspot.com/2020/03/fresnel-equations-schlick-approximation.html

    float normalDotView = saturate(dot(normal, toEye));
    float f0 = 1.0f - normalDotView;

    // 시선이 표면에 평행해질수록 반사율이 1에 가까워진다.
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
    
    [unroll] // 고정된 소수의 광원을 반복하므로 루프를 펼친다.
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
    
    // IBL을 사용하면 직접광 대신 큐브맵에서 디퓨즈와 스페큘러를 얻는다.
    if (useIBL)
    {
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
    else // 직접광 사용
    {
        if (useRimLight)
        {
            // 시선과 노멀이 수직에 가까운 가장자리일수록 림 값을 크게 만든다.
            float rim = saturate(1.0f - dot(toEye, normal));
            // 지수로 림 라이트가 퍼지는 범위를 조절한다.
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
