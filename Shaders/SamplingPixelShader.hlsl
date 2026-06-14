// 밝기 임계값보다 어두운 픽셀을 제거해 블룸 영역을 추출한다.
Texture2D g_texture0 : register(t0);
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
    float3 color = g_texture0.Sample(g_sampler, input.texcoord).rgb;
    float l = (color.r + color.g + color.b) / 3.0f;
    
    return l > threshold ? float4(color, 1.0f) : float4(0.0f, 0.0f, 0.0f, 0.0f);
}
