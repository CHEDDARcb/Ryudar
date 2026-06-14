// 元SceneとBlur済みのBloom画像を合成する。
Texture2D g_texture0 : register(t0); // 元Scene
Texture2D g_texture1 : register(t1); // Blur済みBloom
SamplerState g_sampler : register(s0);

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
    return g_texture0.Sample(g_sampler, input.texcoord) * strength
         + g_texture1.Sample(g_sampler, input.texcoord);
}
