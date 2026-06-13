Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

static const float weights[5] = { 0.0545f, 0.2442f, 0.4026f, 0.2442f, 0.0545f };

cbuffer SamplingPixelConstantData : register(b0)
{
    float texelWidth;
    float texelHeight;
    float threshold;
    float strength;
    float4 options;
};

struct SamplingPixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 main(SamplingPixelShaderInput input) : SV_TARGET
{
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < 5; i++)
    {
        color += weights[i] * g_texture0.Sample(g_sampler, input.texcoord +
                                                float2(0.0f, texelHeight) * float(i - 2)).rgb;
    }
    
    return float4(color, 1.0f);
}
