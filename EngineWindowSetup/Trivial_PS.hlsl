texture2D base : register(t0);
SamplerState samples : register(s0);

float4 main(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	float2 uv;
	uv[0] = color[0];
	uv[1] = 1 - color[1];
	//uv[2] = 0;// color[2];
	//return color;
	float4 combinedColor = base.Sample(samples, uv);
	if (combinedColor[3] <= 0)
	{
		return color;
	}
	else
	{
		return combinedColor;
	}
}