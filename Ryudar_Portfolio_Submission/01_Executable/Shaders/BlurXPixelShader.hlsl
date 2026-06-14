// 入力Textureへ水平方向のGaussian Blurを適用する。
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
    // 水平方向へ5-Tap Gaussian Blurを適用する。
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    int i;
    for (i = 0; i < 5; i++)
    {
        color += weights[i] * g_texture0.Sample(g_sampler, input.texcoord
                                                + float2(texelWidth, 0.0) * float(i - 2)).rgb;
    }
    
    return float4(color, 1.0);
}
