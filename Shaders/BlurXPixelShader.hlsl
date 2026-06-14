// 입력 텍스처에 가로 방향 가우시안 블러를 적용한다.
Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

static const float weights[5] = { 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 };

cbuffer SamplingPixelConstantData : register(b0)
{
    float texelWidth;
    float texelHeight;
    float threshold;
    float strength;
};

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    // 가로 방향으로 5탭 가우시안 블러를 적용한다.
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    int i;
    for (i = 0; i < 5; i++)
    {
        color += weights[i] * g_texture0.Sample(g_sampler, input.texcoord
                                                + float2(texelWidth, 0.0) * float(i - 2)).rgb;
    }
    
    return float4(color, 1.0);
}
