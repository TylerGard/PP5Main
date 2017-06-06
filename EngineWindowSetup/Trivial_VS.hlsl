cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VOut
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

VOut main(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;

	float4 pos = position;
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.color = color;
	//output.uv = input.uv;
	//output.norm = input.norm;

	return output;
}
