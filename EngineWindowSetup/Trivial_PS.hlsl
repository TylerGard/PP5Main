texture2D base : register(t0);
SamplerState samples : register(s0);

float4 main(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	float3 uv;
	uv[0] = color[0];
	uv[1] = 1 - color[1];
	uv[2] = color[2];
	return base.Sample(samples, uv);
	

}