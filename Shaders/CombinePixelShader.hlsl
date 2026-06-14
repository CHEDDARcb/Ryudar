// 원본 장면과 블러 처리된 블룸 이미지를 합성한다.
Texture2D g_texture0 : register(t0); // 원본 버퍼
Texture2D g_texture1 : register(t1); // 블러처리한 버퍼
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
